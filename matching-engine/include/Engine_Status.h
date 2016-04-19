/*
* Copyright (C) 2016, Fabien Aulaire
* All rights reserved.
*/

#pragma once

#include <iosfwd>

namespace exchange
{
    namespace engine
    {

        enum class Status : char
        {
            Ok = 0,
            PriceOutOfReservationRange,
            InstrumentNotFound,
            MarketNotOpened,
            InvalidPrice,
            InvalidQuantity,
            InvalidWay,
            OrderNotFound,
            InternalError
        };


        std::ostream& operator<<(std::ostream & oss, const Status & status);

    }
}