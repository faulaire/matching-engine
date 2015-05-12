#include <gtest/gtest.h>

#include <vector>
#include <map>

#include <Engine_Order.h>
#include <Engine_OrderContainer.h>
#include <Logger.h>

#include <functional>

using namespace exchange::engine;

/**
    class EventHandler
*/
class EventHandler
{
    public:

        typedef std::map<std::uint32_t, std::unique_ptr<Deal> > DealContainerType;

    public:
        EventHandler()
        {}

        void OnDeal(std::unique_ptr<Deal> ipDeal)
        {
            m_Deals.emplace(m_Deals.size(), std::move(ipDeal));

            ASSERT_EQ(*ipDeal, *ipDeal);
        }

        /**/
        void OnUnsolicitedCancelledOrder(const Order & /* order*/ )
        {}

        void Reset()
        {
            m_Deals.clear();
        }

        const DealContainerType & GetDealContainer() const { return m_Deals; }

    private:
        DealContainerType       m_Deals;
};

// To use a test fixture, derive a class from testing::Test.
class OrderContainerTest : public testing::Test
{
    protected:

        typedef OrderContainer<Order, EventHandler>      OrderContainerType;
        typedef OrderContainerType::LimitContainer       LimitContainerType;
        typedef LimitContainerType::value_type           LimiteType;

    protected:
        OrderContainerTest():m_Container(m_EventHandler)
        {}

        void DisplayOrders()
        {
            m_Container.SetViewMode(OrderContainerType::ViewMode::VM_BY_ORDER);
            std::cout << m_Container << std::endl;

            m_Container.SetViewMode(OrderContainerType::ViewMode::VM_BY_PRICE);
            std::cout << m_Container << std::endl;
        }

        void InsertOrders()
        {
            for (auto & order : m_BidOrders)
            {
                ASSERT_TRUE(m_Container.Insert(order));
                ASSERT_FALSE(m_Container.Insert(order));
            }

            for (auto & order : m_AskOrders)
            {
                ASSERT_TRUE(m_Container.Insert(order));
                ASSERT_FALSE(m_Container.Insert(order));
            }
        }


        virtual void SetUp()
        {
            boost::property_tree::ptree         aConfig;

            if (boost::filesystem::exists("config.ini"))
            {
                boost::property_tree::ini_parser::read_ini("config.ini", aConfig);
            }


            m_BidContainerReference = {
                                            LimiteType(2, 11000_qty, 2185_price), LimiteType(1, 4000_qty, 1325_price),
                                            LimiteType(1, 3000_qty, 1321_price), LimiteType(2, 3000_qty, 1234_price)
                                      };

            
            m_AskContainerReference = {
                                            LimiteType(2, 15000_qty, 4321_price), LimiteType(1, 6000_qty, 4526_price),
                                            LimiteType(1, 5000_qty, 4580_price), LimiteType(2, 7000_qty, 8526_price)
                                      };

            m_BidOrders = {
                                { OrderWay::BUY, 1000_qty, 1234_price, 1, 5 }, { OrderWay::BUY, 2000_qty, 1234_price, 1, 6 },
                                { OrderWay::BUY, 3000_qty, 1321_price, 1, 7 }, { OrderWay::BUY, 4000_qty, 1325_price, 1, 8 },
                                { OrderWay::BUY, 5000_qty, 2185_price, 1, 9 }, { OrderWay::BUY, 6000_qty, 2185_price, 1, 10 }
                          };

            m_AskOrders = {
                                { OrderWay::SELL, 8000_qty, 4321_price, 2, 1 }, { OrderWay::SELL, 7000_qty, 4321_price, 2, 2 },
                                { OrderWay::SELL, 6000_qty, 4526_price, 2, 3 }, { OrderWay::SELL, 5000_qty, 4580_price, 2, 4 },
                                { OrderWay::SELL, 4000_qty, 8526_price, 2, 5 }, { OrderWay::SELL, 3000_qty, 8526_price, 2, 6 }
                          };
        }

        
        OrderContainerType  m_Container;
        EventHandler        m_EventHandler;

        LimitContainerType  m_BidContainerReference;
        LimitContainerType  m_AskContainerReference;

