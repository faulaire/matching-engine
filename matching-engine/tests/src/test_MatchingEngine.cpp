#include <gtest/gtest.h>

#include <unistd.h>

#include <Engine_MatchingEngine.h>

#include <ConfigurationMgr.h>

using namespace exchange::engine;

// To use a test fixture, derive a class from testing::Test.
class MatchingEngineTest : public testing::Test
{
    public:

        virtual void SetUp()
        {
            auto & CfgManager = exchange::common::ConfigurationMgr::GetInstance();
            CfgManager.Reset();

            boost::property_tree::ptree aConfig;
            if (boost::filesystem::exists("config.ini"))
            {
                boost::property_tree::ini_parser::read_ini("config.ini", aConfig);

                ASSERT_TRUE(m_Connector.Configure(aConfig));
            }
        }

    protected:

        exchange::engine::MatchingEngine    m_Engine;
        exchange::common::DataBaseConnector m_Connector;
};

TEST_F(MatchingEngineTest, Configure)
{
    ASSERT_TRUE(m_Engine.Configure(m_Connector));
}

TEST_F(MatchingEngineTest, NotExistingDB)
{
    boost::property_tree::ptree aConfig;
    if (boost::filesystem::exists("config_not_existing_db.ini"))
    {
        boost::property_tree::ini_parser::read_ini("config_not_existing_db.ini", aConfig);

        ASSERT_TRUE(m_Connector.Configure(aConfig));
        ASSERT_FALSE(m_Engine.Configure(m_Connector));
    }
    else
    {
        ASSERT_FALSE(true);
    }
}

TEST_F(MatchingEngineTest, CorruptedDB)
{
    boost::property_tree::ptree aConfig;
    if (boost::filesystem::exists("config_corrupted_database.ini"))
    {
        boost::property_tree::ini_parser::read_ini("config_corrupted_database.ini", aConfig);

        ASSERT_TRUE(m_Connector.Configure(aConfig));
        ASSERT_FALSE(m_Engine.Configure(m_Connector));
    }
    else
    {
        ASSERT_FALSE(true);
    }
}

TEST_F(MatchingEngineTest, PhaseSwitching)
{
    ASSERT_TRUE(m_Engine.Configure(m_Connector));
    
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
    if (boost::filesystem::exists("config_always_closed.ini"))
    {
        boost::property_tree::ini_parser::read_ini("config_always_closed.ini", aConfig);

        ASSERT_TRUE(m_Connector.Configure(aConfig));
        ASSERT_TRUE(m_Engine.Configure(m_Connector));

        m_Engine.SetGlobalPhase(TradingPhase::CONTINUOUS_TRADING);
        m_Engine.EngineListen();

        ASSERT_EQ(m_Engine.GetGlobalPhase(), TradingPhase::CLOSING_AUCTION);

        sleep(5);
        m_Engine.EngineListen();

        ASSERT_EQ(m_Engine.GetGlobalPhase(), TradingPhase::CLOSE);
    }
    else
    {
        ASSERT_FALSE(true);
    }
}

TEST_F(MatchingEngineTest, Phases)
{
    ASSERT_TRUE(m_Engine.Configure(m_Connector));

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
    ASSERT_TRUE(m_Engine.Configure(m_Connector));

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
    ASSERT_TRUE(m_Engine.Configure(m_Connector));

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
    ASSERT_TRUE(m_Engine.Configure(m_Connector));

    Order OrderBuy(OrderWay::BUY, 1000, 1254, 1, 2);
    Order OrderSell(OrderWay::SELL, 1000, 1254, 3, 4);

    m_Engine.SetGlobalPhase(TradingPhase::CONTINUOUS_TRADING);


    auto pOrderBook = m_Engine.GetOrderBook(1);
    ASSERT_NE(pOrderBook, nullptr);
    ASSERT_EQ(pOrderBook->GetInstrumentID(), 1);

    ASSERT_TRUE(m_Engine.Insert(OrderBuy, 1));
    ASSERT_TRUE(m_Engine.Insert(OrderSell, 1));

    // TODO : Add mechanism to compute OpenPrice
    ASSERT_EQ(pOrderBook->GetDealCounter(), 1);
    ASSERT_EQ(pOrderBook->GetDailyVolume(), 1000);
    ASSERT_EQ(pOrderBook->GetTurnover(), 1254000);

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

    m_Engine.EngineListen();
    sleep(5);
    m_Engine.EngineListen();

    ASSERT_EQ(pOrderBook->GetTradingPhase(), TradingPhase::CONTINUOUS_TRADING);
}

int main(int argc, char ** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    auto ret = RUN_ALL_TESTS();
    LoggerHolder & Logger = LoggerHolder::GetInstance();
    Logger.Kill();
    return ret;
}
