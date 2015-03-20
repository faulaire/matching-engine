/*
* Copyright (C) 2015, Fabien Aulaire
* All rights reserved.
*/

#pragma once

#include <string>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

namespace exchange
{

    namespace common
    {

        namespace logger
        {

            class ILogger
            {
            public:

                typedef const boost::property_tree::ptree    parser_type;
                typedef boost::optional<const parser_type&>  parser_result;

            public:

                virtual ~ILogger(void) {}
                virtual void write(const std::string&) = 0;
                virtual bool Init(const parser_type &) = 0;
            };
        }
    }
}
