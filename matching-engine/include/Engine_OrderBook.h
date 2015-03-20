/*
* Copyright (C) 2015, Fabien Aulaire
* All rights reserved.
*/

#pragma once

#include <logger/Logger.h>

#include <Engine_Order.h>
#include <Engine_DealHandler.h>
#include <Engine_OrderContainer.h>

#include <boost/date_time/posix_time/posix_time.hpp>

namespace exchange
{
    namespace engine
    {

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
        class OrderBook : public DealHandler<OrderBook<TOrder,TMatchingEngine> >
        {
            private:

                /**/
                friend std::ostream& operator<< <> (std::ostream& o, const OrderBook<TOrder,TMatchingEngine> & x);
                /**/
                using OrderContainerType = OrderContainer<TOrder, OrderBook>;
                using DealHandlerType = DealHandler<OrderBook<TOrder,TMatchingEngine> >;

                using price_type = typename TOrder::price_type;
                using TimeType   = boost::posix_time::ptime;

            public:

                /**/
                OrderBook(const std::string & iSecurityName, std::uint32_t iInstrumentID, price_type iLastClosePrice, TMatchingEngine&);

                /**/
                virtual ~OrderBook();

            public:

                /**/
                bool Insert(TOrder & iOrder);

                /**/
                template <typename TOrderReplace>
                bool Modify(TOrderReplace & iOrderReplace);

                /**/
                bool Delete(std::uint32_t iOrderID, std::uint32_t iClientID, OrderWay iWay);

                /**/
                void ProcessDeal(const Deal * ipDeal);

                /**/
                TimeType GetAuctionStart() const { return m_AuctionStart; }
                
            public:

                /*!
                    Getter/Setter
                */
                inline std::uint64_t GetTurnover() const { return m_Turnover;                 }
                inline std::uint64_t GetDailyVolume() const { return m_DailyVolume;           }
                inline price_type    GetOpenPrice() const { return m_OpenPrice;               }
                inline price_type    GetLastClosePrice() const { return m_LastClosePrice;     }
                inline price_type    GetLastPrice() const { return m_LastPrice;               }
                inline TradingPhase  GetTradingPhase() const { return m_Phase;                }

                inline void SetLastPrice(price_type iPrice) { m_LastPrice = iPrice;                    }
                inline void SetTurnover(std::uint64_t iTurnOver) { m_Turnover = iTurnOver;             }
                inline void SetDailyVolume(std::uint64_t iDailyVolume) { m_DailyVolume = iDailyVolume; }
                inline void SetOpenPrice(price_type iOpenPrice) { m_OpenPrice = iOpenPrice;            }

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
                bool CheckOrder(const Msg & iMsg) const;

                inline bool IsAuctionPhase(const TradingPhase iPhase) const;

            private:

                TMatchingEngine&       m_rMatchingEngine;
                std::string            m_SecurityName;

                OrderContainerType     m_Orders;
                TradingPhase           m_Phase;

                TimeType               m_AuctionStart;

                price_type             m_LastPrice;

                std::uint64_t          m_Turnover;
                std::uint64_t          m_DailyVolume;
                price_type             m_OpenPrice;
                price_type             m_LastClosePrice;
        };

    }
}

#include <Engine_OrderBook.hxx>
