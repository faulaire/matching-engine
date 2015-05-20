#include <gtest/gtest.h>

#include <unistd.h>

#include <Logger.h>

#include <Engine_MatchingEngine.h>
#include <Engine_Instrument.h>

using namespace exchange::engine;

class test_clock : public boost::posix_time::second_clock
{
public:

    using time_type          = boost::posix_time::ptime;
    using time_duration_type = boost::posix_time::ptime::time_duration_type;

public:

    static time_type local_time()
    {
        time_type local_time = boost::posix_time::second_clock::local_time();
        local_time -= static_offset.time_of_day();
        local_time += boost::posix_time::seconds(time_offset);
        return local_time;
    }

    static void set_now(const std::string & snow)
    {
        boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();
        boost::posix_time::ptime today_midnight(now.date());
        boost::posix_time::ptime today_start_time = today_midnight + boost::posix_time::duration_from_string(snow);

        static_offset = now - today_start_time.time_of_day();
    }

    static void go_to_future(long seconds)
    {
        time_offset += seconds;
    }

private:
    static long time_offset;
    static boost::posix_time::ptime static_offset;
};

long test_clock::time_offset = 0;
boost::posix_time::ptime test_clock::static_offset = boost::posix_time::ptime();

class MatchingEngineTest : public testing::Test
{
    public:

        using clock_type  = test_clock;
        using engine_type = exchange::engine::MatchingEngine<clock_type>;

        static const unsigned product_id = 1;

        virtual void SetUp()
        {
            if (boost::filesystem::exists("config.ini"))
            {
                boost::property_tree::ini_parser::read_ini("config.ini", m_Config);

                m_AuctionDurationOffset = m_Config.get<std::uint16_t>("Engine.auction_duration_offset_range");

                auto StartTime = m_Config.get<std::string>("Engine.start_time");

                test_clock::set_now(StartTime);

                WriteInstruments();

                m_pEngine.reset(new engine_type());
            }
            else
            {
                FAIL();
            }
        }

    private:

        void WriteInstruments()
        {
            std::string  InstrumentDBPath = m_Config.get<std::string>("Engine.instrument_db_path");

            auto key_extractor = [](const Instrument<Order> & Instrument) -> const std::string &
            {
                return Instrument.GetName();
            };

            InstrumentManager<Order> InstrMgr(InstrumentDBPath);

            Instrument<Order> Michelin{ "Michelin", "ISINMICH", "EUR", 1, 1254_price };
            Instrument<Order> Natixis{ "Natixis", "ISINNATI", "EUR", 2, 1255_price };
            Instrument<Order> IBM{ "IBM", "ISINIBM", "USD", 3, 1256_price };

            InstrMgr.Write(Michelin, key_extractor, true);
            InstrMgr.Write(Natixis, key_extractor, true);
            InstrMgr.Write(IBM, key_extractor, true);
        }

    protected:
        std::uint16_t                   m_AuctionDurationOffset;
        boost::property_tree::ptree     m_Config;
        std::unique_ptr<engine_type>    m_pEngine;
        
};

#define CREATE_ORDER(Way, Qty, Price, OrderID, ClientID) ( std::make_unique<Order>(Way, Qty, Price, OrderID, ClientID) )
#define CREATE_REPLACE(Way, Qty, Price, OldOrderID, NewOrderID, ClientID) ( std::make_unique<OrderReplace>(Way, Qty, Price, OldOrderID, NewOrderID, ClientID) )

#define INSERT_ORDER(Engine, pOrder, ProductID) ( Engine->Insert( std::move(pOrder), ProductID) )
#define MODIFY_ORDER(Engine, pReplace, ProductID) ( Engine->Modify( std::move(pReplace), ProductID) )


TEST_F(MatchingEngineTest, Should_configuration_success_when_valid_configuration_file)
{
    ASSERT_TRUE(m_pEngine->Configure(m_Config));
}

TEST_F(MatchingEngineTest, Should_configuration_fail_when_invalid_configuration_file)
{
    boost::property_tree::ptree aConfig;
    const std::string invalid_config_path = "invalid_config.ini";

    ASSERT_TRUE(boost::filesystem::exists(invalid_config_path));

    boost::property_tree::ini_parser::read_ini(invalid_config_path, aConfig);

    ASSERT_FALSE(m_pEngine->Configure(aConfig));
}

