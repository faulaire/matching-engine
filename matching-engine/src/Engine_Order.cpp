/*
* Copyright (C) 2014, Fabien Aulaire
* All rights reserved.
*/

#include <Engine_Order.h>

namespace exchange
{
    namespace engine
    {

        const char * OrderWayToString(OrderWay iPhase)
        {
            switch (iPhase)
            {
            case OrderWay::BUY:
                return "BUY";
                break;
            case OrderWay::SELL:
                return "SELL";
                break;
            default:
                return "UNKNOWN_WAY";
            }
        }

        std::ostream& operator<<(std::ostream& o, const Order & x)
        {
            o << "Order : Price[" << x.GetPrice() << "] ; Quantity[" << x.GetQuantity() << "] ;"
              << "Way[" << OrderWayToString(x.GetWay()) << "] ; ClientID[" << x.GetClientID() << "] ; OrderID[" << x.GetOrderID() << "]";
            return o;
        }
    }
}
