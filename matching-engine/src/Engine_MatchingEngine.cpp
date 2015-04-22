/*
* Copyright (C) 2015, Fabien Aulaire
* All rights reserved.
*/

#include <Engine_MatchingEngine.h>
#include <ConfigurationMgr.h>

namespace exchange
{
    namespace engine
    {

        MatchingEngine::MatchingEngine() :
            m_StartTime(), m_StopTime(), m_AuctionStart(),
            m_IntradayAuctionDuration(0), m_OpeningAuctionDuration(0), m_ClosingAuctionDuration(0),
            m_PriceDeviationFactor(), m_GlobalPhase(TradingPhase::CLOSE)
        {}

        MatchingEngine::~MatchingEngine()
        {}

        bool MatchingEngine::Configure(common::DataBaseConnector & iConnector)
        {
            using namespace boost::posix_time;
            using namespace boost::gregorian;

            if (iConnector.Connect())
            {
                auto && CfgMgr = common::ConfigurationMgr::GetInstance();

                if (!CfgMgr.IsConfigured())
                {
                    if (!CfgMgr.Init(iConnector))
                    {
                        return false;
                    }
                }

                common::DataBaseConnector::ResultArray Instruments;

                iConnector.Query("SELECT * from instruments", Instruments);

                using namespace exchange::common::tools;

                /* Create all instrument */
                for (auto && Instrument : Instruments)
                {
                    std::uint32_t aSecurityCode = boost::lexical_cast<std::uint32_t>(Instrument[to_underlying(InstrumentField::SECURITY_CODE)]);
                    std::string InstrumentName = Instrument[to_underlying(InstrumentField::NAME)];

                    Order::price_type ClosePrice = boost::lexical_cast<Order::price_type>(Instrument[to_underlying(InstrumentField::CLOSE_PRICE)]);

                    std::unique_ptr<OrderBookType> pBook = std::make_unique<OrderBookType>(InstrumentName, aSecurityCode, ClosePrice, *this);

                    EXINFO("MatchingEngine::Configure : Adding Instrument : " << InstrumentName);

                    auto pIterator = m_OrderBookContainer.emplace(aSecurityCode, std::move(pBook));
                    if( !pIterator.second )
                    {
                        EXERR("MatchingEngine::Configure : Corrupted database, failed to insert instrument : " << InstrumentName);
                        return false;
                    }
                }

                /*
                    Read the configuration (Start/Stop time and auction duration
                */

                std::string sTmpRes;
                bool    bRes = true;

                // Get the current localtime
                boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();
                //Get the date part out of the time
                boost::posix_time::ptime today_midnight(now.date());

                bRes &= CfgMgr.GetField("engine", "start_time", sTmpRes);
                boost::posix_time::time_duration TmpDuration = boost::posix_time::duration_from_string(sTmpRes);
                m_StartTime = today_midnight + TmpDuration;

                bRes &= CfgMgr.GetField("engine", "stop_time", sTmpRes);
                TmpDuration = boost::posix_time::duration_from_string(sTmpRes);
                m_StopTime = today_midnight + TmpDuration;

                auto Duration = 0;

                bRes &= CfgMgr.GetField("engine", "intraday_auction_duration", Duration);
                m_IntradayAuctionDuration = DurationType(Duration);

                bRes &= CfgMgr.GetField("engine", "opening_auction_duration", Duration);
                m_OpeningAuctionDuration = DurationType(Duration);;

                bRes &= CfgMgr.GetField("engine", "closing_auction_duration", Duration);
                m_ClosingAuctionDuration = DurationType(Duration);;


                std::uint32_t MaxPriceDeviation;
                bRes &= CfgMgr.GetField("engine", "max_price_deviation",MaxPriceDeviation);

                m_PriceDeviationFactor = std::make_tuple(1-(double)MaxPriceDeviation*0.01,
                                                         1+(double)MaxPriceDeviation*0.01);

                return bRes;
            }
            else
            {
                EXERR("MatchingEngine::Configure : Could not connect to DataBase");
                return false;
            }
            return true;
        }

        bool MatchingEngine::Insert(Order & iOrder, std::uint32_t iProductID)
        {
            auto OrderBookIt = m_OrderBookContainer.find(iProductID);
            if (OrderBookIt != m_OrderBookContainer.end())
            {
                return OrderBookIt->second->Insert(iOrder);
            }
            else
            {
                return false;
            }
        }

