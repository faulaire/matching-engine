#include <gtest/gtest.h>

#include <unistd.h>

#include <Engine_MatchingEngine.h>
#include <Logger.h>

using namespace exchange::engine;

class OrderBookTest : public testing::Test
{
public:

    using OrderBookType = OrderBook<Order, MatchingEngine>;

    OrderBookTest():
        m_Instrument{ "MingYiCorporation", "ISIN", "EUR", 1, 1000 }

    {}

    virtual void SetUp()
    {
        m_pEngine.reset(new exchange::engine::MatchingEngine());

        m_pOrderBook.reset(new OrderBookType(m_Instrument, *m_pEngine));

        if (boost::filesystem::exists("config.ini"))
        {
            boost::property_tree::ini_parser::read_ini("config.ini", m_Config);
            ASSERT_TRUE(m_pEngine->Configure(m_Config));
        }
        else
        {
            ASSERT_TRUE(false);
        }  
    }

protected:

    Instrument<Order>                                    m_Instrument;
    boost::property_tree::ptree                          m_Config;
    std::unique_ptr<MatchingEngine>                      m_pEngine;
    std::unique_ptr<OrderBookType>                       m_pOrderBook;
};

// TODO  : For later, orders must be rejected if the price is outside the reservation range
// TODO  : Add a some tests to handle everything related to turnover / dailyvolume ...
// TODO  : Check that the PostAuctionClosePrice in all cases => opening / intraday and closing auction

TEST_F(OrderBookTest, Should_post_auction_price_be_the_previous_close_price_when_no_auctions_occurs)
{
    ASSERT_EQ(m_Instrument.GetClosePrice(), m_pOrderBook->GetPostAuctionPrice());
}

TEST_F(OrderBookTest, Should_phase_switch_to_intraday_aution_when_deal_price_is_higher_than_max_deviation)
{
    ASSERT_TRUE(m_pOrderBook->SetTradingPhase(TradingPhase::CONTINUOUS_TRADING));

    auto MaxPriceDev = m_Config.get<unsigned int>("Engine.max_price_deviation");
    MaxPriceDev++;

    auto ref_price = m_pOrderBook->GetPostAuctionPrice();
    auto to_low_price = ref_price * (1 - MaxPriceDev*0.01);

    Order OrderBuy(OrderWay::BUY, 100, to_low_price, 1, 5);
    Order OrderSell(OrderWay::SELL, 100, to_low_price, 1, 5);

    ASSERT_TRUE(m_pOrderBook->Insert(OrderBuy));
    ASSERT_TRUE(m_pOrderBook->Insert(OrderSell));

    ASSERT_EQ(TradingPhase::INTRADAY_AUCTION, m_pOrderBook->GetTradingPhase());
}

TEST_F(OrderBookTest, Should_phase_switch_to_intraday_aution_when_deal_price_is_lower_than_max_deviation)
{
    ASSERT_TRUE(m_pOrderBook->SetTradingPhase(TradingPhase::CONTINUOUS_TRADING));

    auto MaxPriceDev = m_Config.get<unsigned int>("Engine.max_price_deviation");
    MaxPriceDev++;

    auto ref_price = m_pOrderBook->GetPostAuctionPrice();
    auto to_high_price = ref_price * (1 + MaxPriceDev*0.01);

    Order OrderBuy(OrderWay::BUY, 100, to_high_price, 1, 5);
    Order OrderSell(OrderWay::SELL, 100, to_high_price, 1, 5);

    ASSERT_TRUE(m_pOrderBook->Insert(OrderBuy));
    ASSERT_TRUE(m_pOrderBook->Insert(OrderSell));

    ASSERT_EQ(TradingPhase::INTRADAY_AUCTION, m_pOrderBook->GetTradingPhase());
}

TEST_F(OrderBookTest, Should_order_be_rejected_when_quantity_is_null)
{
    ASSERT_TRUE(m_pOrderBook->SetTradingPhase(TradingPhase::CONTINUOUS_TRADING));
    Order OrderBuy(OrderWay::BUY, 0, 1000, 1, 5);
    Order OrderSell(OrderWay::BUY, 0, 1000, 1, 5);

    ASSERT_FALSE(m_pOrderBook->Insert(OrderBuy));
    ASSERT_FALSE(m_pOrderBook->Insert(OrderSell));
}

TEST_F(OrderBookTest, Should_order_be_rejected_when_price_is_null)
{
    ASSERT_TRUE(m_pOrderBook->SetTradingPhase(TradingPhase::CONTINUOUS_TRADING));
    Order OrderBuy(OrderWay::BUY, 1000, 0, 1, 5);
    Order OrderSell(OrderWay::BUY, 1000, 0, 1, 5);

    ASSERT_FALSE(m_pOrderBook->Insert(OrderBuy));
    ASSERT_FALSE(m_pOrderBook->Insert(OrderSell));
}

TEST_F(OrderBookTest, Should_order_be_rejected_when_way_is_invalid)
{
    ASSERT_TRUE(m_pOrderBook->SetTradingPhase(TradingPhase::CONTINUOUS_TRADING));
    Order OrderBuy(OrderWay::MAX_WAY, 1000, 0, 1, 5);
    Order WeirOrder((OrderWay)27, 1000, 0, 1, 5);

    ASSERT_FALSE(m_pOrderBook->Insert(OrderBuy));
    ASSERT_FALSE(m_pOrderBook->Insert(WeirOrder));
}

TEST_F(OrderBookTest, Should_set_trading_phase_success_when_valid_phases)
{
    ASSERT_TRUE(m_pOrderBook->SetTradingPhase(TradingPhase::CLOSE));
    ASSERT_TRUE(m_pOrderBook->SetTradingPhase(TradingPhase::OPENING_AUCTION));
    ASSERT_TRUE(m_pOrderBook->SetTradingPhase(TradingPhase::CONTINUOUS_TRADING));
    ASSERT_TRUE(m_pOrderBook->SetTradingPhase(TradingPhase::INTRADAY_AUCTION));
    ASSERT_TRUE(m_pOrderBook->SetTradingPhase(TradingPhase::CLOSING_AUCTION));
    ASSERT_TRUE(m_pOrderBook->SetTradingPhase(TradingPhase::CLOSE));
}

TEST_F(OrderBookTest, Should_set_trading_phase_FAIL_when_invalid_phases)
{
    ASSERT_FALSE(m_pOrderBook->SetTradingPhase((TradingPhase)-7572));
    ASSERT_FALSE(m_pOrderBook->SetTradingPhase((TradingPhase)6843));
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
