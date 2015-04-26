/*
* Copyright (C) 2015, Fabien Aulaire
* All rights reserved.
*/

#include <Engine_MatchingEngine.h>
#include <Engine_Instrument.h>

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

        bool MatchingEngine::Configure(boost::property_tree::ptree & iConfig)
        {
            if (!LoadInstruments(iConfig))
            {
                EXERR("Failed to load instruments");
                return false;
            }

            if (!LoadConfiguration(iConfig))
            {
                EXERR("Failed to load the configuration");
                return false;
            }
 
            return true;
        }

        bool MatchingEngine::LoadConfiguration(boost::property_tree::ptree & iConfig)
        {
            using namespace boost::posix_time;
            using namespace boost::gregorian;

            try
            {
                // Get the current localtime
                boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();
                //Get the date part out of the time
                boost::posix_time::ptime today_midnight(now.date());

                auto StartTime = iConfig.get<std::string>("Engine.start_time");
                auto StopTime = iConfig.get<std::string>("Engine.stop_time");

                m_StartTime = today_midnight + boost::posix_time::duration_from_string(StartTime);
                m_StopTime = today_midnight + boost::posix_time::duration_from_string(StopTime);

                m_IntradayAuctionDuration = DurationType(iConfig.get<int>("Engine.intraday_auction_duration"));

                m_OpeningAuctionDuration = DurationType(iConfig.get<int>("Engine.opening_auction_duration"));

                m_ClosingAuctionDuration = DurationType(iConfig.get<int>("Engine.closing_auction_duration"));


                auto MaxPriceDeviation = iConfig.get<std::uint32_t>("Engine.max_price_deviation");

                m_PriceDeviationFactor = std::make_tuple(
                                                            1 - (double)MaxPriceDeviation*0.01,
                                                            1 + (double)MaxPriceDeviation*0.0
                                                        );
                return true;
            }
            catch (const boost::property_tree::ptree_error & Error)
            {
                EXERR("MatchingEngine::LoadConfiguration : " << Error.what());
                return false;
            }
        }

        bool MatchingEngine::LoadInstruments(boost::property_tree::ptree & iConfig)
        {
            try
            {
                auto InstrumentHandler = [this](const Instrument<Order> & Instrument)
                {
                    // TODO : Order book should have an Instrument for argument
                    std::unique_ptr<OrderBookType> pBook = std::make_unique<OrderBookType>(
                                                                                            Instrument.GetName(),
                                                                                            Instrument.GetProductId(),
                                                                                            Instrument.GetClosePrice(),
                                                                                            *this);

                    EXINFO("MatchingEngine::LoadInstruments : Adding Instrument : " << Instrument.GetName());

                    auto pIterator = m_OrderBookContainer.emplace(Instrument.GetProductId(), std::move(pBook));
                    if (!pIterator.second)
                    {
                        EXERR("MatchingEngine::LoadInstruments : Corrupted database, failed to insert instrument : " << Instrument.GetName());
                    }
                };

                auto InstrumentDBPath = iConfig.get<std::string>("Engine.instrument_db_path");

                InstrumentManager<Order> Loader(InstrumentDBPath);

                return Loader.Load(InstrumentHandler);
            }
            catch (const boost::property_tree::ptree_error & Error)
            {
                EXERR("MatchingEngine::LoadInstruments : " << Error.what());
                return false;
            }
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
