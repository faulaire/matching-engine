/*
* Copyright (C) 2014, Fabien Aulaire
* All rights reserved.
*/

#ifndef COMMON_TOOLS_HH
#define COMMON_TOOLS_HH

#include <string>
#include <Types.h>

namespace exchange
{
    namespace common
    {
        namespace tools
        {
            std::string to_hex(unsigned char s);
            std::string sha256(std::string line);

            template <typename Array>
            void to_base64(UInt64 iToEncode, Array & oEncoded, size_t FirstPos);
        }
    }
}

#include <Tools.hxx>

#endif