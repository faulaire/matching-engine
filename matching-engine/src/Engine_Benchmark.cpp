#include <memory>
#include <forward_list>

#include <Engine_Order.h>
#include <Engine_MatchingEngine.h>

using namespace exchange::engine;

using engine_type = exchange::engine::MatchingEngine<>;
using OrderBookType = engine_type::OrderBookType;


#define CREATE_ORDER(Way, Qty, Price, OrderID, ClientID) (  std::make_unique<Order>(Way, Qty, Price, OrderID, ClientID) )
#define INSERT_ORDER(OrderBook, iOrder) ( OrderBook->Insert( std::move(iOrder) ) )

int main(int argc, char ** argv)
{
    if( argc < 2)
    {
        std::cerr << "Usage : " << argv[0] << " NbOrderToInsert" << std::endl;
        return 1;
    }

    auto nb_order_to_insert = atoi(argv[1]);

    Instrument<Order>              Instrument{ "MingYiCorporation", "ISIN", "EUR", 1, 100_price };
    boost::property_tree::ptree    Config;
    auto                           pEngine = std::make_unique<engine_type>();
    auto                           pOrderBook = std::make_unique<OrderBookType>(Instrument, *pEngine);

    if (boost::filesystem::exists("config.ini"))
    {
        boost::property_tree::ini_parser::read_ini("config.ini", Config);
        pEngine->Configure(Config);
    }
    else
    {
        std::cerr << "No such file or directory : config.ini" << std::endl;
        return 2;
    }

    pOrderBook->SetTradingPhase(TradingPhase::CONTINUOUS_TRADING);

    std::forward_list< std::unique_ptr<Order> > m_Orders;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, 100);

    for (auto i = 0; i < nb_order_to_insert; i++)
    {
        m_Orders.push_front(CREATE_ORDER(OrderWay::BUY, Quantity(dis(gen)), Price(dis(gen)), ClientOrderID(i + 1), 5_clientid));
        m_Orders.push_front(CREATE_ORDER(OrderWay::SELL, Quantity(dis(gen)), Price(dis(gen)), ClientOrderID(i + 1), 6_clientid));
    }

    std::string s;

    pOrderBook->RehashOrderIndexes(nb_order_to_insert);
    pOrderBook->RehashDealIndexes(2*nb_order_to_insert);

    std::cout << "Enable callgrind" << std::endl;
    std::cin >> s;

    std::chrono::time_point<std::chrono::high_resolution_clock > start, end;
    start = std::chrono::high_resolution_clock::now();

    for (auto & pOrder : m_Orders)
    {
        INSERT_ORDER(pOrderBook, pOrder);
    }

    end = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double> elapsed_seconds = end - start;
    std::time_t end_time = std::chrono::high_resolution_clock::to_time_t(end);

    std::cout << "finished computation at " << std::ctime(&end_time)
    << "elapsed time: " << elapsed_seconds.count() << "s" << std::endl;
    std::cout << "number of generated deals : " << pOrderBook->GetDealCounter() << std::endl;

    std::cout << "Disable callgrind" << std::endl;
    std::cin >> s;

    return 0;
}