TEST_F(MatchingEngineTest, Should_configuration_fail_when_instrument_db_path_is_missing)
{
    boost::property_tree::ptree aConfig;
    const std::string missing_instrument_db_path = "missing_instrument_db_config.ini";

    ASSERT_TRUE(boost::filesystem::exists(missing_instrument_db_path));

    boost::property_tree::ini_parser::read_ini(missing_instrument_db_path, aConfig);

    ASSERT_FALSE(m_pEngine->Configure(aConfig));
}

TEST_F(MatchingEngineTest, Should_configuration_fail_when_database_is_inconsistent)
{
    boost::property_tree::ptree aConfig;
    const std::string invalid_config_path = "corrupted_db_config.ini";

    ASSERT_TRUE(boost::filesystem::exists(invalid_config_path));

    boost::property_tree::ini_parser::read_ini(invalid_config_path, aConfig);

    std::string  InstrumentDBPath = aConfig.get<std::string>("Engine.instrument_db_path");
    
    auto key_extractor = [](const Instrument<Order> & Instrument) -> const std::string &
    {
        return Instrument.GetName();
    };

    {
        InstrumentManager<Order> InstrMgr(InstrumentDBPath);

        Instrument<Order> Michelin{ "Michelin", "ISINMICH", "EUR", 1, 1254_price };
        Instrument<Order> MichelinBis{ "MichelinBis", "ISINMICH", "EUR", 1, 1254_price };

        InstrMgr.Write(Michelin, key_extractor, true);
        InstrMgr.Write(MichelinBis, key_extractor, true);
    }
    
    ASSERT_FALSE(m_pEngine->Configure(aConfig));
}

TEST_F(MatchingEngineTest, Should_configuration_fail_when_auction_duration_offset_is_higher_than_the_smallest_auction_duration)
{
    boost::property_tree::ptree aConfig;
    const std::string config_path = "invalid_auction_offset.ini";

    ASSERT_TRUE(boost::filesystem::exists(config_path));

    boost::property_tree::ini_parser::read_ini(config_path, aConfig);

    ASSERT_FALSE(m_pEngine->Configure(aConfig));
}


TEST_F(MatchingEngineTest, Should_configuration_fail_when_database_is_already_locked)
{
    std::string  InstrumentDBPath = m_Config.get<std::string>("Engine.instrument_db_path");

    InstrumentManager<Order> InstrMgr(InstrumentDBPath);
    
    InstrMgr.Load([](const auto & /*instr*/) {});

    ASSERT_FALSE(m_pEngine->Configure(m_Config));
}

TEST_F(MatchingEngineTest, Should_engine_state_be_closed_at_startup)
{
    ASSERT_TRUE(m_pEngine->Configure(m_Config));

    ASSERT_EQ(TradingPhase::CLOSE, m_pEngine->GetGlobalPhase());
}

TEST_F(MatchingEngineTest, Should_engine_state_be_opening_auction_after_close_during_trading_hours)
{
    ASSERT_TRUE(m_pEngine->Configure(m_Config));

    m_pEngine->EngineListen();
    ASSERT_EQ(TradingPhase::OPENING_AUCTION, m_pEngine->GetGlobalPhase());
}

TEST_F(MatchingEngineTest, Should_engine_state_be_contiunous_trading_after_opening_auction)
{
    ASSERT_TRUE(m_pEngine->Configure(m_Config));

    m_pEngine->EngineListen();

    auto auction_delay  = m_Config.get<unsigned int>("Engine.opening_auction_duration");

    clock_type::go_to_future(auction_delay + m_AuctionDurationOffset + 1);

    m_pEngine->EngineListen();

    ASSERT_EQ(TradingPhase::CONTINUOUS_TRADING, m_pEngine->GetGlobalPhase());
}

TEST_F(MatchingEngineTest, Should_engine_state_be_close_after_closing_auction)
{
    ASSERT_TRUE(m_pEngine->Configure(m_Config));

    m_pEngine->SetGlobalPhase(TradingPhase::CLOSING_AUCTION);
    m_pEngine->EngineListen();

    auto auction_delay = m_Config.get<unsigned int>("Engine.closing_auction_duration");

    clock_type::go_to_future(auction_delay + m_AuctionDurationOffset + 1);

    m_pEngine->EngineListen();

    ASSERT_EQ(TradingPhase::CLOSE, m_pEngine->GetGlobalPhase());
}

