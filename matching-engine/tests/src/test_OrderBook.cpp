#include <gtest/gtest.h>

#include <unistd.h>

#include <Engine_MatchingEngine.h>
#include <Logger.h>

using namespace exchange::engine;



class OrderBookTest : public testing::Test
{
public:

    using engine_type = exchange::engine::MatchingEngine<>;

    using OrderBookType = OrderBook<Order, engine_type>;

    OrderBookTest():
        m_Instrument{ "MingYiCorporation", "ISIN", "EUR", 1, 1000 }
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
            ASSERT_TRUE(false);
        }  
    }

protected:

    Instrument<Order>                                    m_Instrument;
    boost::property_tree::ptree                          m_Config;
    std::unique_ptr<engine_type>                         m_pEngine;
    std::unique_ptr<OrderBookType>                       m_pOrderBook;
};

// ENH_TODO  : For later, orders must be rejected if the price is outside the reservation range

TEST_F(OrderBookTest, Should_post_auction_price_be_the_previous_close_price_when_no_auctions_occurs)
{
    ASSERT_EQ(m_Instrument.GetClosePrice(), m_pOrderBook->GetPostAuctionPrice());
}

TEST_F(OrderBookTest, Should_open_price_be_the_price_computed_after_opening_auction)
{
    ASSERT_TRUE(m_pOrderBook->SetTradingPhase(TradingPhase::OPENING_AUCTION));

    auto post_opening_auction_price = 150;

    Order OrderBuy(OrderWay::BUY, 100, post_opening_auction_price, 1, 5);
    Order OrderSell(OrderWay::SELL, 100, post_opening_auction_price, 1, 5);

    ASSERT_TRUE(m_pOrderBook->Insert(OrderBuy));
    ASSERT_TRUE(m_pOrderBook->Insert(OrderSell));

    ASSERT_TRUE(m_pOrderBook->SetTradingPhase(TradingPhase::CONTINUOUS_TRADING));

    ASSERT_EQ(post_opening_auction_price, m_pOrderBook->GetOpenPrice());
}

TEST_F(OrderBookTest, Should_close_price_be_the_price_computed_after_closing_auction)
{
    ASSERT_TRUE(m_pOrderBook->SetTradingPhase(TradingPhase::CLOSING_AUCTION));

    auto post_closing_auction_price = 150;

    Order OrderBuy(OrderWay::BUY, 100, post_closing_auction_price, 1, 5);
    Order OrderSell(OrderWay::SELL, 100, post_closing_auction_price, 1, 5);

    ASSERT_TRUE(m_pOrderBook->Insert(OrderBuy));
    ASSERT_TRUE(m_pOrderBook->Insert(OrderSell));

    ASSERT_TRUE(m_pOrderBook->SetTradingPhase(TradingPhase::CLOSE));

    ASSERT_EQ(post_closing_auction_price, m_pOrderBook->GetClosePrice());
}

TEST_F(OrderBookTest, Should_post_auction_price_be_the_price_computed_after_a_closing_auction)
{
    ASSERT_TRUE(m_pOrderBook->SetTradingPhase(TradingPhase::CLOSING_AUCTION));

    auto post_closing_auction_price = 150;

    Order OrderBuy(OrderWay::BUY, 100, post_closing_auction_price, 1, 5);
    Order OrderSell(OrderWay::SELL, 100, post_closing_auction_price, 1, 5);

    ASSERT_TRUE(m_pOrderBook->Insert(OrderBuy));
    ASSERT_TRUE(m_pOrderBook->Insert(OrderSell));

    ASSERT_TRUE(m_pOrderBook->SetTradingPhase(TradingPhase::CLOSE));

    ASSERT_EQ(post_closing_auction_price, m_pOrderBook->GetPostAuctionPrice());
}

TEST_F(OrderBookTest, Should_post_auction_price_be_the_price_computed_after_an_opening_auction)
{
    ASSERT_TRUE(m_pOrderBook->SetTradingPhase(TradingPhase::OPENING_AUCTION));

    auto post_opening_auction_price = 150;

    Order OrderBuy(OrderWay::BUY, 100, post_opening_auction_price, 1, 5);
    Order OrderSell(OrderWay::SELL, 100, post_opening_auction_price, 1, 5);

    ASSERT_TRUE(m_pOrderBook->Insert(OrderBuy));
    ASSERT_TRUE(m_pOrderBook->Insert(OrderSell));

    ASSERT_TRUE(m_pOrderBook->SetTradingPhase(TradingPhase::CONTINUOUS_TRADING));

    ASSERT_EQ(post_opening_auction_price, m_pOrderBook->GetPostAuctionPrice());
}

