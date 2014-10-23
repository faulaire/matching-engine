#include <gtest/gtest.h>

#include <Logger.h>
#include <ConfigurationMgr.h>

TEST(ConfigurationMgrTest, Init)
{
    using namespace exchange::common;

    DataBaseConnector aConnector;

    boost::property_tree::ptree aConfig;
    if (boost::filesystem::exists("config.ini"))
    {
        boost::property_tree::ini_parser::read_ini("config.ini", aConfig);

        ASSERT_TRUE(aConnector.Configure(aConfig));

        auto & aCfgMgr = ConfigurationMgr::GetInstance();

        ASSERT_TRUE(aCfgMgr.Init(aConnector));

        aConnector.SetServer("NotExistingServer");
        aConnector.Disconnect();

        ASSERT_FALSE(aCfgMgr.Init(aConnector));

    }
    else
    {
        std::cerr << "Main : Unable to read the config file. Aborting..." << std::endl;
        ASSERT_FALSE(true);
    }
}

TEST(ConfigurationMgrTest, GetField)
{
    using namespace exchange::common;

    DataBaseConnector aConnector;

    boost::property_tree::ptree aConfig;
    if (boost::filesystem::exists("config.ini"))
    {
        boost::property_tree::ini_parser::read_ini("config.ini", aConfig);

        ASSERT_TRUE(aConnector.Configure(aConfig));

        auto & aCfgMgr = ConfigurationMgr::GetInstance();

        ASSERT_TRUE(aCfgMgr.Init(aConnector));
        
        size_t opening_auction_duration = 0;
        ASSERT_TRUE(aCfgMgr.GetField("engine", "opening_auction_duration", opening_auction_duration));

        std::string start_time;
        ASSERT_TRUE(aCfgMgr.GetField("engine", "start_time", start_time));

        size_t invalid_type;
        ASSERT_FALSE(aCfgMgr.GetField("engine", "start_time", invalid_type));

        ASSERT_FALSE(aCfgMgr.GetField("engine", "not_existing_node", invalid_type));
    }
    else
    {
        std::cerr << "Main : Unable to read the config file. Aborting..." << std::endl;
        ASSERT_FALSE(true);
    }
}

int main(int argc, char ** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}