TEST_F(MatchingEngineTest, Should_engine_state_be_closing_auction_after_continous_trading_outside_trading_hours)
{
    boost::property_tree::ptree aConfig;

    ASSERT_TRUE(boost::filesystem::exists("config_always_closed.ini"));

    boost::property_tree::ini_parser::read_ini("config_always_closed.ini", aConfig);

    ASSERT_TRUE(m_pEngine->Configure(aConfig));

    m_pEngine->SetGlobalPhase(TradingPhase::CONTINUOUS_TRADING);
    m_pEngine->EngineListen();

    ASSERT_EQ(TradingPhase::CLOSING_AUCTION, m_pEngine->GetGlobalPhase());
}

TEST_F(MatchingEngineTest, Should_set_global_phase_success_when_valid_global_phase)
{
    ASSERT_TRUE(m_pEngine->Configure(m_Config));

    ASSERT_TRUE(m_pEngine->SetGlobalPhase(TradingPhase::OPENING_AUCTION));
    ASSERT_TRUE(m_pEngine->SetGlobalPhase(TradingPhase::CONTINUOUS_TRADING));
    ASSERT_TRUE(m_pEngine->SetGlobalPhase(TradingPhase::CLOSING_AUCTION));
    ASSERT_TRUE(m_pEngine->SetGlobalPhase(TradingPhase::CLOSE));
}

TEST_F(MatchingEngineTest, Should_set_global_phase_when_when_invalid_global_phase)
{
    ASSERT_TRUE(m_pEngine->Configure(m_Config));

    ASSERT_FALSE( m_pEngine->SetGlobalPhase(TradingPhase::INTRADAY_AUCTION) );
    ASSERT_FALSE( m_pEngine->SetGlobalPhase(TradingPhase::PHASES_SIZE) );
    ASSERT_FALSE( m_pEngine->SetGlobalPhase( static_cast<TradingPhase>(-1234) ) );
    ASSERT_FALSE( m_pEngine->SetGlobalPhase( static_cast<TradingPhase>(9876)  ) );
}

TEST_F(MatchingEngineTest, Should_order_insertion_fail_when_engine_closed)
{
    ASSERT_TRUE(m_pEngine->Configure(m_Config));

    auto o = CREATE_ORDER(OrderWay::BUY, 1000_qty, 1234_price, 1, 5);

    ASSERT_EQ(Status::MarketNotOpened, INSERT_ORDER(m_pEngine, o, product_id));
}

TEST_F(MatchingEngineTest, Should_order_modification_fail_when_engine_closed)
{
    ASSERT_TRUE(m_pEngine->Configure(m_Config));

    auto o = CREATE_ORDER(OrderWay::BUY, 1000_qty, 1234_price, 1, 5);
    
    ASSERT_TRUE(m_pEngine->SetGlobalPhase(TradingPhase::CONTINUOUS_TRADING));
    ASSERT_EQ(Status::Ok, INSERT_ORDER(m_pEngine, o, product_id));

    ASSERT_TRUE(m_pEngine->SetGlobalPhase(TradingPhase::CLOSE));

    auto Replace = CREATE_REPLACE(OrderWay::BUY, 2000_qty, 1234_price, 1, 2, 5);
    ASSERT_EQ(Status::MarketNotOpened, MODIFY_ORDER(m_pEngine, Replace, product_id));
}

TEST_F(MatchingEngineTest, Should_order_cancellation_fail_when_engine_closed)
{
    ASSERT_TRUE(m_pEngine->Configure(m_Config));

    auto o = CREATE_ORDER(OrderWay::BUY, 1000_qty, 1234_price, 1, 5);
    
    ASSERT_TRUE(m_pEngine->SetGlobalPhase(TradingPhase::CONTINUOUS_TRADING));
    ASSERT_EQ(Status::Ok, INSERT_ORDER(m_pEngine, o, product_id));

    ASSERT_TRUE(m_pEngine->SetGlobalPhase(TradingPhase::CLOSE));
    
    ASSERT_EQ(Status::MarketNotOpened, m_pEngine->Delete(1, 5, OrderWay::BUY, product_id));
}