        std::vector<Order>  m_BidOrders;
        std::vector<Order>  m_AskOrders;
};

TEST_F(OrderContainerTest, AuctionInsert)
{
    InsertOrders();

    OrderContainerType::LimitContainer BidContainer;
    OrderContainerType::LimitContainer AskContainer;

    m_Container.AggregatedView(BidContainer, AskContainer);

    std::vector<Order> ByOrderBidContainer;
    std::vector<Order> ByOrderAskContainer;

    m_Container.ByOrderView(ByOrderBidContainer, ByOrderAskContainer);

    ASSERT_EQ(ByOrderAskContainer.size(), 6);
    ASSERT_EQ(ByOrderBidContainer.size(), 6);

    ASSERT_EQ(ByOrderAskContainer[0], Order(OrderWay::SELL, 8000_qty, 4321_price, 2, 1));
    ASSERT_EQ(ByOrderAskContainer[1], Order(OrderWay::SELL, 7000_qty, 4321_price, 2, 2));

    ASSERT_EQ(ByOrderBidContainer[0], Order(OrderWay::BUY, 5000_qty, 2185_price, 1, 9));
    ASSERT_EQ(ByOrderBidContainer[1], Order(OrderWay::BUY, 6000_qty, 2185_price, 1, 10));

    ASSERT_TRUE(BidContainer == m_BidContainerReference);
    ASSERT_TRUE(AskContainer == m_AskContainerReference);
}

TEST_F(OrderContainerTest, AuctionDelete)
{
    InsertOrders();
    
    ASSERT_TRUE(m_Container.Delete(1, 5, OrderWay::BUY));
    ASSERT_FALSE(m_Container.Delete(1, 5, OrderWay::BUY));

    ASSERT_TRUE(m_Container.Delete(1, 10, OrderWay::BUY));

    ASSERT_TRUE(m_Container.Delete(2, 3, OrderWay::SELL));
    ASSERT_FALSE(m_Container.Delete(2, 3, OrderWay::SELL));

    ASSERT_TRUE(m_Container.Delete(2, 4, OrderWay::SELL));

    OrderContainerType::LimitContainer BidContainer;
    OrderContainerType::LimitContainer AskContainer;

    m_Container.AggregatedView(BidContainer, AskContainer);

    /*
    
    Transition from 

    |         BID          |         ASK         |
    |                      |                     |
    |  2   11000@2185      |  2   15000@4321     |
    |  1   4000@1325       |  1   6000@4526      |
    |  1   3000@1321       |  1   5000@4580      |
    |  2   3000@1234       |  2   7000@8526      |

    To :

    |         BID          |         ASK         |
    |                      |                     |
    |  1   5000@2185       |  2   15000@4321     |
    |  1   4000@1325       |  2   7000@8526      |
    |  1   3000@1321       |              0      |
    |  1   2000@1234       |              0      |
    
    */

    m_BidContainerReference = {
                                LimiteType(1, 5000_qty, 2185_price), LimiteType(1, 4000_qty, 1325_price),
                                LimiteType(1, 3000_qty, 1321_price), LimiteType(1, 2000_qty, 1234_price)
                              };


    m_AskContainerReference = {
                                LimiteType(2, 15000_qty, 4321_price), LimiteType(2, 7000_qty, 8526_price)
                              };

    ASSERT_TRUE(BidContainer == m_BidContainerReference);
    ASSERT_TRUE(AskContainer == m_AskContainerReference);
}

