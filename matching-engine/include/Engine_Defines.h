/*
* Copyright (C) 2016, Fabien Aulaire
* All rights reserved.
*/

#pragma once

#include <Engine_Types.h>

namespace exchange
{
    namespace engine
    {
        namespace constants
        {
            static const auto  MaxPrice = Price::max();
            static const auto  MaxQty   = Quantity::max();

            static const auto  MinPrice = Price::min()    + 1_price;
            static const auto  MinQty   = Quantity::min() + 1_qty;
        }
    }
}