TEST_F(OrderBookTest, Should_post_auction_price_be_the_price_computed_after_a_intraday_auction)
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
    ASSERT_TRUE(m_pOrderBook->SetTradingPhase(TradingPhase::CONTINUOUS_TRADING));

    ASSERT_EQ(to_low_price, m_pOrderBook->GetPostAuctionPrice());
}

TEST_F(OrderBookTest, Should_post_auction_price_not_be_modified_when_regular_deal)
{
    const auto post_auction_price = m_pOrderBook->GetPostAuctionPrice();
    const auto regular_deal_price = post_auction_price + 1;

    ASSERT_TRUE(m_pOrderBook->SetTradingPhase(TradingPhase::CONTINUOUS_TRADING));

    Order OrderBuy(OrderWay::BUY, 100, regular_deal_price, 1, 5);
    Order OrderSell(OrderWay::SELL, 100, regular_deal_price, 1, 5);

    ASSERT_TRUE(m_pOrderBook->Insert(OrderBuy));
    ASSERT_TRUE(m_pOrderBook->Insert(OrderSell));

    ASSERT_EQ(post_auction_price, m_pOrderBook->GetPostAuctionPrice());
}

TEST_F(OrderBookTest, Should_open_price_not_be_modified_when_regular_deal)
{
    const auto post_auction_price = m_pOrderBook->GetPostAuctionPrice();
    const auto open_price         = m_pOrderBook->GetOpenPrice();

    auto regular_deal_price = 0;

    if (open_price == post_auction_price)
    {
        regular_deal_price = open_price + 1;
    }
    else
    {
        regular_deal_price = post_auction_price;
    }

    ASSERT_TRUE(m_pOrderBook->SetTradingPhase(TradingPhase::CONTINUOUS_TRADING));

    Order OrderBuy(OrderWay::BUY, 100, regular_deal_price, 1, 5);
    Order OrderSell(OrderWay::SELL, 100, regular_deal_price, 1, 5);

    ASSERT_TRUE(m_pOrderBook->Insert(OrderBuy));
    ASSERT_TRUE(m_pOrderBook->Insert(OrderSell));

    ASSERT_EQ(open_price, m_pOrderBook->GetOpenPrice());
}
TEST_F(OrderBookTest, Should_close_price_not_be_modified_when_regular_deal)
{
    const auto post_auction_price = m_pOrderBook->GetPostAuctionPrice();
    const auto close_price = m_pOrderBook->GetClosePrice();

    auto regular_deal_price = 0;

    if (close_price == post_auction_price)
    {
        regular_deal_price = close_price + 1;
    }
    else
    {
        regular_deal_price = post_auction_price;
    }

    ASSERT_TRUE(m_pOrderBook->SetTradingPhase(TradingPhase::CONTINUOUS_TRADING));

    Order OrderBuy(OrderWay::BUY, 100, regular_deal_price, 1, 5);
    Order OrderSell(OrderWay::SELL, 100, regular_deal_price, 1, 5);

    ASSERT_TRUE(m_pOrderBook->Insert(OrderBuy));
    ASSERT_TRUE(m_pOrderBook->Insert(OrderSell));

    ASSERT_EQ(close_price, m_pOrderBook->GetClosePrice());
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
    Order OrderSell(OrderWay::SELL, 1000, 0, 1, 5);

    EXINFO(OrderBuy);
    EXINFO(OrderSell);

    ASSERT_FALSE(m_pOrderBook->Insert(OrderBuy));
    ASSERT_FALSE(m_pOrderBook->Insert(OrderSell));
}

