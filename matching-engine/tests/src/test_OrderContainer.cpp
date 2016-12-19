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

        using deal_ptr_type = std::unique_ptr<Deal>;

    public:
        EventHandler()
        {}

        void OnDeal(std::unique_ptr<Deal> ipDeal)
        {
            m_Deals.emplace(m_Deals.size(), std::move(ipDeal));

            ASSERT_EQ(*ipDeal, *ipDeal);
        }

        /**/
        void OnUnsolicitedCancelledOrder(const Order * /* order*/ )
        {}

        template <typename... Args>
        deal_ptr_type CreateDeal(Args &&... args)
        {
            return std::make_unique<Deal>(std::forward<Args>(args)...);
        }

        void Reset()
        {
            m_Deals.clear();
        }

        const DealContainerType & GetDealContainer() const { return m_Deals; }

    private:
        DealContainerType       m_Deals;
};

#define CREATE_ORDER(Way, Qty, Price, OrderID, ClientID) ( std::make_unique<Order>(Way, Qty, Price, OrderID, ClientID) ) 
#define CREATE_RAW_ORDER(Way, Qty, Price, OrderID, ClientID) ( new Order(Way, Qty, Price, OrderID, ClientID) )

#define CREATE_REPLACE(Way, Qty, Price, OldOrderID, NewOrderID, ClientID) ( std::make_unique<OrderReplace>(Way, Qty, Price, OldOrderID, NewOrderID, ClientID) )

#define INSERT_ORDER(Container, pOrder) ( Container.Insert( std::move(pOrder) ) )
#define INSERT_MATCHING_ORDER(Container, pOrder) ( Container.Insert( std::move(pOrder), true ) )

#define MODIFY_ORDER(Container, pReplace) ( Container.Modify( std::move(pReplace), false ) )
#define MODIFY_MATCHING_ORDER(Container, pReplace) ( Container.Modify( std::move(pReplace), true ) )

// To use a test fixture, derive a class from testing::Test.
class OrderContainerTest : public testing::Test
{
    protected:

        typedef OrderContainer<Order, EventHandler>      OrderContainerType;
        typedef OrderContainerType::LimitContainer       LimitContainerType;
        typedef LimitContainerType::value_type           LimiteType;

        using   order_vector          = std::vector <Order*> ;
        using   smart_order_vector = std::vector < std::unique_ptr<Order> > ;

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
                m_sBidOrders.push_back( std::unique_ptr<Order>(order) );
                ASSERT_EQ(Status::Ok, m_Container.Insert( std::move(*m_sBidOrders.rbegin() )));
            }

            for (auto & order : m_AskOrders)
            {
                m_sAskOrders.push_back(std::unique_ptr<Order>(order));
                ASSERT_EQ(Status::Ok, m_Container.Insert( std::move(*m_sAskOrders.rbegin())));
            }
        }


        virtual void SetUp()
        {
            boost::property_tree::ptree         aConfig;

            if (boost::filesystem::exists("config.ini"))
            {
                boost::property_tree::ini_parser::read_ini("config.ini", aConfig);
            }

            m_sBidOrders.clear();
            m_sAskOrders.clear();

            m_BidContainerReference = {
                                            LimiteType(2, 11000_qty, 2185_price), LimiteType(1, 4000_qty, 1325_price),
                                            LimiteType(1, 3000_qty, 1321_price), LimiteType(2, 3000_qty, 1234_price)
                                      };

            
            m_AskContainerReference = {
                                            LimiteType(2, 15000_qty, 4321_price), LimiteType(1, 6000_qty, 4526_price),
                                            LimiteType(1, 5000_qty, 4580_price), LimiteType(2, 7000_qty, 8526_price)
                                      };

            m_BidOrders = {
                CREATE_RAW_ORDER(OrderWay::BUY, 1000_qty, 1234_price, 1_clorderid, 5_clientid), CREATE_RAW_ORDER(OrderWay::BUY, 2000_qty, 1234_price, 1_clorderid, 6_clientid),
                CREATE_RAW_ORDER(OrderWay::BUY, 3000_qty, 1321_price, 1_clorderid, 7_clientid), CREATE_RAW_ORDER(OrderWay::BUY, 4000_qty, 1325_price, 1_clorderid, 8_clientid),
                CREATE_RAW_ORDER(OrderWay::BUY, 5000_qty, 2185_price, 1_clorderid, 9_clientid), CREATE_RAW_ORDER(OrderWay::BUY, 6000_qty, 2185_price, 1_clorderid, 10_clientid)
            };

            m_AskOrders = {
                CREATE_RAW_ORDER(OrderWay::SELL, 8000_qty, 4321_price, 2_clorderid, 1_clientid), CREATE_RAW_ORDER(OrderWay::SELL, 7000_qty, 4321_price, 2_clorderid, 2_clientid),
                CREATE_RAW_ORDER(OrderWay::SELL, 6000_qty, 4526_price, 2_clorderid, 3_clientid), CREATE_RAW_ORDER(OrderWay::SELL, 5000_qty, 4580_price, 2_clorderid, 4_clientid),
                CREATE_RAW_ORDER(OrderWay::SELL, 4000_qty, 8526_price, 2_clorderid, 5_clientid), CREATE_RAW_ORDER(OrderWay::SELL, 3000_qty, 8526_price, 2_clorderid, 6_clientid)
            };

        }

        
        OrderContainerType  m_Container;
        EventHandler        m_EventHandler;

        LimitContainerType  m_BidContainerReference;
        LimitContainerType  m_AskContainerReference;

        order_vector  m_BidOrders;
        order_vector  m_AskOrders;

        smart_order_vector m_sBidOrders;
        smart_order_vector m_sAskOrders;
};

