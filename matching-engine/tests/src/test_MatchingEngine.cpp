#include <gtest/gtest.h>

#include <unistd.h>

#include <Logger.h>

#include <Engine_MatchingEngine.h>
#include <Engine_Instrument.h>

using namespace exchange::engine;

class MatchingEngineTest : public testing::Test
{
    public:

        static const unsigned product_id = 1;

        virtual void SetUp()
        {
            m_pEngine.reset(new exchange::engine::MatchingEngine());

            if (boost::filesystem::exists("config.ini"))
            {
                boost::property_tree::ini_parser::read_ini("config.ini", m_Config);
            }

            WriteInstruments();
        }

    private:

        void WriteInstruments()
        {
            std::string  InstrumentDBPath = m_Config.get<std::string>("Engine.instrument_db_path");

            auto key_extractor = [](const Instrument<Order> & Instrument) -> const std::string &
            {
                return Instrument.GetName();
            };

            InstrumentManager<Order> InstrMgr(InstrumentDBPath, key_extractor);

            Instrument<Order> Michelin{ "Michelin", "ISINMICH", "EUR", 1, 1254 };
            Instrument<Order> Natixis{ "Natixis", "ISINNATI", "EUR", 2, 1255 };
            Instrument<Order> IBM{ "IBM", "ISINIBM", "USD", 3, 1256 };

            InstrMgr.Write(Michelin, true);
            InstrMgr.Write(Natixis, true);
            InstrMgr.Write(IBM, true);
        }

    protected:

        boost::property_tree::ptree                          m_Config;
        std::unique_ptr<exchange::engine::MatchingEngine>    m_pEngine;
        
};

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

    InstrumentManager<Order> InstrMgr(InstrumentDBPath, key_extractor);

    Instrument<Order> Michelin{ "Michelin", "ISINMICH", "EUR", 1, 1254 };
    Instrument<Order> MichelinBis{ "MichelinBis", "ISINMICH", "EUR", 1, 1254 };

    InstrMgr.Write(Michelin, true);
    InstrMgr.Write(MichelinBis, true);
    
    ASSERT_FALSE(m_pEngine->Configure(aConfig));
}

TEST_F(MatchingEngineTest, Should_engine_state_be_closed_at_startup)
{
    ASSERT_TRUE(m_pEngine->Configure(m_Config));

    ASSERT_EQ(m_pEngine->GetGlobalPhase(), TradingPhase::CLOSE);
}

TEST_F(MatchingEngineTest, Should_engine_state_be_opening_auction_after_close_during_trading_hours)
{
    ASSERT_TRUE(m_pEngine->Configure(m_Config));

    m_pEngine->EngineListen();
    ASSERT_EQ(m_pEngine->GetGlobalPhase(), TradingPhase::OPENING_AUCTION);
}

TEST_F(MatchingEngineTest, Should_engine_state_be_contiunous_trading_after_opening_auction)
{
    ASSERT_TRUE(m_pEngine->Configure(m_Config));

    m_pEngine->EngineListen();

    auto auction_delay  = m_Config.get<unsigned int>("Engine.opening_auction_duration");

    sleep(auction_delay+1);

    m_pEngine->EngineListen();

    ASSERT_EQ(m_pEngine->GetGlobalPhase(), TradingPhase::CONTINUOUS_TRADING);
}

TEST_F(MatchingEngineTest, Should_engine_state_be_close_after_closing_auction)
{
    ASSERT_TRUE(m_pEngine->Configure(m_Config));

    m_pEngine->SetGlobalPhase(TradingPhase::CLOSING_AUCTION);
    m_pEngine->EngineListen();

    auto auction_delay = m_Config.get<unsigned int>("Engine.closing_auction_duration");

    sleep(auction_delay + 1);

    m_pEngine->EngineListen();

    ASSERT_EQ(m_pEngine->GetGlobalPhase(), TradingPhase::CLOSE);
}

