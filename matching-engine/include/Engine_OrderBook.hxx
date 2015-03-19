#include <Engine_Defines.h>
#include <Engine_MatchingEngine.h>

namespace exchange
{
    namespace engine
    {

        template <typename TOrder, typename TMatchingEngine>
        OrderBook<TOrder,TMatchingEngine>::OrderBook(const std::string & iSecurityName, std::uint32_t iInstrumentID, price_type iLastClosePrice, TMatchingEngine& rMatchingEngine)
            : DealHandlerType(iInstrumentID), m_rMatchingEngine(rMatchingEngine), m_SecurityName(iSecurityName), m_Orders(*this),
              m_Phase(TradingPhase::CLOSE), m_AuctionStart(), m_LastPrice(iLastClosePrice), m_Turnover(0), m_DailyVolume(0), m_OpenPrice(0), m_LastClosePrice(iLastClosePrice)
        {
        }

        template <typename TOrder, typename TMatchingEngine>
        OrderBook<TOrder,TMatchingEngine>::~OrderBook()
        {}

        template <typename TOrder, typename TMatchingEngine>
        template <typename Msg>
        bool OrderBook<TOrder,TMatchingEngine>::CheckOrder(const Msg & iMsg) const
        {
            if (iMsg.GetQuantity() < constants::MinQty || iMsg.GetQuantity() > constants::MaxQty)
            {
                return false;
            }

            if (iMsg.GetPrice() < constants::MinPrice || iMsg.GetQuantity() > constants::MaxPrice)
            {
                return false;
            }

            if( iMsg.GetWay() != OrderWay::BUY && iMsg.GetWay() != OrderWay::SELL)
            {
                return false;
            }

            return true;
        }

        template <typename TOrder, typename TMatchingEngine>
        inline bool OrderBook<TOrder,TMatchingEngine>::SetTradingPhase(TradingPhase iNewPhase)
        {
            if( iNewPhase == m_Phase )
            {
                return true;
            }

            if (iNewPhase < TradingPhase::PHASES_SIZE && iNewPhase >= TradingPhase::OPENING_AUCTION)
            {
                EXINFO("OrderBook::SetTradingPhase " << m_SecurityName << " : switching from phase " << TradingPhaseToString(m_Phase) <<
                    " to phase " << TradingPhaseToString(iNewPhase) << "]");

                if( IsAuctionPhase(m_Phase) && !IsAuctionPhase(iNewPhase) )
                {
                    m_Orders.MatchOrders();
                }
                m_Phase = iNewPhase;
                // TODO : Register the close price if we switch to CLOSE
                // TODO : Register the OPEN PRICE for opening_auction -> continuous trading switch
                return true;
            }
            else
            {
                EXERR("OrderBook::SetTradingPhase Invalid input phase : " << TradingPhaseToString(iNewPhase));
                return false;
            }
        }

        template <typename TOrder, typename TMatchingEngine>
        bool OrderBook<TOrder,TMatchingEngine>::Insert(TOrder & iOrder)
        {
            if (m_Phase != TradingPhase::CLOSE)
            {
                if (CheckOrder(iOrder))
                {
                    return m_Orders.Insert(iOrder, TradingPhase::CONTINUOUS_TRADING==m_Phase);
                }
            }
            return false;
        }

        template <typename TOrder, typename TMatchingEngine>
        template <typename TOrderReplace>
        bool OrderBook<TOrder,TMatchingEngine>::Modify(TOrderReplace & iOrderReplace)
        {
            if (m_Phase != TradingPhase::CLOSE)
            {
                if (CheckOrder(iOrderReplace))
                {
                    return m_Orders.Modify(iOrderReplace, TradingPhase::CONTINUOUS_TRADING==m_Phase);
                }
            }
            return false;
        }

        template <typename TOrder, typename TMatchingEngine>
        bool OrderBook<TOrder,TMatchingEngine>::Delete(std::uint32_t iOrderID, std::uint32_t iClientID, OrderWay iWay)
        {
            if (m_Phase != TradingPhase::CLOSE)
            {
                return m_Orders.Delete(iOrderID, iClientID, iWay);
            }
            return false;
        }

        template <typename TOrder, typename TMatchingEngine>
        void OrderBook<TOrder,TMatchingEngine>::ProcessDeal(const Deal * ipDeal)
        {
            /*
             * TODO : Must be enhanced : What should we do at the end of the INTRADAY_AUCTION phase ?
             * Register a new price as close price ? not possible.
             * So a ref price must be introduce ( how to compute ? ) in order to perfom the comparison
             */
            SetTurnover( GetTurnover() + ipDeal->GetQuantity()*ipDeal->GetPrice() );
            SetDailyVolume( GetDailyVolume() + ipDeal->GetQuantity() );
            SetLastPrice( ipDeal->GetPrice() );

            auto && PriceDevFactors = m_rMatchingEngine.GetPriceDevFactors();

            price_type min_price = GetLastClosePrice() * std::get<0>(PriceDevFactors);
            price_type max_price = GetLastClosePrice() * std::get<1>(PriceDevFactors);

            if( ipDeal->GetPrice() > max_price || ipDeal->GetPrice() < min_price)
            {
                if( GetTradingPhase() != TradingPhase::INTRADAY_AUCTION)
                {
                    SetTradingPhase(TradingPhase::INTRADAY_AUCTION);
                    m_AuctionStart = boost::posix_time::second_clock::local_time();

                    m_rMatchingEngine.MonitorOrderBook(this);
                }
            }
        }

        template <typename TOrder, typename TMatchingEngine>
        inline bool OrderBook<TOrder,TMatchingEngine>::IsAuctionPhase(const TradingPhase iPhase) const
        {
            return ( iPhase == TradingPhase::OPENING_AUCTION) ||
                   ( iPhase == TradingPhase::CLOSING_AUCTION) ||
                   ( iPhase == TradingPhase::INTRADAY_AUCTION);
        }

        template <typename TOrder, typename TMatchingEngine>
        std::ostream& operator<< (std::ostream& oss, const OrderBook<TOrder,TMatchingEngine> & iOrders)
        {
            oss << "TradingPhase["   << TradingPhaseToString(iOrders.m_Phase) << "] ; "
                << "SecurityName["   << iOrders.m_SecurityName << "] ; "
                << "TurnOver"        << iOrders.m_Turnover << "] ; "
                << "DailyVolume["    << iOrders.m_DailyVolume << "] ; "
                << "OpenPrice["      << iOrders.m_OpenPrice << "] ; "
                << "LastClosePrice[" << iOrders.m_LastClosePrice << "]" << std::endl;

            oss << iOrders.m_Orders;
            return oss;
        }
    }
}
