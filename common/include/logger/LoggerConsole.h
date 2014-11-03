/*
* Copyright (C) 2014, Fabien Aulaire
* All rights reserved.
*/

#pragma once

#include <string>
#include <iostream>
#include <ILogger.h>

namespace exchange
{

    namespace common
    {

        namespace logger
        {

            class LoggerConsole : public ILogger
            {
            public:

                virtual bool Init(const parser_type &)
                {
                    // Nothing to configure
                    return true;
                }

                virtual void write(const std::string& msg)
                {
                    std::cout << msg;
                }
            };

        }
    }
}