TEST_F(OrderContainerTest, AuctionInsert)
{
    InsertOrders();

    OrderContainerType::LimitContainer BidContainer;
    OrderContainerType::LimitContainer AskContainer;

    m_Container.AggregatedView(BidContainer, AskContainer);

    std::vector<Order*> ByOrderBidContainer;
    std::vector<Order*> ByOrderAskContainer;

    m_Container.ByOrderView(ByOrderBidContainer, ByOrderAskContainer);

    ASSERT_EQ(ByOrderAskContainer.size(), 6);
    ASSERT_EQ(ByOrderBidContainer.size(), 6);

    ASSERT_EQ(*ByOrderAskContainer[0], Order(OrderWay::SELL, 8000_qty, 4321_price, 2_clorderid, 1_clientid));
    ASSERT_EQ(*ByOrderAskContainer[1], Order(OrderWay::SELL, 7000_qty, 4321_price, 2_clorderid, 2_clientid));

    ASSERT_EQ(*ByOrderBidContainer[0], Order(OrderWay::BUY, 5000_qty, 2185_price, 1_clorderid, 9_clientid));
    ASSERT_EQ(*ByOrderBidContainer[1], Order(OrderWay::BUY, 6000_qty, 2185_price, 1_clorderid, 10_clientid));

    ASSERT_TRUE(BidContainer == m_BidContainerReference);
    ASSERT_TRUE(AskContainer == m_AskContainerReference);
}