TEST_F(MatchingEngineTest, Should_order_insertion_fail_when_invalid_instrument)
{
    ASSERT_TRUE(m_pEngine->Configure(m_Config));

    auto o = CREATE_ORDER(OrderWay::BUY, 1000_qty, 1234_price, 1, 5);

    ASSERT_TRUE(m_pEngine->SetGlobalPhase(TradingPhase::CONTINUOUS_TRADING));
    ASSERT_EQ(Status::InstrumentNotFound, INSERT_ORDER(m_pEngine, o, 25));
}

TEST_F(MatchingEngineTest, Should_order_modification_fail_when_invalid_instrument)
{
    ASSERT_TRUE(m_pEngine->Configure(m_Config));

    auto o = CREATE_ORDER(OrderWay::BUY, 1000_qty, 1234_price, 1, 5);

    ASSERT_TRUE(m_pEngine->SetGlobalPhase(TradingPhase::CONTINUOUS_TRADING));
    ASSERT_EQ(Status::Ok, INSERT_ORDER(m_pEngine, o, product_id));

    auto Replace = CREATE_REPLACE(OrderWay::BUY, 2000_qty, 1234_price, 1, 2, 5);
    ASSERT_EQ(Status::InstrumentNotFound, MODIFY_ORDER(m_pEngine, Replace, 25));
}

TEST_F(MatchingEngineTest, Should_order_cancellation_fail_when_invalid_instrument)
{
    ASSERT_TRUE(m_pEngine->Configure(m_Config));

    auto o = CREATE_ORDER(OrderWay::BUY, 1000_qty, 1234_price, 1, 5);

    ASSERT_TRUE(m_pEngine->SetGlobalPhase(TradingPhase::CONTINUOUS_TRADING));
    ASSERT_EQ(Status::Ok, INSERT_ORDER(m_pEngine, o, product_id));

    ASSERT_EQ(Status::InstrumentNotFound, m_pEngine->Delete(1, 5, OrderWay::BUY, 25));
}

TEST_F(MatchingEngineTest, Should_order_insertion_fail_when_already_inserted)
{
    ASSERT_TRUE(m_pEngine->Configure(m_Config));

    auto o1 = CREATE_ORDER(OrderWay::BUY, 1000_qty, 1234_price, 1, 5);

    ASSERT_TRUE(m_pEngine->SetGlobalPhase(TradingPhase::CONTINUOUS_TRADING));
    ASSERT_EQ(Status::Ok, INSERT_ORDER(m_pEngine, o1, product_id));

    auto o2 = CREATE_ORDER(OrderWay::BUY, 1000_qty, 1234_price, 1, 5);
    ASSERT_EQ(Status::IDAlreadyUsed, INSERT_ORDER(m_pEngine, o2, product_id));
}

TEST_F(MatchingEngineTest, Should_order_cancellation_fail_when_already_cancelled)
{
    ASSERT_TRUE(m_pEngine->Configure(m_Config));

    auto o = CREATE_ORDER(OrderWay::BUY, 1000_qty, 1234_price, 1, 5);

    ASSERT_TRUE(m_pEngine->SetGlobalPhase(TradingPhase::CONTINUOUS_TRADING));
    ASSERT_EQ(Status::Ok, INSERT_ORDER(m_pEngine, o, product_id));

    ASSERT_EQ(Status::Ok, m_pEngine->Delete(1, 5, OrderWay::BUY, product_id));
    ASSERT_EQ(Status::OrderNotFound, m_pEngine->Delete(1, 5, OrderWay::BUY, product_id));
}

TEST_F(MatchingEngineTest, Should_order_modification_fail_when_cancelled)
{
    ASSERT_TRUE(m_pEngine->Configure(m_Config));

    auto o = CREATE_ORDER(OrderWay::BUY, 1000_qty, 1234_price, 1, 5);

    ASSERT_TRUE(m_pEngine->SetGlobalPhase(TradingPhase::CONTINUOUS_TRADING));
    ASSERT_EQ(Status::Ok, INSERT_ORDER(m_pEngine, o, product_id));

    ASSERT_EQ(Status::Ok, m_pEngine->Delete(1, 5, OrderWay::BUY, product_id));

    auto Replace = CREATE_REPLACE(OrderWay::BUY, 2000_qty, 1234_price, 1, 2, 5);
    ASSERT_EQ(Status::OrderNotFound, MODIFY_ORDER(m_pEngine, Replace, product_id));
}