TEST_F(MatchingEngineTest, Should_engine_state_be_closing_auction_after_continous_trading_outside_trading_hours)
{
    boost::property_tree::ptree aConfig;

    ASSERT_TRUE(boost::filesystem::exists("config_always_closed.ini"));

    boost::property_tree::ini_parser::read_ini("config_always_closed.ini", aConfig);

    ASSERT_TRUE(m_pEngine->Configure(aConfig));

    m_pEngine->SetGlobalPhase(TradingPhase::CONTINUOUS_TRADING);
    m_pEngine->EngineListen();

    ASSERT_EQ(m_pEngine->GetGlobalPhase(), TradingPhase::CLOSING_AUCTION);
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

    Order o(OrderWay::BUY, 1000, 1234, 1, 5);

    ASSERT_FALSE(m_pEngine->Insert(o, product_id));
}

TEST_F(MatchingEngineTest, Should_order_modification_fail_when_engine_closed)
{
    ASSERT_TRUE(m_pEngine->Configure(m_Config));

    Order o(OrderWay::BUY, 1000, 1234, 1, 5);
    
    ASSERT_TRUE(m_pEngine->SetGlobalPhase(TradingPhase::CONTINUOUS_TRADING));
    ASSERT_TRUE(m_pEngine->Insert(o, product_id));

    ASSERT_TRUE(m_pEngine->SetGlobalPhase(TradingPhase::CLOSE));

    OrderReplace Replace(OrderWay::BUY, 2000, 1234, 1, 2, 5);
    ASSERT_FALSE(m_pEngine->Modify(Replace, product_id));
}

TEST_F(MatchingEngineTest, Should_order_cancellation_fail_when_engine_closed)
{
    ASSERT_TRUE(m_pEngine->Configure(m_Config));

    Order o(OrderWay::BUY, 1000, 1234, 1, 5);
    
    ASSERT_TRUE(m_pEngine->SetGlobalPhase(TradingPhase::CONTINUOUS_TRADING));
    ASSERT_TRUE(m_pEngine->Insert(o, product_id));

    ASSERT_TRUE(m_pEngine->SetGlobalPhase(TradingPhase::CLOSE));
    
    ASSERT_FALSE(m_pEngine->Delete(1, 5, OrderWay::BUY, product_id));
}

TEST_F(MatchingEngineTest, Should_order_insertion_fail_when_invalid_instrument)
{
    ASSERT_TRUE(m_pEngine->Configure(m_Config));

    Order o(OrderWay::BUY, 1000, 1234, 1, 5);

    ASSERT_TRUE(m_pEngine->SetGlobalPhase(TradingPhase::CONTINUOUS_TRADING));
    ASSERT_FALSE(m_pEngine->Insert(o, 25));
}

TEST_F(MatchingEngineTest, Should_order_modification_fail_when_invalid_instrument)
{
    ASSERT_TRUE(m_pEngine->Configure(m_Config));

    Order o(OrderWay::BUY, 1000, 1234, 1, 5);

    ASSERT_TRUE(m_pEngine->SetGlobalPhase(TradingPhase::CONTINUOUS_TRADING));
    ASSERT_TRUE(m_pEngine->Insert(o, product_id));

    OrderReplace Replace(OrderWay::BUY, 2000, 1234, 1, 2, 5);
    ASSERT_FALSE(m_pEngine->Modify(Replace, 25));
}

TEST_F(MatchingEngineTest, Should_order_cancellation_fail_when_invalid_instrument)
{
    ASSERT_TRUE(m_pEngine->Configure(m_Config));

    Order o(OrderWay::BUY, 1000, 1234, 1, 5);

    ASSERT_TRUE(m_pEngine->SetGlobalPhase(TradingPhase::CONTINUOUS_TRADING));
    ASSERT_TRUE(m_pEngine->Insert(o, product_id));

    ASSERT_FALSE(m_pEngine->Delete(1, 5, OrderWay::BUY, 25));
}

TEST_F(MatchingEngineTest, Should_order_insertion_fail_when_already_inserted)
{
    ASSERT_TRUE(m_pEngine->Configure(m_Config));

    Order o(OrderWay::BUY, 1000, 1234, 1, 5);

    ASSERT_TRUE(m_pEngine->SetGlobalPhase(TradingPhase::CONTINUOUS_TRADING));
    ASSERT_TRUE(m_pEngine->Insert(o, product_id));
    ASSERT_FALSE(m_pEngine->Insert(o, product_id));
}

