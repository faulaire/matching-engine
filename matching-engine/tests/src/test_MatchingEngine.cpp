#include <gtest/gtest.h>

#include <unistd.h>

#include <Engine_MatchingEngine.h>

using namespace exchange::engine;

// To use a test fixture, derive a class from testing::Test.
class MatchingEngineTest : public testing::Test
{
    public:

        virtual void SetUp()
        {
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

TEST_F(MatchingEngineTest, PhaseSwitching)
{
    ASSERT_TRUE(m_Engine.Configure(m_Connector));
    
    ASSERT_EQ(m_Engine.GetGlobalPhase(), CLOSE);
    m_Engine.EngineListen();
    ASSERT_EQ(m_Engine.GetGlobalPhase(), OPENING_AUCTION);

    for (auto x = 0; x < 5; x++)
    {
        m_Engine.EngineListen();
        sleep(1);
    }
    ASSERT_EQ(m_Engine.GetGlobalPhase(), CONTINUOUS_TRADING);

    m_Engine.SetGlobalPhase(CLOSING_AUCTION);

    ASSERT_EQ(m_Engine.GetGlobalPhase(), CLOSING_AUCTION);
    for (auto x = 0; x < 5; x++)
    {
        m_Engine.EngineListen();
        sleep(1);
    }
    ASSERT_EQ(m_Engine.GetGlobalPhase(), CLOSE);
}

TEST_F(MatchingEngineTest, InsertClose)
{
    ASSERT_TRUE(m_Engine.Configure(m_Connector));

    Order o(BUY, 1000, 1234, 1, 5);

    ASSERT_FALSE(m_Engine.Insert(o, 1));
    ASSERT_FALSE(m_Engine.Insert(o, 17));

    OrderReplace Replace(BUY, 2000, 1234, 1, 2, 5);
    ASSERT_FALSE(m_Engine.Modify(Replace, 1));
    ASSERT_FALSE(m_Engine.Modify(Replace, 2));

    ASSERT_FALSE(m_Engine.Delete(2, 5, BUY, 1));
    ASSERT_FALSE(m_Engine.Delete(2, 5, BUY, 1));
}

TEST_F(MatchingEngineTest, InsertAuction)
{
    ASSERT_TRUE(m_Engine.Configure(m_Connector));

    Order o(BUY, 1000, 1234, 1, 5);

    // Switching to auction phase
    m_Engine.EngineListen();

    ASSERT_TRUE(m_Engine.Insert(o, 1));
    ASSERT_FALSE(m_Engine.Insert(o, 1));

    OrderReplace Replace(BUY, 2000, 1234, 1, 2, 5);
    ASSERT_TRUE(m_Engine.Modify(Replace, 1));
    ASSERT_FALSE(m_Engine.Modify(Replace, 2));

    ASSERT_TRUE(m_Engine.Delete(2, 5, BUY, 1));
    ASSERT_FALSE(m_Engine.Delete(2, 5, BUY, 1));
}

int main(int argc, char ** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    auto ret = RUN_ALL_TESTS();
    LoggerHolder & Logger = LoggerHolder::GetInstance();
    Logger.Kill();
    return ret;
}