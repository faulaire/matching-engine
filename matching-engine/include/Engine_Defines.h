/*
* Copyright (C) 2015, Fabien Aulaire
* All rights reserved.
*/

#include <limits>
#include "Engine_Types.h"

#pragma once

namespace exchange
{
    namespace engine
    {
        namespace constants
        {
            static const auto MaxPrice = std::numeric_limits<Price>::max();
            static const auto MaxQty = std::numeric_limits<Quantity>::max();

            static const auto MinPrice = std::numeric_limits<Price>::min() + 1;
            static const auto MinQty = std::numeric_limits<Quantity>::min() + 1;
        }
    }
}
