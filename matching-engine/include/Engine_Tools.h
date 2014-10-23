/*
* Copyright (C) 2014, Fabien Aulaire
* All rights reserved.
*/

#ifndef ENGINE_TOOLS_INCLUDE
#define ENGINE_TOOLS_INCLUDE


namespace exchange
{
    namespace engine
    {

        template<typename> struct Void { typedef void type; };

        template<typename T, typename Sfinae = void>
        struct is_replaced_order : std::false_type {};

        template<typename T>
        struct is_replaced_order<
            T
            , typename Void<
            decltype(std::declval<T&>().GetReplacedOrderID())
            >::type
        > : std::true_type{};
    }
}

#endif