TEST_F(MatchingEngineTest, Should_order_insertion_success_when_engine_state_is_opening_auction)
{
    ASSERT_TRUE(m_pEngine->Configure(m_Config));

    m_pEngine->EngineListen();
    ASSERT_EQ(TradingPhase::OPENING_AUCTION, m_pEngine->GetGlobalPhase());

    auto o = CREATE_ORDER(OrderWay::BUY, 1000_qty, 1234_price, 1, 5);

    ASSERT_EQ(Status::Ok, INSERT_ORDER(m_pEngine, o, product_id));
}

TEST_F(MatchingEngineTest, Should_order_insertion_success_when_engine_state_is_closing_auction)
{
    ASSERT_TRUE(m_pEngine->Configure(m_Config));

    ASSERT_TRUE(m_pEngine->SetGlobalPhase(TradingPhase::CLOSING_AUCTION));

    auto o = CREATE_ORDER(OrderWay::BUY, 1000_qty, 1234_price, 1, 5);

    ASSERT_EQ(Status::Ok, INSERT_ORDER(m_pEngine, o, product_id));
}

TEST_F(MatchingEngineTest, Should_engine_not_generate_executions_during_opening_auction)
{
    ASSERT_TRUE(m_pEngine->Configure(m_Config));

    m_pEngine->EngineListen();
    ASSERT_EQ(m_pEngine->GetGlobalPhase(), TradingPhase::OPENING_AUCTION);

    auto ob = CREATE_ORDER(OrderWay::BUY, 1000_qty, 1234_price, 1, 5);
    auto os = CREATE_ORDER(OrderWay::SELL, 1000_qty, 1234_price, 2, 6);

    ASSERT_EQ(Status::Ok, INSERT_ORDER(m_pEngine, ob, product_id));
    ASSERT_EQ(Status::Ok, INSERT_ORDER(m_pEngine, os, product_id));

    auto pOrderBook = m_pEngine->GetOrderBook(product_id);
    ASSERT_NE(pOrderBook, nullptr);

    ASSERT_EQ(0, pOrderBook->GetDealCounter());
}

TEST_F(MatchingEngineTest, Should_engine_not_generate_executions_during_closing_auction)
{
    ASSERT_TRUE(m_pEngine->Configure(m_Config));

    ASSERT_TRUE(m_pEngine->SetGlobalPhase(TradingPhase::CLOSING_AUCTION));

    auto ob = CREATE_ORDER(OrderWay::BUY, 1000_qty, 1234_price, 1, 5);
    auto os = CREATE_ORDER(OrderWay::SELL, 1000_qty, 1234_price, 2, 6);

    ASSERT_EQ(Status::Ok, INSERT_ORDER(m_pEngine, ob, product_id));
    ASSERT_EQ(Status::Ok, INSERT_ORDER(m_pEngine, os, product_id));

    auto pOrderBook = m_pEngine->GetOrderBook(product_id);
    ASSERT_NE(pOrderBook, nullptr);

    ASSERT_EQ(0, pOrderBook->GetDealCounter());
}

TEST_F(MatchingEngineTest, Should_order_insertion_success_when_engine_state_is_continous_trading)
{
    ASSERT_TRUE(m_pEngine->Configure(m_Config));

    ASSERT_TRUE(m_pEngine->SetGlobalPhase(TradingPhase::CONTINUOUS_TRADING));

    auto o = CREATE_ORDER(OrderWay::BUY, 1000_qty, 1234_price, 1, 5);

    ASSERT_EQ(Status::Ok, INSERT_ORDER(m_pEngine, o, product_id));
}

TEST_F(MatchingEngineTest, Should_engine_generate_executions_during_continous_trading)
{
    ASSERT_TRUE(m_pEngine->Configure(m_Config));

    ASSERT_TRUE(m_pEngine->SetGlobalPhase(TradingPhase::CONTINUOUS_TRADING));

    auto ob = CREATE_ORDER(OrderWay::BUY, 1000_qty, 1234_price, 1, 5);
    auto os = CREATE_ORDER(OrderWay::SELL, 1000_qty, 1234_price, 2, 6);

    ASSERT_EQ(Status::Ok, INSERT_ORDER(m_pEngine, ob, product_id));
    ASSERT_EQ(Status::Ok, INSERT_ORDER(m_pEngine, os, product_id));

    auto pOrderBook = m_pEngine->GetOrderBook(product_id);
    ASSERT_NE(pOrderBook, nullptr);

    ASSERT_EQ(1, pOrderBook->GetDealCounter());
}

