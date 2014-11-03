#include <Engine_Defines.h>

namespace exchange
{
    namespace engine
    {

        template <typename TOrder>
        OrderBook<TOrder>::OrderBook(const std::string & iSecurityName, UInt32 iInstrumentID)
            : DealHandler(iInstrumentID), m_SecurityName(iSecurityName), m_Orders(*this), m_Phase(TradingPhase::CLOSE), m_Turnover(0),
            m_DailyVolume(0), m_OpenPrice(0), m_LastClosePrice(0)
        {
        }

        template <typename TOrder>
        OrderBook<TOrder>::~OrderBook()
        {}

        template <typename TOrder>
        template <typename Msg>
        bool OrderBook<TOrder>::CheckOrder(const Msg & iMsg) const
        {
            if (iMsg.GetQuantity() < constants::MinQty || iMsg.GetQuantity() > constants::MaxQty)
            {
                return false;
            }

            if (iMsg.GetPrice() < constants::MinPrice || iMsg.GetQuantity() > constants::MaxPrice)
            {
                return false;
            }
            return true;
        }

        template <typename TOrder>
        inline bool OrderBook<TOrder>::SetTradingPhase(TradingPhase iNewPhase)
        {
            if( iNewPhase == m_Phase )
            {
                return true;
            }

            if (iNewPhase < TradingPhase::PHASES_SIZE && iNewPhase >= TradingPhase::OPENING_AUCTION)
            {
                EXINFO("OrderBook::SetTradingPhase " << m_SecurityName << " : switching from phase " << TradingPhaseToString(m_Phase) <<
                    " to phase " << TradingPhaseToString(iNewPhase));

                if( IsAuctionPhase(m_Phase) && !IsAuctionPhase(iNewPhase) )
                {
                    m_Orders.MatchOrders();
                }
                m_Phase = iNewPhase;
                return true;
            }
            else
            {
                EXERR("OrderBook::SetTradingPhase Invalid input phase : " << TradingPhaseToString(iNewPhase));
                return false;
            }
        }

        template <typename TOrder>
        bool OrderBook<TOrder>::Insert(TOrder & iOrder)
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

        template <typename TOrder>
        template <typename TOrderReplace>
        bool OrderBook<TOrder>::Modify(TOrderReplace & iOrderReplace)
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

        template <typename TOrder>
        bool OrderBook<TOrder>::Delete(UInt32 iOrderID, UInt32 iClientID, OrderWay iWay)
        {
            if (m_Phase != TradingPhase::CLOSE)
            {
                return m_Orders.Delete(iOrderID, iClientID, iWay);
            }
            return false;
        }

        template<typename TOrder>
        inline bool OrderBook<TOrder>::IsAuctionPhase(const TradingPhase iPhase) const
        {
            return ( iPhase == TradingPhase::OPENING_AUCTION) ||
                   ( iPhase == TradingPhase::CLOSING_AUCTION) ||
                   ( iPhase == TradingPhase::INTRADAY_AUCTION);
        }

        template<typename TOrder>
        std::ostream& operator<< (std::ostream& oss, const OrderBook<TOrder> & iOrders)
        {
            oss << "TradingPhase[" << TradingPhaseToString(iOrders.m_Phase) << "] ; "
                << "SecurityName[" << iOrders.m_SecurityName << "] ; "
                << "TurnOver" << iOrders.m_Turnover << "] ; "
                << "DailyVolume[" << iOrders.m_DailyVolume << "] ; "
                << "OpenPrice[" << iOrders.m_OpenPrice << "] ; "
                << "LastClosePrice[" << iOrders.m_LastClosePrice << "]" << std::endl;

            oss << iOrders.m_Orders;
            return oss;
        }
    }
}