TEST_F(OrderContainerTest, AuctionDelete)
{
    InsertOrders();
    
    ASSERT_EQ(Status::Ok, m_Container.Delete(1_clorderid, 5_clientid, OrderWay::BUY));
    ASSERT_EQ(Status::OrderNotFound, m_Container.Delete(1_clorderid, 5_clientid, OrderWay::BUY));

    ASSERT_EQ(Status::Ok, m_Container.Delete(1_clorderid, 10_clientid, OrderWay::BUY));

    ASSERT_EQ(Status::Ok, m_Container.Delete(2_clorderid, 3_clientid, OrderWay::SELL));
    ASSERT_EQ(Status::OrderNotFound, m_Container.Delete(2_clorderid, 3_clientid, OrderWay::SELL));

    ASSERT_EQ(Status::Ok, m_Container.Delete(2_clorderid, 4_clientid, OrderWay::SELL));

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
    auto ReplaceBuy = CREATE_REPLACE(OrderWay::BUY, 1337_qty, 2185_price, 1_clorderid, 2_clorderid, 8_clientid);
    ASSERT_EQ(Status::Ok, MODIFY_ORDER(m_Container, ReplaceBuy));

    

    auto ReplaceSell = CREATE_REPLACE(OrderWay::SELL, 3000_qty, 4526_price, 2_clorderid, 12_clorderid, 4_clientid);
    ASSERT_EQ(Status::Ok, MODIFY_ORDER(m_Container, ReplaceSell));

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
        CREATE_RAW_ORDER(OrderWay::BUY, 1200_qty, 90_price, 1_clorderid, 5_clientid), CREATE_RAW_ORDER(OrderWay::BUY, 350_qty, 89_price, 1_clorderid, 6_clientid),
        CREATE_RAW_ORDER(OrderWay::BUY, 150_qty, 88_price, 1_clorderid, 7_clientid), CREATE_RAW_ORDER(OrderWay::BUY, 230_qty, 87_price, 1_clorderid, 8_clientid)
                  };

    m_AskOrders = {
        CREATE_RAW_ORDER(OrderWay::SELL, 900_qty, 90_price, 2_clorderid, 1_clientid), CREATE_RAW_ORDER(OrderWay::SELL, 650_qty, 91_price, 2_clorderid, 2_clientid),
        CREATE_RAW_ORDER(OrderWay::SELL, 500_qty, 92_price, 2_clorderid, 3_clientid), CREATE_RAW_ORDER(OrderWay::SELL, 350_qty, 93_price, 2_clorderid, 4_clientid),
        CREATE_RAW_ORDER(OrderWay::SELL, 400_qty, 94_price, 2_clorderid, 5_clientid)
                  };
    
    InsertOrders();

    auto OpenPrice = m_Container.GetTheoriticalAuctionInformations();

    ASSERT_EQ(std::get<0>(OpenPrice), 90_price);
    ASSERT_EQ(std::get<1>(OpenPrice), 900_volume);

    m_Container.CancelAllOrders();

    m_BidOrders = {
        CREATE_RAW_ORDER(OrderWay::BUY, 200_qty, 41_price, 3_clorderid, 5_clientid), CREATE_RAW_ORDER(OrderWay::BUY, 300_qty, 40_price, 3_clorderid, 6_clientid),
        CREATE_RAW_ORDER(OrderWay::BUY, 150_qty, 39_price, 3_clorderid, 7_clientid), CREATE_RAW_ORDER(OrderWay::BUY, 50_qty, 38_price, 3_clorderid, 8_clientid),
        CREATE_RAW_ORDER(OrderWay::BUY, 10_qty, 37_price, 3_clorderid, 9_clientid)
                  };
    
    m_AskOrders = {
        CREATE_RAW_ORDER(OrderWay::SELL, 100_qty, 35_price, 4_clorderid, 1_clientid), CREATE_RAW_ORDER(OrderWay::SELL, 200_qty, 36_price, 4_clorderid, 2_clientid),
        CREATE_RAW_ORDER(OrderWay::SELL, 50_qty, 37_price, 4_clorderid, 3_clientid), CREATE_RAW_ORDER(OrderWay::SELL, 200_qty, 39_price, 4_clorderid, 4_clientid),
        CREATE_RAW_ORDER(OrderWay::SELL, 20_qty, 40_price, 4_clorderid, 5_clientid)
                  };

    InsertOrders();

    OpenPrice = m_Container.GetTheoriticalAuctionInformations();

    ASSERT_EQ(std::get<0>(OpenPrice), 39_price);
    ASSERT_EQ(std::get<1>(OpenPrice), 550_volume);
}


