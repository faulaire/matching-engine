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

#include <boost/date_time/posix_time/posix_time.hpp>

#include <unordered_map>

namespace exchange
{
    namespace engine
    {
        class MatchingEngine
        {
            public:

                using TimeType      = boost::posix_time::ptime;
                using OrderBookType = OrderBook<Order>;

                using OrderBookMap        = std::unordered_map<UInt32, OrderBookType*>;
                using OrderBookValueType  = OrderBookMap::value_type;
                using OrderBookIterator   = OrderBookMap::iterator;

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

                inline TradingPhase GetGlobalPhase() const { return m_GlobalPhase; }
                bool SetGlobalPhase(TradingPhase iNewPhase);
            
            protected:

                /**/
                void UpdateInstrumentsPhase(TradingPhase iNewPhase);

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
                OrderBookMap m_OrderBookContainer;
                /* Time of the Close -> Opening Auction transition */
                TimeType     m_StartTime;
                /* Time of the Continuout Trading -> Closing Auction transition */
                TimeType     m_StopTime;
                /* Start time of any auction phase */
                TimeType     m_AuctionStart;
                /* Duration of the intraday auction state */
                UInt16       m_IntradayAuctionDuration;
                /* Duration of the open auction state */
                UInt16       m_OpeningAuctionDuration;
                /* Duration of the close auction state */
                UInt16       m_ClosingAuctionDuration;
                /* Trading phase of all products ( but Intraday Auction ) */
                TradingPhase m_GlobalPhase;
        };
    }
}
