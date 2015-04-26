#include <gtest/gtest.h>

#include <unistd.h>

#include <Logger.h>

#include <Engine_MatchingEngine.h>
#include <Engine_Instrument.h>

using namespace exchange::engine;

class MatchingEngineTest : public testing::Test
{
    public:

        virtual void SetUp()
        {
            auto & Logger = LoggerHolder::GetInstance();

            if (boost::filesystem::exists("config.ini"))
            {
                boost::property_tree::ini_parser::read_ini("config.ini", m_Config);
                Logger.Init(m_Config);
            }

            {
                // TODO : Add a specific test for the Instrument Manager
                std::string  InstrumentDBPath = m_Config.get<std::string>("Engine.instrument_db_path");
                InstrumentManager<Order> InstrMgr(InstrumentDBPath);

                Instrument<Order> Michelin{ "Michelin", "ISINMICH", "EUR", 1, 1254 };
                Instrument<Order> Natixis{ "Natixis", "ISINNATI", "EUR", 2, 1255 };
                Instrument<Order> IBM{ "IBM", "ISINIBM", "USD", 3, 1256 };

                EXINFO(Michelin);
                EXINFO(Natixis);
                EXINFO(IBM);

                InstrMgr.Write(Michelin, true);
                InstrMgr.Write(Natixis, true);
                InstrMgr.Write(IBM, true);
            }
        }

    protected:

        boost::property_tree::ptree         m_Config;
        exchange::engine::MatchingEngine    m_Engine;
        
};

TEST_F(MatchingEngineTest, Configure)
{
    ASSERT_TRUE(m_Engine.Configure(m_Config));
}


TEST_F(MatchingEngineTest, PhaseSwitching)
{
    ASSERT_TRUE(m_Engine.Configure(m_Config));
    
    ASSERT_EQ(m_Engine.GetGlobalPhase(), TradingPhase::CLOSE);
    m_Engine.EngineListen();
    ASSERT_EQ(m_Engine.GetGlobalPhase(), TradingPhase::OPENING_AUCTION);

    sleep(5);
    m_Engine.EngineListen();

    ASSERT_EQ(m_Engine.GetGlobalPhase(), TradingPhase::CONTINUOUS_TRADING);

    m_Engine.SetGlobalPhase(TradingPhase::CLOSING_AUCTION);

    ASSERT_EQ(m_Engine.GetGlobalPhase(), TradingPhase::CLOSING_AUCTION);

    sleep(5);
    m_Engine.EngineListen();

    ASSERT_EQ(m_Engine.GetGlobalPhase(), TradingPhase::CLOSE);
}

TEST_F(MatchingEngineTest, ContinousToClose)
{
    boost::property_tree::ptree aConfig;

    ASSERT_TRUE(boost::filesystem::exists("config_always_closed.ini"));

    boost::property_tree::ini_parser::read_ini("config_always_closed.ini", aConfig);
    
    ASSERT_TRUE(m_Engine.Configure(aConfig));
    
    m_Engine.SetGlobalPhase(TradingPhase::CONTINUOUS_TRADING);
    m_Engine.EngineListen();
    
    ASSERT_EQ(m_Engine.GetGlobalPhase(), TradingPhase::CLOSING_AUCTION);
    
    sleep(5);
    m_Engine.EngineListen();
    
    ASSERT_EQ(m_Engine.GetGlobalPhase(), TradingPhase::CLOSE);

}

TEST_F(MatchingEngineTest, Phases)
{
    ASSERT_TRUE(m_Engine.Configure(m_Config));

    ASSERT_FALSE( m_Engine.SetGlobalPhase(TradingPhase::INTRADAY_AUCTION) );
    ASSERT_FALSE( m_Engine.SetGlobalPhase(TradingPhase::PHASES_SIZE) );
    ASSERT_FALSE( m_Engine.SetGlobalPhase( static_cast<TradingPhase>(-1234) ) );
    ASSERT_FALSE( m_Engine.SetGlobalPhase( static_cast<TradingPhase>(9876)  ) );

    ASSERT_TRUE( m_Engine.SetGlobalPhase(TradingPhase::OPENING_AUCTION) );
    ASSERT_TRUE( m_Engine.SetGlobalPhase(TradingPhase::CONTINUOUS_TRADING) );
    ASSERT_TRUE( m_Engine.SetGlobalPhase(TradingPhase::CLOSING_AUCTION) );
    ASSERT_TRUE( m_Engine.SetGlobalPhase(TradingPhase::CLOSE) );
}