TEST_F(MatchingEngineTest, Should_order_modification_fail_when_fully_executed)
{
    ASSERT_TRUE(m_pEngine->Configure(m_Config));

    ASSERT_TRUE(m_pEngine->SetGlobalPhase(TradingPhase::CONTINUOUS_TRADING));

    auto ob = CREATE_ORDER(OrderWay::BUY, 1000_qty, 1234_price, 1, 5);
    auto os = CREATE_ORDER(OrderWay::SELL, 1000_qty, 1234_price, 2, 6);

    ASSERT_EQ(Status::Ok, INSERT_ORDER(m_pEngine, ob, product_id));
    ASSERT_EQ(Status::Ok, INSERT_ORDER(m_pEngine, os, product_id));

    auto Replace = CREATE_REPLACE(OrderWay::BUY, 2000_qty, 1234_price, 1, 2, 5);
    ASSERT_EQ(Status::OrderNotFound, MODIFY_ORDER(m_pEngine, Replace, product_id));
}

TEST_F(MatchingEngineTest, Should_order_cancellation_fail_when_fully_executed)
{
    ASSERT_TRUE(m_pEngine->Configure(m_Config));

    ASSERT_TRUE(m_pEngine->SetGlobalPhase(TradingPhase::CONTINUOUS_TRADING));

    auto ob = CREATE_ORDER(OrderWay::BUY, 1000_qty, 1234_price, 1, 5);
    auto os = CREATE_ORDER(OrderWay::SELL, 1000_qty, 1234_price, 2, 6);

    ASSERT_EQ(Status::Ok, INSERT_ORDER(m_pEngine, ob, product_id));
    ASSERT_EQ(Status::Ok, INSERT_ORDER(m_pEngine, os, product_id));

    ASSERT_EQ(Status::OrderNotFound, m_pEngine->Delete(1, 5, OrderWay::BUY, product_id));
}

TEST_F(MatchingEngineTest, Should_non_persistent_orders_being_cancelled_after_clausing_auction)
{
    ASSERT_TRUE(m_pEngine->Configure(m_Config));

    ASSERT_TRUE(m_pEngine->SetGlobalPhase(TradingPhase::CLOSING_AUCTION));

    auto ob = CREATE_ORDER(OrderWay::BUY, 1000_qty, 1234_price, 1, 5);

    ASSERT_EQ(Status::Ok, INSERT_ORDER(m_pEngine, ob, product_id));

    m_pEngine->SetGlobalPhase(TradingPhase::CLOSING_AUCTION);

    auto auction_delay = m_Config.get<unsigned int>("Engine.closing_auction_duration");
    clock_type::go_to_future(auction_delay + m_AuctionDurationOffset + 1);

    m_pEngine->EngineListen();

    ASSERT_EQ(m_pEngine->GetGlobalPhase(), TradingPhase::CLOSE);
    ASSERT_TRUE(m_pEngine->SetGlobalPhase(TradingPhase::OPENING_AUCTION));

    ASSERT_EQ(Status::OrderNotFound, m_pEngine->Delete(1, 5, OrderWay::BUY, product_id));
}

TEST_F(MatchingEngineTest, Should_orderbook_be_monitored_when_switching_to_intraday_auction)
{
    ASSERT_TRUE(m_pEngine->Configure(m_Config));

    ASSERT_TRUE(m_pEngine->SetGlobalPhase(TradingPhase::CONTINUOUS_TRADING));

    auto ob = CREATE_ORDER(OrderWay::BUY, 1000_qty, 2000_price, 1, 5);
    auto os = CREATE_ORDER(OrderWay::SELL, 1000_qty, 2000_price, 1, 6);

    ASSERT_EQ(Status::Ok, INSERT_ORDER(m_pEngine, ob, product_id));
    ASSERT_EQ(Status::Ok, INSERT_ORDER(m_pEngine, os, product_id));

    ASSERT_EQ(1, m_pEngine->GetMonitoredOrderBookCounter());
}

