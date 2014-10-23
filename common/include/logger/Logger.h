/*
* Copyright (C) 2014, Fabien Aulaire
* All rights reserved.
*/

#ifndef LOGGER_INTERFACE_HH
#define LOGGER_INTERFACE_HH


#include <LoggerHolder.h>
#include <LoggerConsole.h>
#include <LoggerFile.h>

namespace exch_logger = exchange::common::logger;

typedef boost::fusion::vector<exch_logger::LoggerConsole, exch_logger::LoggerFile> ExchangeLoggers;
typedef exch_logger::LoggerHolder<ExchangeLoggers>                                 LoggerHolder;


#define EXINFO(MSG)  LoggerHolder::GetInstance() << exch_logger::header_info  << MSG << exch_logger::eos;
#define EXWARN(MSG)  LoggerHolder::GetInstance() << exch_logger::header_warn  << MSG << exch_logger::eos;
#define EXERR(MSG)   LoggerHolder::GetInstance() << exch_logger::header_err   << MSG << exch_logger::eos;
#define EXPANIC(MSG) LoggerHolder::GetInstance() << exch_logger::header_panic << MSG << exch_logger::eos;

#define EXLOG(Category, Verbosity, MSG)                                                    \
    if ( LoggerHolder::GetInstance().IsReporting( Category, Verbosity ) )                  \
    {                                                                                      \
        LoggerHolder::GetInstance() << exch_logger::header_info << MSG << exch_logger::eos;\
    }

#endif