TEST_F(OrderBookTest, Should_order_be_rejected_when_way_is_invalid)
{
    ASSERT_TRUE(m_pOrderBook->SetTradingPhase(TradingPhase::CONTINUOUS_TRADING));
    Order OrderBuy(OrderWay::MAX_WAY, 1000, 0, 1, 5);
    Order WeirOrder((OrderWay)27, 1000, 0, 1, 5);

    EXINFO(OrderBuy);
    EXINFO(WeirOrder);

    ASSERT_FALSE(m_pOrderBook->Insert(OrderBuy));
    ASSERT_FALSE(m_pOrderBook->Insert(WeirOrder));
}

TEST_F(OrderBookTest, Should_turnover_be_updated_after_a_deal)
{
    const auto post_auction_price = m_pOrderBook->GetPostAuctionPrice();
    const auto order_quantity     = 100;

    ASSERT_TRUE(m_pOrderBook->SetTradingPhase(TradingPhase::CONTINUOUS_TRADING));

    Order OrderBuy(OrderWay::BUY, order_quantity, post_auction_price, 1, 5);
    Order OrderSell(OrderWay::SELL, order_quantity, post_auction_price, 1, 5);

    const auto current_turnover = m_pOrderBook->GetTurnover();

    ASSERT_TRUE(m_pOrderBook->Insert(OrderBuy));
    ASSERT_TRUE(m_pOrderBook->Insert(OrderSell));

    const auto new_turnover = current_turnover + order_quantity*post_auction_price;

    ASSERT_EQ(new_turnover, m_pOrderBook->GetTurnover());
}

TEST_F(OrderBookTest, Should_dailyvolume_be_updated_after_a_deal)
{
    const auto post_auction_price = m_pOrderBook->GetPostAuctionPrice();
    const auto order_quantity = 100;

    ASSERT_TRUE(m_pOrderBook->SetTradingPhase(TradingPhase::CONTINUOUS_TRADING));

    Order OrderBuy(OrderWay::BUY, order_quantity, post_auction_price, 1, 5);
    Order OrderSell(OrderWay::SELL, order_quantity, post_auction_price, 1, 5);

    const auto current_dailyvolume = m_pOrderBook->GetDailyVolume();

    ASSERT_TRUE(m_pOrderBook->Insert(OrderBuy));
    ASSERT_TRUE(m_pOrderBook->Insert(OrderSell));

    const auto new_dailyvolume = current_dailyvolume + order_quantity;

    ASSERT_EQ(new_dailyvolume, m_pOrderBook->GetDailyVolume());
}

TEST_F(OrderBookTest, Should_last_price_be_the_previous_close_price_when_no_auctions_occurs)
{
    ASSERT_EQ(m_Instrument.GetClosePrice(), m_pOrderBook->GetLastPrice());
}

TEST_F(OrderBookTest, Should_last_price_be_updated_after_a_deal)
{
    const auto previous_last_price = m_pOrderBook->GetLastPrice();
    const auto new_last_price = previous_last_price + 1;

    const auto order_quantity = 100;

    ASSERT_TRUE(m_pOrderBook->SetTradingPhase(TradingPhase::CONTINUOUS_TRADING));

    Order OrderBuy(OrderWay::BUY, order_quantity, new_last_price, 1, 5);
    Order OrderSell(OrderWay::SELL, order_quantity, new_last_price, 1, 5);

    ASSERT_TRUE(m_pOrderBook->Insert(OrderBuy));
    ASSERT_TRUE(m_pOrderBook->Insert(OrderSell));

    ASSERT_EQ(new_last_price, m_pOrderBook->GetLastPrice());
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

TEST_F(OrderBookTest, Should_orderbook_be_monitored_when_switching_from_continuous_trading_to_intraday_auction)
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

    ASSERT_EQ(1, m_pEngine->GetMonitoredOrderBookCounter());
}

TEST_F(OrderBookTest, Should_orderbook_be_unmonitored_when_switching_from_intraday_auction_to_closing_auction)
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

    ASSERT_EQ(1, m_pEngine->GetMonitoredOrderBookCounter());

    ASSERT_EQ(TradingPhase::INTRADAY_AUCTION, m_pOrderBook->GetTradingPhase());
    ASSERT_TRUE(m_pOrderBook->SetTradingPhase(TradingPhase::CLOSING_AUCTION));

    ASSERT_EQ(0, m_pEngine->GetMonitoredOrderBookCounter());
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
