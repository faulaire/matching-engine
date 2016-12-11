/*
* Copyright (C) 2016, Fabien Aulaire
* All rights reserved.
*/

#include <Engine_Defines.h>
#include <Engine_MatchingEngine.h>

namespace exchange
{
    namespace engine
    {

        template <typename TOrder, typename TMatchingEngine>
        OrderBook<TOrder, TMatchingEngine>::OrderBook(const Instrument<TOrder> & iInstrument, TMatchingEngine& rMatchingEngine)
            : EventHandlerType(iInstrument.GetProductId()), m_rMatchingEngine(rMatchingEngine), m_SecurityName(iInstrument.GetName()), m_Orders(*this),
            m_Phase(TradingPhase::CLOSE), m_AuctionEnd(), m_LastPrice(iInstrument.GetClosePrice()), m_Turnover(0), m_DailyVolume(0),
            m_OpenPrice(0), m_ClosePrice(iInstrument.GetClosePrice()), m_PostAuctionPrice(iInstrument.GetClosePrice())
        {
        }

        template <typename TOrder, typename TMatchingEngine>
        OrderBook<TOrder,TMatchingEngine>::~OrderBook()
        {}

        template <typename TOrder, typename TMatchingEngine>
        template <typename Msg>
        Status OrderBook<TOrder, TMatchingEngine>::CheckOrder(const std::unique_ptr<Msg> & ipMsg) const
        {
            if (ipMsg->GetQuantity() < constants::MinQty || ipMsg->GetQuantity() > constants::MaxQty)
            {
                return Status::InvalidQuantity;
            }

            if (ipMsg->GetPrice() < constants::MinPrice || ipMsg->GetPrice() > constants::MaxPrice)
            {
                return Status::InvalidPrice;
            }

            if (ipMsg->GetWay() != OrderWay::BUY && ipMsg->GetWay() != OrderWay::SELL)
            {
                return Status::InvalidWay;
            }

            return Status::Ok;;
        }