TEST_F(MatchingEngineTest, Should_intraday_auction_duration_be_updated_when_intraday_auction_occurs)
{
    ASSERT_TRUE(m_pEngine->Configure(m_Config));

    ASSERT_TRUE(m_pEngine->SetGlobalPhase(TradingPhase::CONTINUOUS_TRADING));

    auto current_intraday_auction_duration = m_pEngine->GetIntradayAuctionDuration();

    auto ob = CREATE_ORDER(OrderWay::BUY, 1000_qty, 2000_price, 1, 5);
    auto os = CREATE_ORDER(OrderWay::SELL, 1000_qty, 2000_price, 1, 6);

    ASSERT_EQ(Status::Ok, INSERT_ORDER(m_pEngine, ob, product_id));
    ASSERT_EQ(Status::Ok, INSERT_ORDER(m_pEngine, os, product_id));

    auto new_intraday_auction_duration = m_pEngine->GetIntradayAuctionDuration();

    ASSERT_NE(current_intraday_auction_duration, new_intraday_auction_duration);
}

TEST_F(MatchingEngineTest, Should_orderbook_be_unmonitored_at_the_end_of_intraday_auction)
{
    ASSERT_TRUE(m_pEngine->Configure(m_Config));

    ASSERT_TRUE(m_pEngine->SetGlobalPhase(TradingPhase::CONTINUOUS_TRADING));

    auto ob = CREATE_ORDER(OrderWay::BUY, 1000_qty, 2000_price, 1, 5);
    auto os = CREATE_ORDER(OrderWay::SELL, 1000_qty, 2000_price, 1, 6);

    ASSERT_EQ(Status::Ok, INSERT_ORDER(m_pEngine, ob, product_id));
    ASSERT_EQ(Status::Ok, INSERT_ORDER(m_pEngine, os, product_id));

    ASSERT_EQ(1, m_pEngine->GetMonitoredOrderBookCounter());
    
    m_pEngine->EngineListen();
    
    ASSERT_EQ(1, m_pEngine->GetMonitoredOrderBookCounter());
    
    auto auction_delay = m_Config.get<unsigned int>("Engine.intraday_auction_duration");
    clock_type::go_to_future(auction_delay + m_AuctionDurationOffset + 1);
    
    m_pEngine->EngineListen();
    
    ASSERT_EQ(0, m_pEngine->GetMonitoredOrderBookCounter());
}

TEST_F(MatchingEngineTest, Should_close_price_be_saved_when_global_phase_switch_to_close)
{
    ASSERT_TRUE(m_pEngine->Configure(m_Config));

    auto pOrderBook = m_pEngine->GetOrderBook(product_id);
    ASSERT_NE(pOrderBook, nullptr);

    auto PreviousClosePrice = pOrderBook->GetClosePrice();
    auto NewClosePrice = PreviousClosePrice + 1_price;

    auto ob = CREATE_ORDER(OrderWay::BUY, 1000_qty, NewClosePrice, 1, 5);
    auto os = CREATE_ORDER(OrderWay::SELL, 1000_qty, NewClosePrice, 1, 6);

    ASSERT_TRUE(m_pEngine->SetGlobalPhase(TradingPhase::CLOSING_AUCTION));

    ASSERT_EQ(Status::Ok, INSERT_ORDER(m_pEngine, ob, product_id));
    ASSERT_EQ(Status::Ok, INSERT_ORDER(m_pEngine, os, product_id));

    ASSERT_TRUE(m_pEngine->SetGlobalPhase(TradingPhase::CLOSE));

    ASSERT_EQ(NewClosePrice, pOrderBook->GetClosePrice());

    std::string  InstrumentDBPath = m_Config.get<std::string>("Engine.instrument_db_path");
    InstrumentManager<Order> InstrMgr(InstrumentDBPath);

    Instrument<Order> Instrument;
    InstrMgr.Get(pOrderBook->GetSecurityName(), Instrument);

    ASSERT_EQ(NewClosePrice, Instrument.GetClosePrice());
}

int main(int argc, char ** argv)
{
    auto & Logger = LoggerHolder::GetInstance();

    if (boost::filesystem::exists("config.ini"))
    {
        boost::property_tree::ptree aConfig;

        boost::property_tree::ini_parser::read_ini("config.ini", aConfig);
        Logger.Init(aConfig);
    }

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