TEST_F(OrderContainerTest, AuctionModify)
{
    InsertOrders();

    /* OrderWay iWay, qty_type iQty, price_type iPrice, std::uint32_t iExistingOrderID, std::uint32_t iReplacedID, std::uint32_t iClientID */
    OrderReplace ReplaceBuy(OrderWay::BUY, 1337_qty, 2185_price, 1, 2, 8);
    ASSERT_TRUE(m_Container.Modify(ReplaceBuy));

    OrderReplace ReplaceSell(OrderWay::SELL, 3000_qty, 4526_price, 2, 12, 4);
    ASSERT_TRUE(m_Container.Modify(ReplaceSell));

    /*
        Transition from

        |         BID          |         ASK         |
        |                      |                     |
        |  2   11000@2185      |  2   15000@4321      |
        |  1   4000@1325       |  1   6000@4526      |
        |  1   3000@1321       |  1   5000@4580      |
        |  2   3000@1234       |  2   7000@8526      |

        To : 

        |         BID          |         ASK         |
        |                      |                     |
        |  3   12337@2185      |  2   15000@4321      |
        |  1   3000@1321       |  2   9000@4526      |
        |  2   3000@1234       |  2   7000@8526      |

    */

    OrderContainerType::LimitContainer BidContainer;
    OrderContainerType::LimitContainer AskContainer;

    m_Container.AggregatedView(BidContainer, AskContainer);

    m_BidContainerReference = {
                                    LimiteType(3, 12337_qty, 2185_price), LimiteType(1, 3000_qty, 1321_price),
                                    LimiteType(2, 3000_qty, 1234_price)
                              };


    m_AskContainerReference = {
                                    LimiteType(2, 15000_qty, 4321_price), LimiteType(2, 9000_qty, 4526_price),
                                    LimiteType(2, 7000_qty, 8526_price)
                              };

    ASSERT_TRUE(BidContainer == m_BidContainerReference);
    ASSERT_TRUE(AskContainer == m_AskContainerReference);

}

TEST_F(OrderContainerTest, AuctionFixing)
{
    m_Container.CancelAllOrders();

    m_BidOrders = {
        { OrderWay::BUY, 1200_qty, 90_price, 1, 5 }, { OrderWay::BUY, 350_qty, 89_price, 1, 6 },
        { OrderWay::BUY, 150_qty, 88_price, 1, 7 }, { OrderWay::BUY, 230_qty, 87_price, 1, 8 }
                  };

    m_AskOrders = {
        { OrderWay::SELL, 900_qty, 90_price, 2, 1 }, { OrderWay::SELL, 650_qty, 91_price, 2, 2 },
        { OrderWay::SELL, 500_qty, 92_price, 2, 3 }, { OrderWay::SELL, 350_qty, 93_price, 2, 4 },
        { OrderWay::SELL, 400_qty, 94_price, 2, 5 }
                  };

    InsertOrders();

    auto OpenPrice = m_Container.GetTheoriticalAuctionInformations();

    ASSERT_EQ(std::get<0>(OpenPrice), 90_price);
    ASSERT_EQ(std::get<1>(OpenPrice), 900_qty);

    m_Container.CancelAllOrders();

    m_BidOrders = {
                        { OrderWay::BUY, 200_qty, 41_price, 3, 5 }, { OrderWay::BUY, 300_qty, 40_price, 3, 6 },
                        { OrderWay::BUY, 150_qty, 39_price, 3, 7 }, { OrderWay::BUY, 50_qty, 38_price, 3, 8 },
                        { OrderWay::BUY, 10_qty, 37_price, 3, 9 }
                  };

    m_AskOrders = {
                        { OrderWay::SELL, 100_qty, 35_price, 4, 1 }, { OrderWay::SELL, 200_qty, 36_price, 4, 2 },
                        { OrderWay::SELL, 50_qty, 37_price, 4, 3 }, { OrderWay::SELL, 200_qty, 39_price, 4, 4 },
                        { OrderWay::SELL, 20_qty, 40_price, 4, 5 }
                  };

    InsertOrders();

    OpenPrice = m_Container.GetTheoriticalAuctionInformations();

    ASSERT_EQ(std::get<0>(OpenPrice), 39_price);
    ASSERT_EQ(std::get<1>(OpenPrice), 550_qty);
}


