/*
* Copyright (C) 2014, Fabien Aulaire
* All rights reserved.
*/

#include <limits>
#include <Types.h>

#ifndef ENGINE_DEFINES_H
#define ENGINE_DEFINES_H

namespace exchange
{
    namespace engine
    {
        namespace constants
        {
            static const UInt32 MaxPrice = std::numeric_limits<UInt32>::max();
            static const UInt32 MaxQty = std::numeric_limits<UInt32>::max();

            static const UInt32 MinPrice = std::numeric_limits<UInt32>::min() + 1;
            static const UInt32 MinQty = std::numeric_limits<UInt32>::min() + 1;
        }
    }
};


#endif // ENGINE_DEFINES_H
