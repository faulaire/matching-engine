/*
* Copyright (C) 2014, Fabien Aulaire
* All rights reserved.
*/

#pragma once


#include <LoggerHolder.h>
#include <LoggerConsole.h>
#include <LoggerFile.h>

namespace exch_logger = exchange::common::logger;

typedef boost::fusion::vector<exch_logger::LoggerConsole, exch_logger::LoggerFile> ExchangeLoggers;
typedef exch_logger::LoggerHolder<ExchangeLoggers>                                 LoggerHolder;


#define EXINFO(MSG)  do { LoggerHolder::GetInstance() << exch_logger::header_info  << MSG << exch_logger::eos; } while (0)
#define EXWARN(MSG)  do { LoggerHolder::GetInstance() << exch_logger::header_warn  << MSG << exch_logger::eos; } while (0)
#define EXERR(MSG)   do { LoggerHolder::GetInstance() << exch_logger::header_err   << MSG << exch_logger::eos; } while (0)
#define EXPANIC(MSG) do { LoggerHolder::GetInstance() << exch_logger::header_panic << MSG << exch_logger::eos; } while (0)

#define EXLOG(Category, Verbosity, MSG)                                                    \
    do { if ( LoggerHolder::GetInstance().IsReporting( Category, Verbosity ) )                  \
    {                                                                                      \
        LoggerHolder::GetInstance() << exch_logger::header_info << MSG << exch_logger::eos;\
    } } while (0)