TEST_F(OrderContainerTest, AuctionMatching)
{
    m_Container.CancelAllOrders();

    m_BidOrders = {
                        { OrderWay::BUY, 1200_qty, 90_price, 1, 5 }, { OrderWay::BUY, 350_qty, 89_price, 1, 6 },
                        { OrderWay::BUY, 150_qty, 88_price, 1, 7 }, { OrderWay::BUY, 230_qty, 87_price, 1, 8 }
                  };

    m_AskOrders = {
                        { OrderWay::SELL, 900_qty, 90_price, 2, 1 }, { OrderWay::SELL, 650_qty, 91_price, 2, 2 },
                        { OrderWay::SELL, 500_qty, 92_price, 2, 3 }, { OrderWay::SELL, 350_qty, 93_price, 2, 4 },
                        { OrderWay::SELL, 400_qty, 94_price, 2, 5 }
                  };

    InsertOrders();
    
    m_Container.MatchOrders();

    auto & DealContainer = m_EventHandler.GetDealContainer();

    ASSERT_EQ(DealContainer.size(), 1);

    ASSERT_EQ(*DealContainer.at(0), Deal(90_price, 900_qty, 5, 1, 1, 2));

    m_EventHandler.Reset();
    m_Container.CancelAllOrders();

    m_BidOrders = {
        { OrderWay::BUY, 200_qty, 41_price, 1, 9 }, { OrderWay::BUY, 300_qty, 40_price, 1, 10 },
        { OrderWay::BUY, 150_qty, 39_price, 1, 11 }, { OrderWay::BUY, 50_qty, 38_price, 1, 12 },
        { OrderWay::BUY, 10_qty, 37_price, 1, 13 }
                  };

    m_AskOrders = {
        { OrderWay::SELL, 100_qty, 35_price, 2, 6 }, { OrderWay::SELL, 200_qty, 36_price, 2, 7 },
        { OrderWay::SELL, 50_qty, 37_price, 2, 8 }, { OrderWay::SELL, 200_qty, 39_price, 2, 9 },
        { OrderWay::SELL, 20_qty, 40_price, 2, 10 }
                  };

    InsertOrders();

    m_Container.MatchOrders();

    ASSERT_EQ(DealContainer.size(), 6);

    ASSERT_EQ(*DealContainer.at(0), Deal(39_price, 100_qty, 9, 1, 6, 2));
    ASSERT_EQ(*DealContainer.at(1), Deal(39_price, 100_qty, 9, 1, 7, 2));
    ASSERT_EQ(*DealContainer.at(2), Deal(39_price, 100_qty, 10, 1, 7, 2));
    ASSERT_EQ(*DealContainer.at(3), Deal(39_price, 50_qty, 10, 1, 8, 2));
    ASSERT_EQ(*DealContainer.at(4), Deal(39_price, 150_qty, 10, 1, 9, 2));
    ASSERT_EQ(*DealContainer.at(5), Deal(39_price, 50_qty, 11, 1, 9, 2));

    m_EventHandler.Reset();
    m_Container.CancelAllOrders();
}


TEST_F(OrderContainerTest, InsertMatching)
{
    auto & DealContainer = m_EventHandler.GetDealContainer();

    m_Container.CancelAllOrders();

    m_BidOrders = {
                        { OrderWay::BUY, 350_qty, 89_price, 1, 6 }, { OrderWay::BUY, 150_qty, 88_price, 1, 7 },
                        { OrderWay::BUY, 230_qty, 87_price, 1, 8 }
                  };

    m_AskOrders = {
                        { OrderWay::SELL, 900_qty, 90_price, 2, 1 }, { OrderWay::SELL, 650_qty, 91_price, 2, 2 },
                        { OrderWay::SELL, 500_qty, 92_price, 2, 3 }, { OrderWay::SELL, 350_qty, 93_price, 2, 4 }
                  };

    InsertOrders();

    Order BuyOrder(OrderWay::BUY, 123_qty, 88_price, 5, 1);
    Order SellOrder(OrderWay::SELL, 123_qty, 91_price, 6, 1);

    ASSERT_TRUE(m_Container.Insert(BuyOrder, true));
    ASSERT_TRUE(m_Container.Insert(SellOrder, true));

    ASSERT_EQ(DealContainer.size(), 0);

    BuyOrder = Order(OrderWay::BUY, 500_qty, 91_price, 7, 1);
    SellOrder = Order(OrderWay::SELL, 150_qty, 89_price, 8, 1);

    ASSERT_TRUE(m_Container.Insert(BuyOrder, true));
    ASSERT_TRUE(m_Container.Insert(SellOrder, true));

    ASSERT_EQ(DealContainer.size(), 2);

    BuyOrder = Order(OrderWay::BUY, 2500_qty, 93_price, 9, 1);
    SellOrder = Order(OrderWay::SELL, 1500_qty, 87_price, 10, 1);

    ASSERT_TRUE(m_Container.Insert(BuyOrder, true));

    ASSERT_TRUE(m_Container.Insert(SellOrder, true));

    m_BidContainerReference = {};

    m_AskContainerReference = { LimiteType(1, 320_qty, 87_price) };

    OrderContainerType::LimitContainer BidContainer;
    OrderContainerType::LimitContainer AskContainer;

    m_Container.AggregatedView(BidContainer, AskContainer);

    ASSERT_TRUE(BidContainer == m_BidContainerReference);
    ASSERT_TRUE(AskContainer == m_AskContainerReference);

    m_EventHandler.Reset();
}

