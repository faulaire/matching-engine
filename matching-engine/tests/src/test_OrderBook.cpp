#include <gtest/gtest.h>

#include <unistd.h>

#include <Engine_MatchingEngine.h>
#include <Logger.h>

using namespace exchange::engine;

// To use a test fixture, derive a class from testing::Test.
class OrderBookTest : public testing::Test
{
public:

    using OrderBookType = OrderBook<Order, MatchingEngine>;

    OrderBookTest()
        : m_OrderBook("MingYiCorporation", 1, 10, m_Engine)
    {}

    virtual void SetUp()
    {
        auto & Logger = LoggerHolder::GetInstance();

        boost::property_tree::ptree         aConfig;

        if (boost::filesystem::exists("config.ini"))
        {
            boost::property_tree::ini_parser::read_ini("config.ini", aConfig);
            Logger.Init(aConfig);
        }
    }

protected:

    MatchingEngine m_Engine;
    OrderBookType  m_OrderBook;
};

TEST_F(OrderBookTest, InvalidOrders)
{
    ASSERT_TRUE(m_OrderBook.SetTradingPhase(TradingPhase::CONTINUOUS_TRADING));

    Order OrderBuy(OrderWay::BUY, 0, 1000, 1, 5);
    std::cout << OrderBuy << std::endl;

    Order OrderSell(OrderWay::SELL, 0, 1000, 1, 5);
    std::cout << OrderSell << std::endl;

    ASSERT_TRUE(OrderBuy == OrderBuy);

    ASSERT_FALSE( m_OrderBook.Insert(OrderBuy));

    OrderBuy.SetQuantity(1000);
    OrderBuy.SetPrice(0);

    ASSERT_FALSE( m_OrderBook.Insert(OrderBuy));

    Order WeirdOrder((OrderWay)27, 1000, 1000, 1, 5);
    ASSERT_FALSE( m_OrderBook.Insert(WeirdOrder));
    std::cout << WeirdOrder << std::endl;
}

TEST_F(OrderBookTest, Phases)
{
    ASSERT_TRUE(m_OrderBook.SetTradingPhase(TradingPhase::INTRADAY_AUCTION));
    ASSERT_TRUE(m_OrderBook.SetTradingPhase(TradingPhase::INTRADAY_AUCTION));

    ASSERT_TRUE(m_OrderBook.SetTradingPhase(TradingPhase::CONTINUOUS_TRADING));

    ASSERT_FALSE(m_OrderBook.SetTradingPhase((TradingPhase)-7572));
    ASSERT_FALSE(m_OrderBook.SetTradingPhase((TradingPhase)6843));
}

int main(int argc, char ** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
