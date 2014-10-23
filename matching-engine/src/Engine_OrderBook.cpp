#include <Engine_OrderBook.h>

namespace exchange
{
    namespace engine
    {

        const char * TradingPhaseToString(TradingPhase iPhase)
        {
            switch (iPhase)
            {
                case OPENING_AUCTION:
                    return "OPENING_AUCTION";
                    break;
                case CONTINUOUS_TRADING:
                    return "CONTINUOUS_TRADING";
                    break;
                case CLOSING_AUCTION:
                    return "CLOSING_AUCTION";
                    break;
                case CLOSE:
                    return "CLOSE";
                    break;
                case INTRADAY_AUCTION:
                    return "INTRADAY_AUCTION";
                    break;
                default:
                    return "UNKNOW_TRADING_PHASE";
            }
        }
    }
}

