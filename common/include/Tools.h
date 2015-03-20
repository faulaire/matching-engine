/*
* Copyright (C) 2015, Fabien Aulaire
* All rights reserved.
*/

#pragma once

#include <string>

namespace exchange
{
    namespace common
    {
        namespace tools
        {
            std::string to_hex(unsigned char s);
            std::string sha256(std::string line);

            template <typename Array>
            void to_base64(std::uint64_t iToEncode, Array & oEncoded, size_t FirstPos);

            template <typename E>
            typename std::underlying_type<E>::type to_underlying(E e)
            {
                return static_cast<typename std::underlying_type<E>::type>(e);
            }

        }
    }
}

#include <Tools.hxx>