TEST_F(OrderContainerTest, AuctionMatching)
{
    m_Container.CancelAllOrders();

    m_BidOrders = {
        CREATE_RAW_ORDER(OrderWay::BUY, 1200_qty, 90_price, 1_clorderid, 5_clientid), CREATE_RAW_ORDER(OrderWay::BUY, 350_qty, 89_price, 1_clorderid, 6_clientid),
        CREATE_RAW_ORDER(OrderWay::BUY, 150_qty, 88_price, 1_clorderid, 7_clientid), CREATE_RAW_ORDER(OrderWay::BUY, 230_qty, 87_price, 1_clorderid, 8_clientid)
                  };

    m_AskOrders = {
        CREATE_RAW_ORDER(OrderWay::SELL, 900_qty, 90_price, 2_clorderid, 1_clientid), CREATE_RAW_ORDER(OrderWay::SELL, 650_qty, 91_price, 2_clorderid, 2_clientid),
        CREATE_RAW_ORDER(OrderWay::SELL, 500_qty, 92_price, 2_clorderid, 3_clientid), CREATE_RAW_ORDER(OrderWay::SELL, 350_qty, 93_price, 2_clorderid, 4_clientid),
        CREATE_RAW_ORDER(OrderWay::SELL, 400_qty, 94_price, 2_clorderid, 5_clientid)
                  };

    InsertOrders();
    
    m_Container.MatchOrders();

    auto & DealContainer = m_EventHandler.GetDealContainer();

    ASSERT_EQ(DealContainer.size(), 1);

    ASSERT_EQ(*DealContainer.at(0), Deal(90_price, 900_qty, 5_clientid, 1_clorderid, 1_clientid, 2_clorderid));

    m_EventHandler.Reset();
    m_Container.CancelAllOrders();

    m_BidOrders = {
        CREATE_RAW_ORDER(OrderWay::BUY, 200_qty, 41_price, 1_clorderid, 9_clientid), CREATE_RAW_ORDER(OrderWay::BUY, 300_qty, 40_price, 1_clorderid, 10_clientid),
        CREATE_RAW_ORDER(OrderWay::BUY, 150_qty, 39_price, 1_clorderid, 11_clientid), CREATE_RAW_ORDER(OrderWay::BUY, 50_qty, 38_price, 1_clorderid, 12_clientid),
        CREATE_RAW_ORDER(OrderWay::BUY, 10_qty, 37_price, 1_clorderid, 13_clientid)
                  };

    m_AskOrders = {
        CREATE_RAW_ORDER(OrderWay::SELL, 100_qty, 35_price, 2_clorderid, 6_clientid), CREATE_RAW_ORDER(OrderWay::SELL, 200_qty, 36_price, 2_clorderid, 7_clientid),
        CREATE_RAW_ORDER(OrderWay::SELL, 50_qty, 37_price, 2_clorderid, 8_clientid), CREATE_RAW_ORDER(OrderWay::SELL, 200_qty, 39_price, 2_clorderid, 9_clientid),
        CREATE_RAW_ORDER(OrderWay::SELL, 20_qty, 40_price, 2_clorderid, 10_clientid)
                  };

    InsertOrders();

    m_Container.MatchOrders();

    ASSERT_EQ(DealContainer.size(), 6);

    ASSERT_EQ(*DealContainer.at(0), Deal(39_price, 100_qty, 9_clientid, 1_clorderid, 6_clientid, 2_clorderid));
    ASSERT_EQ(*DealContainer.at(1), Deal(39_price, 100_qty, 9_clientid, 1_clorderid, 7_clientid, 2_clorderid));
    ASSERT_EQ(*DealContainer.at(2), Deal(39_price, 100_qty, 10_clientid, 1_clorderid, 7_clientid, 2_clorderid));
    ASSERT_EQ(*DealContainer.at(3), Deal(39_price, 50_qty, 10_clientid, 1_clorderid, 8_clientid, 2_clorderid));
    ASSERT_EQ(*DealContainer.at(4), Deal(39_price, 150_qty, 10_clientid, 1_clorderid, 9_clientid, 2_clorderid));
    ASSERT_EQ(*DealContainer.at(5), Deal(39_price, 50_qty, 11_clientid, 1_clorderid, 9_clientid, 2_clorderid));

    m_EventHandler.Reset();
    m_Container.CancelAllOrders();
}