        bool MatchingEngine::Modify(OrderReplace & iOrderReplace, std::uint32_t iProductID)
        {
            auto OrderBookIt = m_OrderBookContainer.find(iProductID);
            if (OrderBookIt != m_OrderBookContainer.end())
            {
                return OrderBookIt->second->Modify(iOrderReplace);
            }
            else
            {
                return false;
            }
        }

        bool MatchingEngine::Delete(std::uint32_t iOrderID, std::uint32_t iClientID, OrderWay iWay, std::uint32_t iProductID)
        {
            auto OrderBookIt = m_OrderBookContainer.find(iProductID);
            if (OrderBookIt != m_OrderBookContainer.end())
            {
                return OrderBookIt->second->Delete(iOrderID, iClientID,iWay);
            }
            else
            {
                return false;
            }
        }

        bool MatchingEngine::SetGlobalPhase(TradingPhase iNewPhase)
        {
            /*
             * INTRADAY_AUCTION can't be set because it's managed at OrderBook level
             */
            if (iNewPhase > TradingPhase::CLOSE || iNewPhase < TradingPhase::OPENING_AUCTION)
            {
                EXERR("MatchingEngine::SetGlobalPhase [" <<  TradingPhaseToString(iNewPhase) <<
                      " is not a valid global phase");
                return false;
            }

            if (iNewPhase == TradingPhase::OPENING_AUCTION || iNewPhase == TradingPhase::CLOSING_AUCTION)
            {
                m_AuctionStart = boost::posix_time::second_clock::local_time();
            }

            UpdateInstrumentsPhase(iNewPhase);
            return true;
        }

        void MatchingEngine::UpdateInstrumentsPhase(TradingPhase iNewPhase)
        {
            if( iNewPhase != m_GlobalPhase)
            {
                EXINFO("MatchingEngine::UpdateInstrumentsPhase : Switching from phase[" << TradingPhaseToString(m_GlobalPhase)
                       << "] to phase[" << TradingPhaseToString(iNewPhase) << "]");

                m_GlobalPhase = iNewPhase;

                for (auto && OrderBook : m_OrderBookContainer)
                {
                    OrderBook.second->SetTradingPhase(iNewPhase);
                }
            }
        }

        void MatchingEngine::CheckOrderBooks(const TimeType Now)
        {
            auto iterator = m_MonitoredOrderBook.begin();
            while (iterator != m_MonitoredOrderBook.end())
            {
                auto && pBook = *iterator;
                auto AuctionEnd = pBook->GetAuctionStart() + GetIntradayAuctionDuration();
                if (Now > AuctionEnd)
                {
                    pBook->SetTradingPhase(m_GlobalPhase);
                    m_MonitoredOrderBook.erase(iterator++);
                }
                else
                {
                    ++iterator;
                }
            }
        }

        void MatchingEngine::EngineListen()
        {
            const TimeType now = boost::posix_time::second_clock::local_time();

            /* Verify if some orderbook are in intraday auction state */
            CheckOrderBooks(now);

            const bool bInOpenPeriod = (now < m_StopTime) && (now > m_StartTime);

            switch (GetGlobalPhase())
            {
                case TradingPhase::CLOSE:
                    {
                        if (bInOpenPeriod)
                        {
                            m_AuctionStart = now;
                            UpdateInstrumentsPhase(TradingPhase::OPENING_AUCTION);
                        }
                    }
                    break;
                case TradingPhase::OPENING_AUCTION:
                    {
                        auto AuctionEnd = m_AuctionStart + m_OpeningAuctionDuration;
                        if (now > AuctionEnd)
                        {
                            UpdateInstrumentsPhase(TradingPhase::CONTINUOUS_TRADING);
                        }
                    }
                    break;
                case TradingPhase::CONTINUOUS_TRADING:
                    {
                        if (!bInOpenPeriod)
                        {
                            m_AuctionStart = now;
                            UpdateInstrumentsPhase(TradingPhase::CLOSING_AUCTION);
                        }
                    }
                    break;
                case TradingPhase::CLOSING_AUCTION:
                    {
                        auto AuctionEnd = m_AuctionStart + m_ClosingAuctionDuration;
                        if (now > AuctionEnd)
                        {
                            UpdateInstrumentsPhase(TradingPhase::CLOSE);
                        }
                    }
                    break;
                default:
                    break;
            }
        }

    }
}