TEST_F(MatchingEngineTest, Should_order_cancellation_fail_when_already_cancelled)
{
    ASSERT_TRUE(m_pEngine->Configure(m_Config));

    Order o(OrderWay::BUY, 1000, 1234, 1, 5);

    ASSERT_TRUE(m_pEngine->SetGlobalPhase(TradingPhase::CONTINUOUS_TRADING));
    ASSERT_TRUE(m_pEngine->Insert(o, product_id));

    ASSERT_TRUE(m_pEngine->Delete(1, 5, OrderWay::BUY, product_id));
    ASSERT_FALSE(m_pEngine->Delete(1, 5, OrderWay::BUY, product_id));
}

TEST_F(MatchingEngineTest, Should_order_modification_fail_when_cancelled)
{
    ASSERT_TRUE(m_pEngine->Configure(m_Config));

    Order o(OrderWay::BUY, 1000, 1234, 1, 5);

    ASSERT_TRUE(m_pEngine->SetGlobalPhase(TradingPhase::CONTINUOUS_TRADING));
    ASSERT_TRUE(m_pEngine->Insert(o, product_id));

    ASSERT_TRUE(m_pEngine->Delete(1, 5, OrderWay::BUY, product_id));

    OrderReplace Replace(OrderWay::BUY, 2000, 1234, 1, 2, 5);
    ASSERT_FALSE(m_pEngine->Modify(Replace, product_id));
}

TEST_F(MatchingEngineTest, Should_order_insertion_success_when_engine_state_is_opening_auction)
{
    ASSERT_TRUE(m_pEngine->Configure(m_Config));

    m_pEngine->EngineListen();
    ASSERT_EQ(m_pEngine->GetGlobalPhase(), TradingPhase::OPENING_AUCTION);

    Order o(OrderWay::BUY, 1000, 1234, 1, 5);

    ASSERT_TRUE(m_pEngine->Insert(o, product_id));
}

TEST_F(MatchingEngineTest, Should_order_insertion_success_when_engine_state_is_closing_auction)
{
    ASSERT_TRUE(m_pEngine->Configure(m_Config));

    ASSERT_TRUE(m_pEngine->SetGlobalPhase(TradingPhase::CLOSING_AUCTION));

    Order o(OrderWay::BUY, 1000, 1234, 1, 5);

    ASSERT_TRUE(m_pEngine->Insert(o, product_id));
}

TEST_F(MatchingEngineTest, Should_engine_not_generate_executions_during_opening_auction)
{
    ASSERT_TRUE(m_pEngine->Configure(m_Config));

    m_pEngine->EngineListen();
    ASSERT_EQ(m_pEngine->GetGlobalPhase(), TradingPhase::OPENING_AUCTION);

    Order ob(OrderWay::BUY, 1000, 1234, 1, 5);
    Order os(OrderWay::SELL, 1000, 1234, 2, 6);

    ASSERT_TRUE(m_pEngine->Insert(ob, product_id));
    ASSERT_TRUE(m_pEngine->Insert(os, product_id));

    auto pOrderBook = m_pEngine->GetOrderBook(product_id);
    ASSERT_NE(pOrderBook, nullptr);

    ASSERT_EQ(0, pOrderBook->GetDealCounter());
}

TEST_F(MatchingEngineTest, Should_engine_not_generate_executions_during_closing_auction)
{
    ASSERT_TRUE(m_pEngine->Configure(m_Config));

    ASSERT_TRUE(m_pEngine->SetGlobalPhase(TradingPhase::CLOSING_AUCTION));

    Order ob(OrderWay::BUY, 1000, 1234, 1, 5);
    Order os(OrderWay::SELL, 1000, 1234, 2, 6);

    ASSERT_TRUE(m_pEngine->Insert(ob, product_id));
    ASSERT_TRUE(m_pEngine->Insert(os, product_id));

    auto pOrderBook = m_pEngine->GetOrderBook(product_id);
    ASSERT_NE(pOrderBook, nullptr);

    ASSERT_EQ(0, pOrderBook->GetDealCounter());
}

