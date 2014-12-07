/*
* Copyright (C) 2014, Fabien Aulaire
* All rights reserved.
*/

#pragma once

#include <Types.h>
#include <Tools.h>

#include <database/MariaDB_Connector.h>
#include <logger/Logger.h>

#include <Engine_Order.h>
#include <Engine_OrderBook.h>

#include <unordered_map>
#include <memory>

namespace exchange
{
    namespace engine
    {
        class MatchingEngine
        {
            public:

                using TimeType            = boost::posix_time::ptime;
                using OrderBookType       = OrderBook<Order,MatchingEngine>;

                using OrderBookMap        = std::unordered_map<UInt32, std::unique_ptr<OrderBookType> >;
                using OrderBookValueType  = OrderBookMap::value_type;
                using OrderBookIterator   = OrderBookMap::iterator;

                using OrderBookList       = std::list<OrderBookType*>;
                using PriceDevFactors     = std::tuple<double, double>;

            public:

                /**/
                MatchingEngine();

                /**/
                ~MatchingEngine();

                /**/
                bool Configure(common::DataBaseConnector & iConnector);

                /**/
                bool Insert(Order & iOrder, UInt32 iProductID);

                /**/
                bool Modify(OrderReplace & iOrderReplace, UInt32 iProductID);

                /**/
                bool Delete(UInt32 iOrderID, UInt32 iClientID, OrderWay iWay, UInt32 iProductID);

                /**/
                void EngineListen();

                /**/
                inline void MonitorOrderBook(OrderBookType * pOrderBook);

                /**/
                bool SetGlobalPhase(TradingPhase iNewPhase);
                inline TradingPhase GetGlobalPhase() const { return m_GlobalPhase; }

                /**/
                UInt16 GetIntradayAuctionDuration() const { return m_IntradayAuctionDuration; }

                /**/
                const PriceDevFactors& GetPriceDevFactors() const { return m_PriceDeviationFactor; }
            
                /**/
                inline const OrderBookType* GetOrderBook(UInt32 iProductID) const;

            protected:

                /**/
                void UpdateInstrumentsPhase(TradingPhase iNewPhase);

                /**/
                void CheckOrderBooks(const TimeType iTime);

            private:

                enum class InstrumentField
                {
                    ID = 0,
                    NAME,
                    ISIN,
                    SECURITY_CODE,
                    TYPE,
                    CURRENCY,
                    CLOSE_PRICE
                };

            private:

                /* Contain all registered products */
                OrderBookMap    m_OrderBookContainer;
                /* OrderBook that must be monitored because of their state ( IntradayAuction ) */
                OrderBookList   m_MonitoredOrderBook;
                /* Time of the Close -> Opening Auction transition */
                TimeType        m_StartTime;
                /* Time of the Continuout Trading -> Closing Auction transition */
                TimeType        m_StopTime;
                /* Start time of any auction phase but intraday */
                TimeType        m_AuctionStart;
                /* Duration of the intraday auction state */
                UInt16          m_IntradayAuctionDuration;
                /* Duration of the open auction state */
                UInt16          m_OpeningAuctionDuration;
                /* Duration of the close auction state */
                UInt16          m_ClosingAuctionDuration;
                /* Price deviation factors to compute minimum and maximum price */
                PriceDevFactors m_PriceDeviationFactor;
                /* Trading phase of all products ( but Intraday Auction ) */
                TradingPhase    m_GlobalPhase;
        };

        inline const MatchingEngine::OrderBookType* MatchingEngine::GetOrderBook(UInt32 iProductID) const
        {
            auto It = m_OrderBookContainer.find(iProductID);
            if( It != m_OrderBookContainer.end())
            {
                return It->second.get();
            }
            else
            {
                return nullptr;
            }
        }

        inline void MatchingEngine::MonitorOrderBook(OrderBookType * pOrderBook)
        {
            m_MonitoredOrderBook.push_back(pOrderBook);
        }
    }
}
