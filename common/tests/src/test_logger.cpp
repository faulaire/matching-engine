#include <gtest/gtest.h>

#include <Logger.h>

typedef enum 
{
	Session = 0,
	Server,
	Matching,
	Gateway
} LogCat;

TEST(LoggerTest, Configuration)
{
    using namespace exchange;

	boost::property_tree::ptree aConfig;
	if( boost::filesystem::exists("config.ini") )
	{
		boost::property_tree::ini_parser::read_ini("config.ini", aConfig);
	}
	else
	{
		ASSERT_TRUE(false) << "Unable to read the config file. Aborting...";
	}
	
	LoggerHolder & Logger = LoggerHolder::GetInstance();

	/*
		Session and Server are already available in the configuration file,
		verbosity will no be overwritten. Matching and Gateway are new categories and the 
		default verbosity will be applied
	*/
	Logger.AddCategory(Session,"Session",common::logger::LOW);
	Logger.AddCategory(Server,"Server",common::logger::MEDIUM);
	Logger.AddCategory(Matching, "Matching", common::logger::MEDIUM);
	Logger.AddCategory(Gateway, "Gateway", common::logger::INSANE);

	Logger.Init(aConfig);
}

TEST(LoggerTest, Reporting)
{
    using namespace exchange;

	LoggerHolder & Logger = LoggerHolder::GetInstance();

	ASSERT_FALSE(Logger.IsReporting(Session, common::logger::LOW));
	ASSERT_TRUE(Logger.IsReporting(Session, common::logger::MEDIUM));
	ASSERT_TRUE(Logger.IsReporting(Session, common::logger::HIGH));
	ASSERT_TRUE(Logger.IsReporting(Session, common::logger::INSANE));

	ASSERT_FALSE(Logger.IsReporting(Server, common::logger::LOW));
	ASSERT_FALSE(Logger.IsReporting(Server, common::logger::MEDIUM));
	ASSERT_TRUE(Logger.IsReporting(Server, common::logger::HIGH));
	ASSERT_TRUE(Logger.IsReporting(Server, common::logger::INSANE));

	ASSERT_FALSE(Logger.IsReporting(Matching, common::logger::LOW));
	ASSERT_TRUE(Logger.IsReporting(Matching, common::logger::MEDIUM));
	ASSERT_TRUE(Logger.IsReporting(Matching, common::logger::HIGH));
	ASSERT_TRUE(Logger.IsReporting(Matching, common::logger::INSANE));

	ASSERT_FALSE(Logger.IsReporting(Gateway, common::logger::LOW));
	ASSERT_FALSE(Logger.IsReporting(Gateway, common::logger::MEDIUM));
	ASSERT_FALSE(Logger.IsReporting(Gateway, common::logger::HIGH));
	ASSERT_TRUE(Logger.IsReporting(Gateway, common::logger::INSANE));
}

TEST(LoggerTest, Streaming)
{
    using namespace exchange;

	EXINFO("Steaming from EXINFO"); 
	EXWARN("Steaming from EXWARN");  
	EXERR("Steaming from EXERR");  
	EXPANIC("Steaming from EXPANIC"); 

	EXLOG(Session, common::logger::LOW, "Session::LOW : Streaming from EXLOG");
	EXLOG(Session, common::logger::MEDIUM, "Session::MEDIUM : Streaming from EXLOG");
	EXLOG(Session, common::logger::HIGH, "Session::HIGH : Streaming from EXLOG");
	EXLOG(Session, common::logger::INSANE, "Session::INSANE : Streaming from EXLOG");

	EXLOG(Server, common::logger::LOW, "Server::LOW : Streaming from EXLOG");
	EXLOG(Server, common::logger::MEDIUM, "Server::MEDIUM : Streaming from EXLOG");
	EXLOG(Server, common::logger::HIGH, "Server::HIGH : Streaming from EXLOG");
	EXLOG(Server, common::logger::INSANE, "Server::INSANE :Streaming from EXLOG");

	EXLOG(Matching, common::logger::LOW, "Matching::LOW : Streaming from EXLOG");
	EXLOG(Matching, common::logger::MEDIUM, "Matching::MEDIUM : Streaming from EXLOG");
	EXLOG(Matching, common::logger::HIGH, "Matching::HIGH : Streaming from EXLOG");
	EXLOG(Matching, common::logger::INSANE, "Matching::INSANE :Streaming from EXLOG");

	EXLOG(Gateway, common::logger::LOW, "Gateway::LOW : Streaming from EXLOG");
	EXLOG(Gateway, common::logger::MEDIUM, "Gateway::MEDIUM : Streaming from EXLOG");
	EXLOG(Gateway, common::logger::HIGH, "Gateway::HIGH : Streaming from EXLOG");
	EXLOG(Gateway, common::logger::INSANE, "Gateway::INSANE :Streaming from EXLOG");
}

int main(int argc, char ** argv)
{
	::testing::InitGoogleTest(&argc, argv);
	auto ret = RUN_ALL_TESTS();	
	LoggerHolder & Logger = LoggerHolder::GetInstance();
	Logger.Kill();
	return ret;
}