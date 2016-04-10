#include <gtest/gtest.h>

#include <unistd.h>

#include <Engine_MatchingEngine.h>
#include <Logger.h>

#include <vector>
#include <chrono>
#include <ctime>

using namespace exchange::engine;

class OrderBookBenchmark : public testing::Test
{
public:

    using engine_type = exchange::engine::MatchingEngine<>;

    using OrderBookType = engine_type::OrderBookType;

    OrderBookBenchmark() :
        m_Instrument{ "MingYiCorporation", "ISIN", "EUR", 1, 1000_price }
    {}

    virtual void SetUp()
    {
        EXINFO("OrderBookTest::SetUp : Working with :" << m_Instrument);

        m_pEngine.reset(new engine_type());

        m_pOrderBook.reset(new OrderBookType(m_Instrument, *m_pEngine));

        if (boost::filesystem::exists("config.ini"))
        {
            boost::property_tree::ini_parser::read_ini("config.ini", m_Config);
            ASSERT_TRUE(m_pEngine->Configure(m_Config));
        }
        else
        {
            FAIL();
        }
    }

protected:

    Instrument<Order>                                    m_Instrument;
    boost::property_tree::ptree                          m_Config;
    std::unique_ptr<engine_type>                         m_pEngine;
    std::unique_ptr<OrderBookType>                       m_pOrderBook;
};



#define CREATE_ORDER(Way, Qty, Price, OrderID, ClientID) (  std::make_unique<Order>(Way, Qty, Price, OrderID, ClientID) )
#define INSERT_ORDER(OrderBook, iOrder) ( OrderBook->Insert( iOrder ) )

TEST_F(OrderBookBenchmark, Should_open_price_be_the_price_computed_after_opening_auction)
{
    ASSERT_TRUE(m_pOrderBook->SetTradingPhase(TradingPhase::OPENING_AUCTION));

    std::vector< std::unique_ptr<Order> > m_Orders;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, 100);

    for (auto i = 0; i < 500000; i++)
    {
        m_Orders.push_back(CREATE_ORDER(OrderWay::BUY, Quantity(dis(gen)), Price(dis(gen)), ClientOrderID(i + 1), 5_clientid));
        m_Orders.push_back(CREATE_ORDER(OrderWay::SELL, Quantity(dis(gen)), Price(dis(gen)), ClientOrderID(i + 1), 6_clientid));
    }
    
    m_pOrderBook->RehashIndexes(1000000);

    std::chrono::time_point<std::chrono::system_clock> start, end;
    start = std::chrono::system_clock::now();

    for (auto & pOrder : m_Orders)
    {
        INSERT_ORDER(m_pOrderBook, pOrder);
    }
    
    end = std::chrono::system_clock::now();

    std::chrono::duration<double> elapsed_seconds = end - start;
    std::time_t end_time = std::chrono::system_clock::to_time_t(end);

    std::cout << "finished computation at " << std::ctime(&end_time)
        << "elapsed time: " << elapsed_seconds.count() << "s\n";
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