TEST_F(MatchingEngineTest, Should_order_insertion_success_when_engine_state_is_continous_trading)
{
    ASSERT_TRUE(m_pEngine->Configure(m_Config));

    ASSERT_TRUE(m_pEngine->SetGlobalPhase(TradingPhase::CONTINUOUS_TRADING));

    Order o(OrderWay::BUY, 1000, 1234, 1, 5);

    ASSERT_TRUE(m_pEngine->Insert(o, product_id));
}

TEST_F(MatchingEngineTest, Should_engine_generate_executions_during_continous_trading)
{
    ASSERT_TRUE(m_pEngine->Configure(m_Config));

    ASSERT_TRUE(m_pEngine->SetGlobalPhase(TradingPhase::CONTINUOUS_TRADING));

    Order ob(OrderWay::BUY, 1000, 1234, 1, 5);
    Order os(OrderWay::SELL, 1000, 1234, 2, 6);

    ASSERT_TRUE(m_pEngine->Insert(ob, product_id));
    ASSERT_TRUE(m_pEngine->Insert(os, product_id));

    auto pOrderBook = m_pEngine->GetOrderBook(product_id);
    ASSERT_NE(pOrderBook, nullptr);

    ASSERT_EQ(1, pOrderBook->GetDealCounter());
}

TEST_F(MatchingEngineTest, Should_order_modification_fail_when_fully_executed)
{
    ASSERT_TRUE(m_pEngine->Configure(m_Config));

    ASSERT_TRUE(m_pEngine->SetGlobalPhase(TradingPhase::CONTINUOUS_TRADING));

    Order ob(OrderWay::BUY, 1000, 1234, 1, 5);
    Order os(OrderWay::SELL, 1000, 1234, 2, 6);

    ASSERT_TRUE(m_pEngine->Insert(ob, product_id));
    ASSERT_TRUE(m_pEngine->Insert(os, product_id));

    OrderReplace Replace(OrderWay::BUY, 2000, 1234, 1, 2, 5);
    ASSERT_FALSE(m_pEngine->Modify(Replace, product_id));

}

TEST_F(MatchingEngineTest, Should_order_cancellation_fail_when_fully_executed)
{
    ASSERT_TRUE(m_pEngine->Configure(m_Config));

    ASSERT_TRUE(m_pEngine->SetGlobalPhase(TradingPhase::CONTINUOUS_TRADING));

    Order ob(OrderWay::BUY, 1000, 1234, 1, 5);
    Order os(OrderWay::SELL, 1000, 1234, 2, 6);

    ASSERT_TRUE(m_pEngine->Insert(ob, product_id));
    ASSERT_TRUE(m_pEngine->Insert(os, product_id));

    ASSERT_FALSE(m_pEngine->Delete(1,5,OrderWay::BUY, product_id));
}

TEST_F(MatchingEngineTest, Should_non_persistent_orders_being_cancelled_after_clausing_auction)
{
    ASSERT_TRUE(m_pEngine->Configure(m_Config));

    ASSERT_TRUE(m_pEngine->SetGlobalPhase(TradingPhase::CLOSING_AUCTION));

    Order ob(OrderWay::BUY, 1000, 1234, 1, 5);

    ASSERT_TRUE(m_pEngine->Insert(ob, product_id));

    m_pEngine->SetGlobalPhase(TradingPhase::CLOSING_AUCTION);

    auto auction_delay = m_Config.get<unsigned int>("Engine.closing_auction_duration");
    sleep(auction_delay + 1);

    m_pEngine->EngineListen();

    ASSERT_EQ(m_pEngine->GetGlobalPhase(), TradingPhase::CLOSE);

    ASSERT_FALSE(m_pEngine->Delete(1, 5, OrderWay::BUY, product_id));
}

/*
    TODO  Test that we cannot reinsert a full executed order
    TODO  CheckOrderBooks never iterates over monitored order books
*/

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
