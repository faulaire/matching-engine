/*
* Copyright (C) 2014, Fabien Aulaire
* All rights reserved.
*/

#pragma once

#include <logger/Logger.h>

#include <Engine_Order.h>
#include <Engine_DealHandler.h>
#include <Engine_OrderContainer.h>

namespace exchange
{
    namespace engine
    {

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

        template<typename TOrder>
        class OrderBook;

        template<typename TOrder> 
        std::ostream& operator<< (std::ostream& o, const OrderBook<TOrder> & x);


        template <typename TOrder>
        class OrderBook : public DealHandler<OrderBook<TOrder> >
        {
            private:

                /**/
                friend std::ostream& operator<< <> (std::ostream& o, const OrderBook<TOrder> & x);
                /**/
                using OrderContainerType = OrderContainer<TOrder, OrderBook>;
                using DealHandlerType = DealHandler<OrderBook<TOrder> >;

            public:

                /**/
                OrderBook(const std::string & iSecurityName, UInt32 iInstrumentID);

                /**/
                virtual ~OrderBook();

            public:

                /**/
                bool Insert(TOrder & iOrder);

                /**/
                template <typename TOrderReplace>
                bool Modify(TOrderReplace & iOrderReplace);

                /**/
                bool Delete(UInt32 iOrderID, UInt32 iClientID, OrderWay iWay);

                /**/
                void ProcessDeal(Deal * ipDeal);
                
            public:

                /*!
                Getter/Setter
                */
                inline UInt64 GetTurnover() const { return m_Turnover;             }
                inline UInt64 GetDailyVolume() const { return m_DailyVolume;       }
                inline UInt32 GetOpenPrice() const { return m_OpenPrice;           }
                inline UInt32 GetLastClosePrice() const { return m_LastClosePrice; }

                inline void SetTurnover(UInt64 iTurnOver) { m_Turnover = iTurnOver; }
                inline void SetDailyVolume(UInt64 iDailyVolume) { m_DailyVolume = iDailyVolume; }
                inline void SetOpenPrice(UInt32 iOpenPrice) { m_OpenPrice = iOpenPrice; }
                inline void SetLastClosePrice(UInt32 iLastCloseInformation) { m_LastClosePrice = iLastCloseInformation; }

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
                
                std::string         m_SecurityName;

                OrderContainerType  m_Orders;
                TradingPhase        m_Phase;
                
                UInt64              m_Turnover;
                UInt64              m_DailyVolume;
                UInt32              m_OpenPrice;
                UInt32              m_LastClosePrice;
        };

    }
}

#include <Engine_OrderBook.hxx>