TEST_F(MatchingEngineTest, InsertClose)
{
    ASSERT_TRUE(m_Engine.Configure(m_Config));

    Order o(OrderWay::BUY, 1000, 1234, 1, 5);

    // Insert on a valid instrument
    ASSERT_FALSE(m_Engine.Insert(o, 1));

    ASSERT_EQ( m_Engine.GetOrderBook(17), nullptr);
    // Insert on a non-existing instrument
    ASSERT_FALSE(m_Engine.Insert(o, 17));


    OrderReplace Replace(OrderWay::BUY, 2000, 1234, 1, 2, 5);
    ASSERT_FALSE(m_Engine.Modify(Replace, 1));
    // Replace on a non-existing instrument
    ASSERT_FALSE(m_Engine.Modify(Replace, 17));

    ASSERT_FALSE(m_Engine.Delete(2, 5, OrderWay::BUY, 1));
    // Delete on a non-existing instrument
    ASSERT_FALSE(m_Engine.Delete(2, 5, OrderWay::BUY, 17));
}

TEST_F(MatchingEngineTest, InsertAuction)
{

    ASSERT_TRUE(m_Engine.Configure(m_Config));

    Order o(OrderWay::BUY, 1000, 1234, 1, 5);

    // Switching to auction phase
    m_Engine.EngineListen();

    ASSERT_TRUE(m_Engine.Insert(o, 1));
    ASSERT_FALSE(m_Engine.Insert(o, 1));

    OrderReplace Replace(OrderWay::BUY, 2000, 1234, 1, 2, 5);
    ASSERT_TRUE(m_Engine.Modify(Replace, 1));
    ASSERT_FALSE(m_Engine.Modify(Replace, 2));

    ASSERT_TRUE(m_Engine.Delete(2, 5, OrderWay::BUY, 1));
    ASSERT_FALSE(m_Engine.Delete(2, 5, OrderWay::BUY, 1));
}

TEST_F(MatchingEngineTest, InsertContinousTrading)
{

    InstrumentManager<Order> Mgr("");

    ASSERT_TRUE(m_Engine.Configure(m_Config));

    Order OrderBuy(OrderWay::BUY, 1000, 1254, 1, 2);
    Order OrderSell(OrderWay::SELL, 1000, 1254, 3, 4);

    m_Engine.SetGlobalPhase(TradingPhase::CONTINUOUS_TRADING);

    // TODO : These tests should be moved to OrderBook
    auto pOrderBook = m_Engine.GetOrderBook(1);
    ASSERT_NE(pOrderBook, nullptr);
    ASSERT_EQ(pOrderBook->GetInstrumentID(), 1);

    ASSERT_TRUE(m_Engine.Insert(OrderBuy, 1));
    ASSERT_TRUE(m_Engine.Insert(OrderSell, 1));

    ASSERT_EQ(pOrderBook->GetDealCounter(), 1);
    ASSERT_EQ(pOrderBook->GetDailyVolume(), 1000);
    ASSERT_EQ(pOrderBook->GetTurnover(), 1254000);
    ASSERT_EQ(pOrderBook->GetLastPrice(), 1254);

    ASSERT_EQ(pOrderBook->GetTradingPhase(), TradingPhase::CONTINUOUS_TRADING);

    /* Generate a deal with a too big price => Switching to Intraday auction */
    OrderBuy = Order(OrderWay::BUY, 1000, 1545, 1, 2);
    OrderSell = Order(OrderWay::SELL, 1000, 1545, 3, 4);

    ASSERT_TRUE(m_Engine.Insert(OrderBuy, 1));
    ASSERT_TRUE(m_Engine.Insert(OrderSell, 1));

    ASSERT_EQ(pOrderBook->GetTradingPhase(), TradingPhase::INTRADAY_AUCTION);
    ASSERT_EQ(pOrderBook->GetDealCounter(), 2);
    ASSERT_EQ(pOrderBook->GetDailyVolume(), 2000);
    ASSERT_EQ(pOrderBook->GetTurnover(), 2799000);
    ASSERT_EQ(pOrderBook->GetLastPrice(), 1545);

    m_Engine.EngineListen();
    sleep(5);
    m_Engine.EngineListen();

    ASSERT_EQ(pOrderBook->GetTradingPhase(), TradingPhase::CONTINUOUS_TRADING);
}

int main(int argc, char ** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
