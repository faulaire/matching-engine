/*
* Copyright (C) 2015, Fabien Aulaire
* All rights reserved.
*/

#include <limits>

#pragma once

namespace exchange
{
    namespace engine
    {
        namespace constants
        {
            static const std::uint32_t MaxPrice = std::numeric_limits<std::uint32_t>::max();
            static const std::uint32_t MaxQty = std::numeric_limits<std::uint32_t>::max();

            static const std::uint32_t MinPrice = std::numeric_limits<std::uint32_t>::min() + 1;
            static const std::uint32_t MinQty = std::numeric_limits<std::uint32_t>::min() + 1;
        }
    }
}