TEST_F(OrderContainerTest, ModifyMatching)
{
    auto & DealContainer = m_EventHandler.GetDealContainer();

    m_Container.CancelAllOrders();

    m_BidOrders = {
                        { OrderWay::BUY, 350_qty, 89_price, 1, 6 }, { OrderWay::BUY, 150_qty, 88_price, 1, 7 },
                        { OrderWay::BUY, 230_qty, 87_price, 1, 8 }
                  };

    m_AskOrders = {
                        { OrderWay::SELL, 900_qty, 90_price, 2, 1 }, { OrderWay::SELL, 650_qty, 91_price, 2, 2 },
                        { OrderWay::SELL, 500_qty, 92_price, 2, 3 }, { OrderWay::SELL, 350_qty, 93_price, 2, 4 }
                  };

    InsertOrders();

    OrderReplace BuyReplace(OrderWay::BUY, 500_qty, 88_price, 1, 2, 6);
    OrderReplace SellReplace(OrderWay::SELL, 1200_qty, 91_price, 2, 4, 1);


    ASSERT_TRUE(m_Container.Modify(BuyReplace, true));
    ASSERT_TRUE(m_Container.Modify(SellReplace, true));
    
    BuyReplace = OrderReplace(OrderWay::BUY, 2000_qty, 91_price, 1, 5, 8);

    ASSERT_TRUE(m_Container.Modify(BuyReplace, true));

    ASSERT_EQ( DealContainer.size(), 2 );

    ASSERT_EQ(*DealContainer.at(0), Deal(91_price, 650_qty, 8, 5, 2, 2));
    ASSERT_EQ(*DealContainer.at(1), Deal(91_price, 1200_qty, 8, 5, 1, 4));
    
    SellReplace = OrderReplace(OrderWay::SELL, 500_qty, 91_price, 2, 8, 4);

    ASSERT_TRUE(m_Container.Modify(SellReplace, true));

    m_BidContainerReference = { LimiteType(2, 650_qty, 88_price) };

    m_AskContainerReference = { LimiteType(1, 350_qty, 91_price), LimiteType(1, 500_qty, 92_price) };

    OrderContainerType::LimitContainer BidContainer;
    OrderContainerType::LimitContainer AskContainer;

    m_Container.AggregatedView(BidContainer, AskContainer);

    ASSERT_TRUE(BidContainer == m_BidContainerReference);
    ASSERT_TRUE(AskContainer == m_AskContainerReference);

    ASSERT_EQ( DealContainer.size(), 3 );
    ASSERT_EQ(*DealContainer.at(2), Deal(91_price, 150_qty, 8, 5, 4, 8));

    m_EventHandler.Reset();
}

TEST_F(OrderContainerTest, Modified_orders_should_be_removed_from_order_book_when_Fully_filled)
{
    InsertOrders();

    OrderReplace BuyReplace(OrderWay::BUY, 4000_qty, 4321_price, 1, 2, 8);
    
    ASSERT_TRUE(m_Container.Modify(BuyReplace, true));

    m_BidContainerReference = {
                                    LimiteType(2, 11000_qty, 2185_price), LimiteType(1, 3000_qty, 1321_price),
                                    LimiteType(2, 3000_qty, 1234_price)
                              };


    m_AskContainerReference = {
                                    LimiteType(2, 11000_qty, 4321_price), LimiteType(1, 6000_qty, 4526_price),
                                    LimiteType(1, 5000_qty, 4580_price), LimiteType(2, 7000_qty, 8526_price)
                              };

    OrderContainerType::LimitContainer BidContainer;
    OrderContainerType::LimitContainer AskContainer;

    m_Container.AggregatedView(BidContainer, AskContainer);

    ASSERT_EQ(m_BidContainerReference, BidContainer);
    ASSERT_EQ(m_AskContainerReference, AskContainer);
}

