#include <Engine_Defines.h>

namespace exchange
{
    namespace engine
    {

        template<typename TOrder>
        const bool OrderBook<TOrder>::MatchingMapping[PHASES_SIZE] = { false, true, false, false, false };

        template <typename TOrder>
        OrderBook<TOrder>::OrderBook(const std::string & iSecurityName, UInt32 iInstrumentID)
            : DealHandler(iInstrumentID), m_SecurityName(iSecurityName), m_Orders(*this), m_Phase(CLOSE), m_Turnover(0),
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
            if (iNewPhase < PHASES_SIZE)
            {
                EXINFO("OrderBook::SetTradingPhase " << m_SecurityName << " : switching from phase " << TradingPhaseToString(m_Phase) <<
                    " to phase " << TradingPhaseToString(iNewPhase));

                m_Phase = iNewPhase;
                if (iNewPhase == CONTINUOUS_TRADING)
                {
                    m_Orders.MatchOrders();
                }
                return true;
            }
            else
            {
                EXERR("OrderBook::SetTradingPhase Invalid input phase : " << iNewPhase);
                return false;
            }
        }

        template <typename TOrder>
        bool OrderBook<TOrder>::Insert(TOrder & iOrder)
        {
            if (m_Phase != CLOSE)
            {
                if (CheckOrder(iOrder))
                {
                    return m_Orders.Insert(iOrder, MatchingMapping[m_Phase]);
                }
            }
            return false;
        }

        template <typename TOrder>
        template <typename TOrderReplace>
        bool OrderBook<TOrder>::Modify(TOrderReplace & iOrderReplace)
        {
            if (m_Phase != CLOSE)
            {
                if (CheckOrder(iOrderReplace))
                {
                    return m_Orders.Modify(iOrderReplace, MatchingMapping[m_Phase]);
                }
            }
            return false;
        }

        template <typename TOrder>
        bool OrderBook<TOrder>::Delete(UInt32 iOrderID, UInt32 iClientID, OrderWay iWay)
        {
            if (m_Phase != CLOSE)
            {
                return m_Orders.Delete(iOrderID, iClientID, iWay);
            }
            return false;
        }

        template<typename TOrder>
        std::ostream& operator<< (std::ostream& oss, const OrderBook<TOrder> & iOrders)
        {
            oss << "TradingPhase[" << TradingPhaseToString(iOrders.m_Phase) << "] ; ";
            oss << "SecurityName[" << iOrders.m_SecurityName << "]" << std::endl;

            oss << iOrders.m_Orders;
            return oss;
        }
    }
}