        template <typename TOrder, typename TMatchingEngine>
        inline bool OrderBook<TOrder,TMatchingEngine>::SetTradingPhase(TradingPhase iNewPhase)
        {
            if( iNewPhase == m_Phase )
            {
                return true;
            }

            if (IsValidPhase(iNewPhase))
            {
                EXINFO("OrderBook::SetTradingPhase " << m_SecurityName << " : switching from phase " << TradingPhaseToString(m_Phase) <<
                    " to phase " << TradingPhaseToString(iNewPhase) << "]");

                if( IsAuctionPhase(m_Phase) && !IsAuctionPhase(iNewPhase) )
                {
                    m_Orders.MatchOrders();
                    SetPostAuctionPrice(GetLastPrice());
                }

                HandleIntradayAuctionPhaseSwitching(iNewPhase);

                if (m_Phase == TradingPhase::OPENING_AUCTION && iNewPhase == TradingPhase::CONTINUOUS_TRADING)
                {
                    SetOpenPrice(GetLastPrice());
                }

                if (m_Phase == TradingPhase::CLOSING_AUCTION && iNewPhase == TradingPhase::CLOSE)
                {
                    SetClosePrice(GetLastPrice());
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

        template <typename TOrder, typename TMatchingEngine>
        Status OrderBook<TOrder, TMatchingEngine>::Insert(std::unique_ptr<TOrder> ipOrder)
        {
            assert(ipOrder != nullptr);

            if (m_Phase != TradingPhase::CLOSE)
            {
                auto status = CheckOrder(ipOrder);
                if (Status::Ok == status)
                {
                    return m_Orders.Insert(std::move(ipOrder), TradingPhase::CONTINUOUS_TRADING == m_Phase);
                }
                else
                {
                    return status;
                }
            }
            return Status::MarketNotOpened;
        }

        template <typename TOrder, typename TMatchingEngine>
        template <typename TOrderReplace>
        Status OrderBook<TOrder, TMatchingEngine>::Modify(std::unique_ptr<TOrderReplace> ipOrderReplace)
        {
            if (m_Phase != TradingPhase::CLOSE)
            {
                auto status = CheckOrder(ipOrderReplace);

                if (Status::Ok == status)
                {
                    return m_Orders.Modify(std::move(ipOrderReplace), TradingPhase::CONTINUOUS_TRADING == m_Phase);
                }
                else
                {
                    return status;
                }
            }
            return Status::MarketNotOpened;
        }

        template <typename TOrder, typename TMatchingEngine>
        Status OrderBook<TOrder, TMatchingEngine>::Delete(Order::client_orderid_type iOrderID, Order::client_id_type iClientID, OrderWay iWay)
        {
            if (m_Phase != TradingPhase::CLOSE)
            {
                return m_Orders.Delete(iOrderID, iClientID, iWay);
            }
            return Status::MarketNotOpened;
        }

        template <typename TOrder, typename TMatchingEngine>
        void OrderBook<TOrder,TMatchingEngine>::ProcessDeal(const Deal * ipDeal)
        {
            SetTurnover(GetTurnover() + ipDeal->GetQuantity()*ipDeal->GetPrice());
            SetDailyVolume( GetDailyVolume() + ipDeal->GetQuantity() );
            SetLastPrice( ipDeal->GetPrice() );

            auto && PriceDevFactors = m_rMatchingEngine.GetPriceDevFactors();

            price_type min_price = GetPostAuctionPrice() * std::get<0>(PriceDevFactors);
            price_type max_price = GetPostAuctionPrice() * std::get<1>(PriceDevFactors);

            if( ipDeal->GetPrice() > max_price || ipDeal->GetPrice() < min_price)
            {
                if( !IsAuctionPhase(GetTradingPhase()) )
                {
                    SetTradingPhase(TradingPhase::INTRADAY_AUCTION);
                    m_AuctionEnd = TMatchingEngine::ClockType::local_time() + m_rMatchingEngine.GetIntradayAuctionDuration();

                    m_rMatchingEngine.UpdateIntradayAuctionDuration();

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
        inline bool OrderBook<TOrder, TMatchingEngine>::IsValidPhase(const TradingPhase iPhase) const
        {
            return iPhase < TradingPhase::PHASES_SIZE && iPhase >= TradingPhase::OPENING_AUCTION;
        }

        template <typename TOrder, typename TMatchingEngine>
        void OrderBook<TOrder, TMatchingEngine>::CancelAllOrders()
        {
            m_Orders.CancelAllOrders();
        }

        template <typename TOrder, typename TMatchingEngine>
        void OrderBook<TOrder, TMatchingEngine>::HandleIntradayAuctionPhaseSwitching(const TradingPhase iNewPhase)
        {
            if (m_Phase == TradingPhase::INTRADAY_AUCTION)
            {
                if (iNewPhase == TradingPhase::CLOSING_AUCTION)
                {
                    m_rMatchingEngine.UnMonitorOrderBook(this);
                }
                else if ( iNewPhase != TradingPhase::CONTINUOUS_TRADING )
                {
                    EXERR("OrderBook::SetTradingPhase " << m_SecurityName << " : invalid transition from phase " << TradingPhaseToString(m_Phase) <<
                        " to phase " << TradingPhaseToString(iNewPhase) << "]");
                    assert(false);
                }
            }
        }
        
        template <typename TOrder, typename TMatchingEngine>
        void OrderBook<TOrder, TMatchingEngine>::ProcessUnsolicitedCancelledOrder(const Order * order)
        {
            m_rMatchingEngine.OnUnsolicitedCancelledOrder(order);
        }

        template <typename TOrder, typename TMatchingEngine>
        std::ostream& operator<< (std::ostream& oss, const OrderBook<TOrder,TMatchingEngine> & iOrders)
        {
            oss << "TradingPhase["   << TradingPhaseToString(iOrders.m_Phase) << "] ; "
                << "SecurityName["   << iOrders.m_SecurityName << "] ; "
                << "TurnOver"        << iOrders.m_Turnover << "] ; "
                << "DailyVolume["    << iOrders.m_DailyVolume << "] ; "
                << "OpenPrice["      << iOrders.m_OpenPrice << "] ; "
                << "LastClosePrice[" << iOrders.m_ClosePrice << "]" << std::endl;

            oss << iOrders.m_Orders;
            return oss;
        }
    }
}