TEST_F(OrderContainerTest, Orders_should_loses_their_prority_when_modified)
{
    m_Container.CancelAllOrders();

    m_BidOrders = {
                        { OrderWay::BUY, 100_qty, 10_price, 5, 6 }, { OrderWay::BUY, 200_qty, 11_price, 5, 7 },
                        { OrderWay::BUY, 300_qty, 12_price, 5, 8 }, { OrderWay::BUY, 300_qty, 13_price, 5, 9 }
                  };


    m_AskOrders = {
                        { OrderWay::SELL, 100_qty, 10_price, 10, 6 }, { OrderWay::SELL, 200_qty, 11_price, 10, 7 },
                        { OrderWay::SELL, 300_qty, 12_price, 10, 8 }, { OrderWay::SELL, 400_qty, 13_price, 10, 9 }
                  };

    InsertOrders();

    OrderReplace BuyReplace(OrderWay::BUY, 300_qty, 11_price, 5, 15, 8);
    OrderReplace SellReplace(OrderWay::SELL, 400_qty, 12_price, 10, 20, 9);

    ASSERT_TRUE(m_Container.Modify(BuyReplace, false));
    ASSERT_TRUE(m_Container.Modify(SellReplace, false));

    std::vector<Order> ByOrderBidContainer;
    std::vector<Order> ByOrderAskContainer;

    m_Container.ByOrderView(ByOrderBidContainer, ByOrderAskContainer);
    
    ASSERT_EQ(ByOrderBidContainer[0], Order(OrderWay::BUY, 300_qty, 13_price, 5, 9));
    ASSERT_EQ(ByOrderBidContainer[1], Order(OrderWay::BUY, 200_qty, 11_price, 5, 7));
    ASSERT_EQ(ByOrderBidContainer[2], Order(OrderWay::BUY, 300_qty, 11_price, 15, 8));
    ASSERT_EQ(ByOrderBidContainer[3], Order(OrderWay::BUY, 100_qty, 10_price, 5, 6));
    
    ASSERT_EQ(ByOrderAskContainer[0], Order(OrderWay::SELL, 100_qty, 10_price, 10, 6));
    ASSERT_EQ(ByOrderAskContainer[1], Order(OrderWay::SELL, 200_qty, 11_price, 10, 7));
    ASSERT_EQ(ByOrderAskContainer[2], Order(OrderWay::SELL, 300_qty, 12_price, 10, 8));
    ASSERT_EQ(ByOrderAskContainer[3], Order(OrderWay::SELL, 400_qty, 12_price, 20, 9));
}

TEST_F(OrderContainerTest, Order_insertion_should_fail_when_order_id_already_used)
{
    Order ob(OrderWay::BUY, 8000_qty, 4321_price, 1, 1);
    ASSERT_TRUE(m_Container.Insert(ob));
    ASSERT_FALSE(m_Container.Insert(ob));

    ASSERT_TRUE(m_Container.Delete(1, 1, OrderWay::BUY));

    ASSERT_FALSE(m_Container.Insert(ob));
}

TEST_F(OrderContainerTest, Order_modification_should_fail_when_order_id_already_used)
{
    Order ob(OrderWay::BUY, 8000_qty, 4321_price, 1, 1);
    Order os(OrderWay::SELL, 8000_qty, 4321_price, 2, 1);

    ASSERT_TRUE(m_Container.Insert(ob));
    ASSERT_TRUE(m_Container.Insert(os));
    
    OrderReplace BuyReplace(OrderWay::BUY, 4000_qty, 4321_price, 1, 2, 1);

    ASSERT_FALSE(m_Container.Modify(BuyReplace));
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
