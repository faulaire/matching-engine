#include <gtest/gtest.h>

#include <unistd.h>

#include <Logger.h>

#include <Engine_Instrument.h>
#include <Engine_Order.h>

using namespace exchange::engine;

class InstrumentManagerTest : public testing::Test
{
public:

    using key_extractor_type = std::function < const std::string &(const Instrument<Order> &) >;

public:

	static const unsigned product_id = 1;

	virtual void SetUp()
	{
		m_InstrumentCounter = 0;

		boost::filesystem::remove_all(m_DBFilePath);
	}

	void InstrumentHandler(const Instrument<Order> & /*Instrument*/)
	{
		m_InstrumentCounter++;
	}

protected:

    const key_extractor_type  key_extractor = [](const Instrument<Order> & Instrument) -> const std::string &
    {
        return Instrument.GetName();
    };

	const std::string								     m_DBFilePath = "/tmp/InstrumentDatabase";

	unsigned int									     m_InstrumentCounter;
	boost::property_tree::ptree                          m_Config;
};


TEST_F(InstrumentManagerTest, Should_load_success_when_valid_database)
{
	InstrumentManager<Order> InstrMgr(m_DBFilePath, key_extractor);

	auto instr_handler = [this](const auto & instr) { this->InstrumentHandler(instr); };
	ASSERT_EQ(true, InstrMgr.Load(instr_handler));
}

TEST_F(InstrumentManagerTest, Should_load_return_nothing_when_empty_database)
{
	InstrumentManager<Order> InstrMgr(m_DBFilePath, key_extractor);

	auto instr_handler = [this](const auto & instr) { this->InstrumentHandler(instr); };
	InstrMgr.Load(instr_handler);

	ASSERT_EQ(0, m_InstrumentCounter);
}

TEST_F(InstrumentManagerTest, Should_instrument_insertion_success_when_new_instrument)
{
	InstrumentManager<Order> InstrMgr(m_DBFilePath, key_extractor);

	Instrument<Order> Michelin{ "Michelin", "ISINMICH", "EUR", 1, 1254 };

	ASSERT_EQ(true, InstrMgr.Write(Michelin));
}

TEST_F(InstrumentManagerTest, Should_instrument_insertion_fail_when_already_inserted_instrument)
{
	InstrumentManager<Order> InstrMgr(m_DBFilePath, key_extractor);

	Instrument<Order> Michelin{ "Michelin", "ISINMICH", "EUR", 1, 1254 };

	ASSERT_EQ(true, InstrMgr.Write(Michelin));
	ASSERT_EQ(false, InstrMgr.Write(Michelin));
}

TEST_F(InstrumentManagerTest, Should_load_return_n_instruments_when_n_instrument_inserted)
{
	InstrumentManager<Order> InstrMgr(m_DBFilePath, key_extractor);

	Instrument<Order> Michelin{ "Michelin", "ISINMICH", "EUR", 1, 1254 };
	Instrument<Order> Natixis{ "Natixis", "ISINATIX", "JPY", 2, 1255 };
	Instrument<Order> Ibm{ "IBM", "ISINIBM", "USD", 3, 1256 };

	ASSERT_EQ(true, InstrMgr.Write(Michelin));
	ASSERT_EQ(true, InstrMgr.Write(Natixis));
	ASSERT_EQ(true, InstrMgr.Write(Ibm));

	auto instr_handler = [this](const auto & instr) { this->InstrumentHandler(instr); };
	InstrMgr.Load(instr_handler);

	ASSERT_EQ(3, m_InstrumentCounter);
}

TEST_F(InstrumentManagerTest, Should_load_return_the_same_instrument_as_the_inserted_one)
{
	InstrumentManager<Order> InstrMgr(m_DBFilePath, key_extractor);

	Instrument<Order> Michelin{ "Michelin", "ISINMICH", "EUR", 1, 1254 };

	ASSERT_EQ(true, InstrMgr.Write(Michelin));

	auto instr_handler = [Michelin, this](const Instrument<Order> & instr)
	{ 
		ASSERT_EQ(Michelin, instr);
		this->InstrumentHandler(instr); 
	};

	ASSERT_TRUE(InstrMgr.Load(instr_handler));
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
