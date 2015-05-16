/*
* Copyright (C) 2015, Fabien Aulaire
* All rights reserved.
*/

#pragma once

#include <logger/Logger.h>

#include <Engine_Order.h>
#include <Engine_EventHandler.h>
#include <Engine_OrderContainer.h>
#include <Engine_Instrument.h>
#include <Engine_Status.h>

#include <boost/date_time/posix_time/posix_time.hpp>

namespace exchange
{
    namespace engine
    {
        template <typename Clock>
        class MatchingEngine;

        /*!
        *  All market available states
        *  Based on Xetra trading phases
        */
        enum class TradingPhase
        {
            OPENING_AUCTION = 0,
            CONTINUOUS_TRADING,
            CLOSING_AUCTION,
            CLOSE,
            INTRADAY_AUCTION,
            PHASES_SIZE
        };

        const char * TradingPhaseToString(TradingPhase iPhase);

        template <typename TOrder, typename TMatchingEngine>
        class OrderBook;

        template <typename TOrder, typename TMatchingEngine>
        std::ostream& operator<< (std::ostream& o, const OrderBook<TOrder,TMatchingEngine> & x);


        template <typename TOrder, typename TMatchingEngine>
        class OrderBook : public EventHandler<OrderBook<TOrder,TMatchingEngine> >
        {
            private:

                /**/
                friend std::ostream& operator<< <> (std::ostream& o, const OrderBook<TOrder,TMatchingEngine> & x);
                /**/
                using OrderContainerType = OrderContainer<TOrder, OrderBook>;
                using EventHandlerType = EventHandler<OrderBook<TOrder,TMatchingEngine> >;

                using price_type = typename TOrder::price_type;
                using TimeType   = boost::posix_time::ptime;

            public:

                using nominal_type = typename TOrder::nominal_type;
                using volume_type = typename TOrder::volume_type;

            public:

                /**/
                OrderBook(const Instrument<TOrder> & iInstrument, TMatchingEngine&);

                /**/
                virtual ~OrderBook();

            public:

                /**/
                Status Insert(TOrder & iOrder);

                /**/
                template <typename TOrderReplace>
                Status Modify(TOrderReplace & iOrderReplace);

                /**/
                Status Delete(std::uint32_t iOrderID, std::uint32_t iClientID, OrderWay iWay);

                /**/
                void ProcessDeal(const Deal * ipDeal);

                /**/
                void ProcessUnsolicitedCancelledOrder(const Order & order);

                /**/
                TimeType GetAuctionEnd() const { return m_AuctionEnd; }

                /***/
                void CancelAllOrders();
                
            public:

                /*!
                    Getter/Setter
                */
                inline nominal_type        GetTurnover() const { return m_Turnover; }
                inline volume_type         GetDailyVolume() const { return m_DailyVolume;           }
                inline price_type          GetOpenPrice() const { return m_OpenPrice;               }
                inline price_type          GetClosePrice() const { return m_ClosePrice;             }
                inline price_type          GetLastPrice() const { return m_LastPrice;               }
                inline TradingPhase        GetTradingPhase() const { return m_Phase;                }
                inline price_type          GetPostAuctionPrice() const { return m_PostAuctionPrice; }
                inline const std::string & GetSecurityName() const { return m_SecurityName;         }

                inline void SetClosePrice(price_type iPrice) { m_ClosePrice = iPrice;                                   }
                inline void SetLastPrice(price_type iPrice) { m_LastPrice = iPrice;                                     }
                inline void SetTurnover(nominal_type iTurnOver) { m_Turnover = iTurnOver; }
                inline void SetDailyVolume(volume_type iDailyVolume) { m_DailyVolume = iDailyVolume; }
                inline void SetOpenPrice(price_type iOpenPrice) { m_OpenPrice = iOpenPrice;                             }
                inline void SetPostAuctionPrice(price_type iPostAuctionPrice) { m_PostAuctionPrice = iPostAuctionPrice; }

                /*
                */
                inline bool SetTradingPhase(TradingPhase iNewPhase);

            private:

                /* No copy constructor and assigment operator */
                OrderBook(const OrderBook & other);
                OrderBook & operator= (const OrderBook & other);

            protected:

                /**/
                template <typename Msg>
                Status CheckOrder(const Msg & iMsg) const;

                inline bool IsAuctionPhase(const TradingPhase iPhase) const;
                inline bool IsValidPhase(const TradingPhase iPhase) const;

                void HandleIntradayAuctionPhaseSwitching(const TradingPhase iNewPhase);

            private:

                TMatchingEngine&       m_rMatchingEngine;
                std::string            m_SecurityName;

                OrderContainerType     m_Orders;
                TradingPhase           m_Phase;

                TimeType               m_AuctionEnd;

                price_type             m_LastPrice;

                nominal_type           m_Turnover;
                volume_type            m_DailyVolume;
                price_type             m_OpenPrice;
                price_type             m_ClosePrice;
                price_type             m_PostAuctionPrice;
        };

    }
}

#include <Engine_OrderBook.hxx>
