#include <gtest/gtest.h>

#include <Logger.h>
#include <MariaDB_Connector.h>

TEST(DataBaseConnectorTest, Basic)
{
    using namespace exchange::common;
    
    DataBaseConnector aConnector;
    
    ASSERT_FALSE(aConnector.IsConnected());

    ASSERT_EQ("", aConnector.GetUserName());
    ASSERT_EQ("", aConnector.GetPassword());
    ASSERT_EQ("", aConnector.GetServer());
    ASSERT_EQ("", aConnector.GetName());

    aConnector.SetUserName("UserName");
    aConnector.SetPassword("PassWord");
    aConnector.SetServer("Server");
    aConnector.SetName("Name");

    ASSERT_EQ("UserName", aConnector.GetUserName());
    ASSERT_EQ("PassWord", aConnector.GetPassword());
    ASSERT_EQ("Server", aConnector.GetServer());
    ASSERT_EQ("Name", aConnector.GetName());
}

TEST(DataBaseConnectorTest, Config)
{
    using namespace exchange::common;

    DataBaseConnector aConnector;

    boost::property_tree::ptree aConfig;
    if (boost::filesystem::exists("config.ini"))
    {
        boost::property_tree::ini_parser::read_ini("config.ini", aConfig);

        ASSERT_TRUE(aConnector.Configure(aConfig));

        ASSERT_EQ("exchange", aConnector.GetUserName());
        ASSERT_EQ("exchange", aConnector.GetPassword());
        ASSERT_EQ("fedorasrv", aConnector.GetServer());
        ASSERT_EQ("exchange", aConnector.GetName());

        boost::property_tree::ptree aInvalidConfig;
        ASSERT_FALSE(aConnector.Configure(aInvalidConfig));
    }
    else
    {
        std::cerr << "Main : Unable to read the config file. Aborting..." << std::endl;
        ASSERT_FALSE(true);
    }
}

TEST(DataBaseConnectorTest, Connect)
{
    using namespace exchange::common;

    boost::property_tree::ptree aConfig;
    if (boost::filesystem::exists("config.ini"))
    {
        DataBaseConnector* pConnector = new DataBaseConnector();

        boost::property_tree::ini_parser::read_ini("config.ini", aConfig);

        ASSERT_TRUE(pConnector->Configure(aConfig));

        ASSERT_TRUE(pConnector->Connect());
        ASSERT_TRUE(pConnector->IsConnected());
        ASSERT_TRUE(pConnector->Connect());

        ASSERT_TRUE(pConnector->Disconnect());
        ASSERT_FALSE(pConnector->IsConnected());
        ASSERT_FALSE(pConnector->Disconnect());

        pConnector->SetServer("NotExistingServer");
        ASSERT_FALSE(pConnector->Connect());

        ASSERT_TRUE(pConnector->Configure(aConfig));

        ASSERT_TRUE(pConnector->Connect());
        delete pConnector;
    }
    else
    {
        std::cerr << "Main : Unable to read the config file. Aborting..." << std::endl;
        ASSERT_FALSE(true);
    }
}

TEST(DataBaseConnectorTest, Query)
{
    using namespace exchange::common;

    DataBaseConnector aConnector;

    boost::property_tree::ptree aConfig;
    if (boost::filesystem::exists("config.ini"))
    {
        boost::property_tree::ini_parser::read_ini("config.ini", aConfig);

        ASSERT_TRUE(aConnector.Configure(aConfig));
        ASSERT_TRUE(aConnector.Connect());
        
        DataBaseConnector::ResultArray Results;
        // Query with results
        ASSERT_TRUE(aConnector.Query("SELECT * FROM users", Results));
        // Query with non results
        ASSERT_TRUE(aConnector.Query("SELECT * FROM `instruments` WHERE currency = \"YUAN\"", Results));

        ASSERT_FALSE(aConnector.Query("SELEEEEEEEEECCCCTTTTTTT....", Results));
        ASSERT_FALSE(aConnector.Query("SELECT * FROM toto", Results));

        ASSERT_TRUE(aConnector.Disconnect());
        ASSERT_FALSE(aConnector.Query("SELECT * FROM users", Results));
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