TEST_F(OrderContainerTest, InsertMatching)
{
    auto & DealContainer = m_EventHandler.GetDealContainer();

    m_Container.CancelAllOrders();

    m_BidOrders = {
        CREATE_RAW_ORDER(OrderWay::BUY, 350_qty, 89_price, 1_clorderid, 6_clientid), CREATE_RAW_ORDER(OrderWay::BUY, 150_qty, 88_price, 1_clorderid, 7_clientid),
        CREATE_RAW_ORDER(OrderWay::BUY, 230_qty, 87_price, 1_clorderid, 8_clientid)
                  };

    m_AskOrders = {
        CREATE_RAW_ORDER(OrderWay::SELL, 900_qty, 90_price, 2_clorderid, 1_clientid), CREATE_RAW_ORDER(OrderWay::SELL, 650_qty, 91_price, 2_clorderid, 2_clientid),
        CREATE_RAW_ORDER(OrderWay::SELL, 500_qty, 92_price, 2_clorderid, 3_clientid), CREATE_RAW_ORDER(OrderWay::SELL, 350_qty, 93_price, 2_clorderid, 4_clientid)
                  };


    InsertOrders();

    auto BuyOrder = CREATE_ORDER(OrderWay::BUY, 123_qty, 88_price, 5_clorderid, 1_clientid);
    auto SellOrder = CREATE_ORDER(OrderWay::SELL, 123_qty, 91_price, 6_clorderid, 1_clientid);

    ASSERT_EQ(Status::Ok, INSERT_MATCHING_ORDER(m_Container, BuyOrder));
    ASSERT_EQ(Status::Ok, INSERT_MATCHING_ORDER(m_Container, SellOrder));

    ASSERT_EQ(DealContainer.size(), 0);

    auto BuyOrder1 = CREATE_ORDER(OrderWay::BUY, 500_qty, 91_price, 7_clorderid, 1_clientid);
    auto SellOrder1 = CREATE_ORDER(OrderWay::SELL, 150_qty, 89_price, 8_clorderid, 1_clientid);

    ASSERT_EQ(Status::Ok, INSERT_MATCHING_ORDER(m_Container, BuyOrder1));
    ASSERT_EQ(Status::Ok, INSERT_MATCHING_ORDER(m_Container, SellOrder1));

    ASSERT_EQ(DealContainer.size(), 2);

    auto BuyOrder2 = CREATE_ORDER(OrderWay::BUY, 2500_qty, 93_price, 9_clorderid, 1_clientid);
    auto SellOrder2 = CREATE_ORDER(OrderWay::SELL, 1500_qty, 87_price, 10_clorderid, 1_clientid);

    ASSERT_EQ(Status::Ok, INSERT_MATCHING_ORDER(m_Container, BuyOrder2));
    ASSERT_EQ(Status::Ok, INSERT_MATCHING_ORDER(m_Container, SellOrder2));

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

    m_BidOrders =   {
        CREATE_RAW_ORDER(OrderWay::BUY, 350_qty, 89_price, 1_clorderid, 6_clientid), CREATE_RAW_ORDER(OrderWay::BUY, 150_qty, 88_price, 1_clorderid, 7_clientid),
        CREATE_RAW_ORDER(OrderWay::BUY, 230_qty, 87_price, 1_clorderid, 8_clientid)
                    };

    m_AskOrders = {
        CREATE_RAW_ORDER(OrderWay::SELL, 900_qty, 90_price, 2_clorderid, 1_clientid), CREATE_RAW_ORDER(OrderWay::SELL, 650_qty, 91_price, 2_clorderid, 2_clientid),
        CREATE_RAW_ORDER(OrderWay::SELL, 500_qty, 92_price, 2_clorderid, 3_clientid), CREATE_RAW_ORDER(OrderWay::SELL, 350_qty, 93_price, 2_clorderid, 4_clientid)
                  };

    InsertOrders();

    auto BuyReplace = CREATE_REPLACE(OrderWay::BUY, 500_qty, 88_price, 1_clorderid, 2_clorderid, 6_clientid);
    auto SellReplace = CREATE_REPLACE(OrderWay::SELL, 1200_qty, 91_price, 2_clorderid, 4_clorderid, 1_clientid);

    ASSERT_EQ(Status::Ok, MODIFY_MATCHING_ORDER(m_Container, BuyReplace));
    ASSERT_EQ(Status::Ok, MODIFY_MATCHING_ORDER(m_Container, SellReplace));
    
    BuyReplace = CREATE_REPLACE(OrderWay::BUY, 2000_qty, 91_price, 1_clorderid, 5_clorderid, 8_clientid);

    ASSERT_EQ(Status::Ok, MODIFY_MATCHING_ORDER(m_Container, BuyReplace));

    ASSERT_EQ( DealContainer.size(), 2 );

    ASSERT_EQ(*DealContainer.at(0), Deal(91_price, 650_qty, 8_clientid, 5_clorderid, 2_clientid, 2_clorderid));
    ASSERT_EQ(*DealContainer.at(1), Deal(91_price, 1200_qty, 8_clientid, 5_clorderid, 1_clientid, 4_clorderid));
    
    SellReplace = CREATE_REPLACE(OrderWay::SELL, 500_qty, 91_price, 2_clorderid, 8_clorderid, 4_clientid);

    ASSERT_EQ(Status::Ok, MODIFY_MATCHING_ORDER(m_Container, SellReplace));

    m_BidContainerReference = { LimiteType(2, 650_qty, 88_price) };

    m_AskContainerReference = { LimiteType(1, 350_qty, 91_price), LimiteType(1, 500_qty, 92_price) };

    OrderContainerType::LimitContainer BidContainer;
    OrderContainerType::LimitContainer AskContainer;

    m_Container.AggregatedView(BidContainer, AskContainer);

    ASSERT_TRUE(BidContainer == m_BidContainerReference);
    ASSERT_TRUE(AskContainer == m_AskContainerReference);

    ASSERT_EQ( DealContainer.size(), 3 );
    ASSERT_EQ(*DealContainer.at(2), Deal(91_price, 150_qty, 8_clientid, 5_clorderid, 4_clientid, 8_clorderid));

    m_EventHandler.Reset();
}

