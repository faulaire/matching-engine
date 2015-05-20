/*
* Copyright (C) 2015, Fabien Aulaire
* All rights reserved.
*/

#pragma once

//#include <Tools.h>

#include <logger/Logger.h>

#include <Engine_Order.h>
#include <Engine_OrderBook.h>
#include <Engine_Status.h>

#include <unordered_map>
#include <unordered_set>

#include <cstdint>

namespace exchange
{
    namespace engine
    {

        template <typename Clock = boost::posix_time::second_clock>
        class MatchingEngine
        {
        public:

            using ClockType     = Clock;
            using TimeType      = boost::posix_time::ptime;
            using DurationType  = boost::posix_time::seconds;
            using OrderBookType = OrderBook<Order, MatchingEngine>;

            using OrderBookMap = std::unordered_map<std::uint32_t, std::unique_ptr<OrderBookType> >;

            using OrderBookList = std::unordered_set<OrderBookType*>;
            using PriceDevFactors = std::tuple<double, double>;

        public:

            /**/
            MatchingEngine();

            /**/
            ~MatchingEngine();

            /**/
            bool Configure(boost::property_tree::ptree & iConfig);

            /**/
            Status Insert(std::unique_ptr<Order> ipOrder, std::uint32_t iProductID);

            /**/
            Status Modify(std::unique_ptr<OrderReplace> ipOrderReplace, std::uint32_t iProductID);

            /**/
            Status Delete(std::uint32_t iOrderID, std::uint32_t iClientID, OrderWay iWay, std::uint32_t iProductID);

            /**/
            void EngineListen();

            /**/
            void CancelAllOrders();

            /**/
            inline void MonitorOrderBook(OrderBookType * pOrderBook);

            /**/
            inline void UnMonitorOrderBook(OrderBookType * pOrderBook);

            /**/
            bool SetGlobalPhase(TradingPhase iNewPhase);
            inline TradingPhase GetGlobalPhase() const { return m_GlobalPhase; }

            /**/
            const PriceDevFactors& GetPriceDevFactors() const { return m_PriceDeviationFactor; }

            /**/
            inline const OrderBookType* GetOrderBook(std::uint32_t iProductID) const;

            /**/
            inline typename OrderBookList::size_type GetMonitoredOrderBookCounter() const;

            /**/
            DurationType GetIntradayAuctionDuration() const { return m_IntradayAuctionDuration; }

            /**/
            void UpdateIntradayAuctionDuration();

            /**/
            void OnUnsolicitedCancelledOrder(const Order * order);

        protected:

            /**/
            void UpdateInstrumentsPhase(TradingPhase iNewPhase);

            /**/
            void CheckOrderBooks(const TimeType iTime);

            /**/
            bool LoadConfiguration(boost::property_tree::ptree & iConfig);

            /**/
            bool LoadAuctionConfiguration(boost::property_tree::ptree & iConfig);

            /**/
            bool LoadInstruments();

            /**/
            void SaveClosePrices();

            /**/
            int GetAuctionDurationOffset(int range);


        private:

            /* Contain all registered products */
            OrderBookMap           m_OrderBookContainer;
            /* OrderBook that must be monitored because of their state ( IntradayAuction ) */
            OrderBookList          m_MonitoredOrderBook;
            /* Time of the Close -> Opening Auction transition */
            TimeType               m_StartTime;
            /* Time of the Continuout Trading -> Closing Auction transition */
            TimeType               m_StopTime;
            /* End time of any auction phase but intraday */
            TimeType               m_AuctionEnd;
            /* Duration of the intraday auction state read from configuration file */
            DurationType           m_RawIntradayAuctionDuration;
            /* Range of available offset for auctions durations ( randomization ) */
            std::uint16_t          m_AuctionDurationOffsetRange;
            /* Duration of the intraday auction state */
            DurationType           m_IntradayAuctionDuration;
            /* Duration of the open auction state */
            DurationType           m_OpeningAuctionDuration;
            /* Duration of the close auction state */
            DurationType           m_ClosingAuctionDuration;
            /* Price deviation factors to compute minimum and maximum price */
            PriceDevFactors        m_PriceDeviationFactor;
            /* Trading phase of all products ( but Intraday Auction ) */
            TradingPhase           m_GlobalPhase;
            /* Path of the database which store instruments */
            std::string            m_InstrumentDBPath;
        };

    }

}

#include <Engine_MatchingEngine.hxx>