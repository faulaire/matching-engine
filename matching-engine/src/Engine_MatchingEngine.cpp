#include <Engine_MatchingEngine.h>
#include <ConfigurationMgr.h>

namespace exchange
{
    namespace engine
    {

        MatchingEngine::MatchingEngine() :
            m_StartTime(), m_StopTime(), m_AuctionStart(), m_AuctionDuration(), m_GlobalPhase(CLOSE)
        {}

        MatchingEngine::~MatchingEngine()
        {
            /* Delete all OrderBook */
            for (auto & OrderBook : m_OrderBookContainer)
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

                /* Create all instrument */
                for (auto & Instrument : Instruments)
                {
                    UInt32 aSecurityCode = boost::lexical_cast<UInt32>(Instrument[3]);
                    std::string InstrumentName = Instrument[1];

                    OrderBookType* pBook = new OrderBookType(InstrumentName, aSecurityCode);
                    pBook->SetLastClosePrice(boost::lexical_cast<UInt32>(Instrument[6]));

                    EXINFO("MatchingEngine::Configure : Adding Instrument : " << InstrumentName);

                    m_OrderBookContainer.insert(OrderBookValueType(aSecurityCode, pBook));
                }

                /*
                Read the configuration (Start/Stop time and auction duration
                TODO : The auction duration may depend of the auction type (closing, opening or volatile)
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

                bRes &= CfgMgr.GetField("engine", "intraday_auction_duration", m_AuctionDuration);

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
            if (iNewPhase >= INTRADAY_AUCTION || iNewPhase < OPENING_AUCTION)
            {
                return false;
            }

            if (iNewPhase == OPENING_AUCTION || iNewPhase == CLOSING_AUCTION)
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
                case CLOSE:
                    {
                        if (bInOpenPeriod)
                        {
                            m_AuctionStart = now;
                            UpdateInstrumentsPhase(OPENING_AUCTION);
                        }
                    }
                    break;
                case OPENING_AUCTION:
                    {
                        auto AuctionEnd = m_AuctionStart + boost::posix_time::seconds(m_AuctionDuration);
                        if (now > AuctionEnd)
                        {
                            UpdateInstrumentsPhase(CONTINUOUS_TRADING);
                        }
                    }
                    break;
                case CONTINUOUS_TRADING:
                    {
                        if (!bInOpenPeriod)
                        {
                            m_AuctionStart = now;
                            UpdateInstrumentsPhase(CLOSING_AUCTION);
                        }
                    }
                    break;
                case CLOSING_AUCTION:
                    {
                        auto AuctionEnd = m_AuctionStart + boost::posix_time::seconds(m_AuctionDuration);
                        if (now > AuctionEnd)
                        {
                            UpdateInstrumentsPhase(CLOSE);
                        }
                    }
                    break;
                case INTRADAY_AUCTION:
                    /* Intraday auction state is managed at OrderBook level */
                    break;
                default:
                    break;
            }
        }

    }
}
