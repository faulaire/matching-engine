/*
* Copyright (C) 2014, Fabien Aulaire
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
            m_IntradayAuctionDuration(), m_OpeningAuctionDuration(), m_ClosingAuctionDuration(),
            m_GlobalPhase(TradingPhase::CLOSE)
        {}

        MatchingEngine::~MatchingEngine()
        {
            /* Delete all OrderBook */
            for (auto && OrderBook : m_OrderBookContainer)
            {
                delete OrderBook.second;
            }
            m_OrderBookContainer.clear();
        }

        bool MatchingEngine::Configure(common::DataBaseConnector & iConnector)
        {
            using namespace boost::posix_time;
            using namespace boost::gregorian;

            if (iConnector.Connect())
            {
                common::DataBaseConnector::ResultArray Instruments;

                iConnector.Query("SELECT * from instruments", Instruments);

                using namespace exchange::common::tools;

                /* Create all instrument */
                for (auto & Instrument : Instruments)
                {
                    UInt32 aSecurityCode = boost::lexical_cast<UInt32>(Instrument[to_underlying(InstrumentField::SECURITY_CODE)]);
                    std::string InstrumentName = Instrument[to_underlying(InstrumentField::NAME)];

                    OrderBookType* pBook = new OrderBookType(InstrumentName, aSecurityCode);
                    pBook->SetLastClosePrice(boost::lexical_cast<UInt32>(Instrument[to_underlying(InstrumentField::CLOSE_PRICE)]));

                    EXINFO("MatchingEngine::Configure : Adding Instrument : " << InstrumentName);

                    auto pIterator = m_OrderBookContainer.insert(OrderBookValueType(aSecurityCode, pBook));
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

                auto & CfgMgr = common::ConfigurationMgr::GetInstance();
                if (!CfgMgr.IsConfigured())
                {
                    if (!CfgMgr.Init(iConnector))
                    {
                        return false;
                    }
                }

                bRes &= CfgMgr.GetField("engine", "start_time", sTmpRes);
                boost::posix_time::time_duration TmpDuration = boost::posix_time::duration_from_string(sTmpRes);
                m_StartTime = today_midnight + TmpDuration;

                bRes &= CfgMgr.GetField("engine", "stop_time", sTmpRes);
                TmpDuration = boost::posix_time::duration_from_string(sTmpRes);
                m_StopTime = today_midnight + TmpDuration;

                bRes &= CfgMgr.GetField("engine", "intraday_auction_duration", m_IntradayAuctionDuration);
                bRes &= CfgMgr.GetField("engine", "opening_auction_duration", m_OpeningAuctionDuration);
                bRes &= CfgMgr.GetField("engine", "closing_auction_duration", m_ClosingAuctionDuration);

                return bRes;
            }
            else
            {
                EXERR("MatchingEngine::Configure  : Could not connect to DataBase");
                return false;
            }
            return true;
        }

        bool MatchingEngine::Insert(Order & iOrder, UInt32 iProductID)
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

        bool MatchingEngine::Modify(OrderReplace & iOrderReplace, UInt32 iProductID)
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

        bool MatchingEngine::Delete(UInt32 iOrderID, UInt32 iClientID, OrderWay iWay, UInt32 iProductID)
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

        void MatchingEngine::UpdateInstrumentsPhase(TradingPhase iNewPhase)
        {
            m_GlobalPhase = iNewPhase;

            for (auto & OrderBook : m_OrderBookContainer)
            {
                OrderBook.second->SetTradingPhase(iNewPhase);
            }
        }

        bool MatchingEngine::SetGlobalPhase(TradingPhase iNewPhase)
        {
            if (iNewPhase >= TradingPhase::INTRADAY_AUCTION || iNewPhase < TradingPhase::OPENING_AUCTION)
            {
                return false;
            }

            if (iNewPhase == TradingPhase::OPENING_AUCTION || iNewPhase == TradingPhase::CLOSING_AUCTION)
            {
                m_AuctionStart = boost::posix_time::second_clock::local_time();
            }

            UpdateInstrumentsPhase(iNewPhase);
            return true;
        }

        void MatchingEngine::EngineListen()
        {
            const boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();
            
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
                        auto AuctionEnd = m_AuctionStart + boost::posix_time::seconds(m_OpeningAuctionDuration);
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
                        auto AuctionEnd = m_AuctionStart + boost::posix_time::seconds(m_ClosingAuctionDuration);
                        if (now > AuctionEnd)
                        {
                            UpdateInstrumentsPhase(TradingPhase::CLOSE);
                        }
                    }
                    break;
                case TradingPhase::INTRADAY_AUCTION:
                    /* Intraday auction state is managed at OrderBook level */
                    break;
                default:
                    break;
            }
        }

    }
}