TEST_F(OrderContainerTest, Modified_orders_should_be_removed_from_order_book_when_Fully_filled)
{
    InsertOrders();

    auto BuyReplace = CREATE_REPLACE(OrderWay::BUY, 4000_qty, 4321_price, 1_clorderid, 2_clorderid, 8_clientid);
    
    ASSERT_EQ(Status::Ok, MODIFY_MATCHING_ORDER(m_Container, BuyReplace));

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
        CREATE_RAW_ORDER(OrderWay::BUY, 100_qty, 10_price, 5_clorderid, 6_clientid), CREATE_RAW_ORDER(OrderWay::BUY, 200_qty, 11_price, 5_clorderid, 7_clientid),
        CREATE_RAW_ORDER(OrderWay::BUY, 300_qty, 12_price, 5_clorderid, 8_clientid), CREATE_RAW_ORDER(OrderWay::BUY, 300_qty, 13_price, 5_clorderid, 9_clientid)
                  };

    m_AskOrders = {
        CREATE_RAW_ORDER(OrderWay::SELL, 100_qty, 10_price, 10_clorderid, 6_clientid), CREATE_RAW_ORDER(OrderWay::SELL, 200_qty, 11_price, 10_clorderid, 7_clientid),
        CREATE_RAW_ORDER(OrderWay::SELL, 300_qty, 12_price, 10_clorderid, 8_clientid), CREATE_RAW_ORDER(OrderWay::SELL, 400_qty, 13_price, 10_clorderid, 9_clientid)
                  };

    InsertOrders();

    auto BuyReplace = CREATE_REPLACE(OrderWay::BUY, 300_qty, 11_price, 5_clorderid, 15_clorderid, 8_clientid);
    auto SellReplace = CREATE_REPLACE(OrderWay::SELL, 400_qty, 12_price, 10_clorderid, 20_clorderid, 9_clientid);

    ASSERT_EQ(Status::Ok, MODIFY_ORDER(m_Container, BuyReplace));
    ASSERT_EQ(Status::Ok, MODIFY_ORDER(m_Container, SellReplace));

    std::vector<Order*> ByOrderBidContainer;
    std::vector<Order*> ByOrderAskContainer;

    m_Container.ByOrderView(ByOrderBidContainer, ByOrderAskContainer);
    
    ASSERT_EQ(*ByOrderBidContainer[0], Order(OrderWay::BUY, 300_qty, 13_price, 5_clorderid, 9_clientid));
    ASSERT_EQ(*ByOrderBidContainer[1], Order(OrderWay::BUY, 200_qty, 11_price, 5_clorderid, 7_clientid));
    ASSERT_EQ(*ByOrderBidContainer[2], Order(OrderWay::BUY, 300_qty, 11_price, 15_clorderid, 8_clientid));
    ASSERT_EQ(*ByOrderBidContainer[3], Order(OrderWay::BUY, 100_qty, 10_price, 5_clorderid, 6_clientid));
    
    ASSERT_EQ(*ByOrderAskContainer[0], Order(OrderWay::SELL, 100_qty, 10_price, 10_clorderid, 6_clientid));
    ASSERT_EQ(*ByOrderAskContainer[1], Order(OrderWay::SELL, 200_qty, 11_price, 10_clorderid, 7_clientid));
    ASSERT_EQ(*ByOrderAskContainer[2], Order(OrderWay::SELL, 300_qty, 12_price, 10_clorderid, 8_clientid));
    ASSERT_EQ(*ByOrderAskContainer[3], Order(OrderWay::SELL, 400_qty, 12_price, 20_clorderid, 9_clientid));
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
