/*
* Copyright (C) 2014, Fabien Aulaire
* All rights reserved.
*/

#include <Engine_OrderBook.h>

namespace exchange
{
    namespace engine
    {

        const char * TradingPhaseToString(TradingPhase iPhase)
        {
            switch (iPhase)
            {
                case TradingPhase::OPENING_AUCTION:
                    return "OPENING_AUCTION";
                case TradingPhase::CONTINUOUS_TRADING:
                    return "CONTINUOUS_TRADING";
                case TradingPhase::CLOSING_AUCTION:
                    return "CLOSING_AUCTION";
                case TradingPhase::CLOSE:
                    return "CLOSE";
                case TradingPhase::INTRADAY_AUCTION:
                    return "INTRADAY_AUCTION";
                default:
                    return "UNKNOW_TRADING_PHASE";
            }
        }
    }
}
