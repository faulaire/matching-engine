#include <Engine_Status.h>
#include <string>

#include <ostream>

namespace exchange
{
    namespace engine
    {
        std::ostream& operator<<(std::ostream & oss, const Status & status)
        {
            switch (status)
            {
                case Status::Ok:
                    oss << "Ok";
                    break;
                case Status::PriceOutOfReservationRange:
                    oss << "Price Out Of Reservation Range";
                    break;
                case Status::InstrumentNotFound:
                    oss << "Instrument Not Found";
                    break;
                case Status::MarketNotOpened:
                    oss << "Market Not Opened";
                    break;
                case Status::InvalidPrice:
                    oss << "Invalid Price";
                    break;
                case Status::InvalidQuantity:
                    oss << "Invalid Quantity";
                    break;
                case Status::InvalidWay:
                    oss << "Invalid Way";
                    break;
                case Status::OrderNotFound:
                    oss << "Order Not Found";
                    break;
                case Status::InternalError:
                    oss << "Internal Error";
                    break;

            };
            return oss;
        }
    }
}
