cmake_minimum_required(VERSION 3.6)
project(matching_engine)

set(CMAKE_CXX_STANDARD 14)

include_directories(common/include
                    matching-engine/include
                    trading-gateway/include)

set(SOURCE_FILES
    common/include/logger/ILogger.h
    common/include/logger/Logger.h
    common/include/logger/LoggerConsole.h
    common/include/logger/LoggerFile.h
    common/include/logger/LoggerHolder.h
    common/include/CSingleton.h
    common/include/MemoryPool.h
    common/include/MemoryPool.hxx
    common/include/NoSqlStorage.h
    common/include/ScopedExit.h
    common/src/logger/LoggerFile.cpp
    common/src/NoSqlStorage.cpp
    common/tests/src/test_logger.cpp
    common/wscript
    matching-engine/config/config.ini
    matching-engine/include/Engine_Deal.h
    matching-engine/include/Engine_Defines.h
    matching-engine/include/Engine_EventHandler.h
    matching-engine/include/Engine_Instrument.h
    matching-engine/include/Engine_MatchingEngine.h
    matching-engine/include/Engine_MatchingEngine.hxx
    matching-engine/include/Engine_Order.h
    matching-engine/include/Engine_OrderBook.h
    matching-engine/include/Engine_OrderBook.hxx
    matching-engine/include/Engine_OrderContainer.h
    matching-engine/include/Engine_OrderContainer.hxx
    matching-engine/include/Engine_Status.h
    matching-engine/include/Engine_Tools.h
    matching-engine/include/Engine_Types.h
    matching-engine/src/Engine_Benchmark.cpp
    matching-engine/src/Engine_Deal.cpp
    matching-engine/src/Engine_EventHandler.cpp
    matching-engine/src/Engine_Order.cpp
    matching-engine/src/Engine_OrderBook.cpp
    matching-engine/src/Engine_Status.cpp
    matching-engine/src/Engine_Types.cpp
    matching-engine/tests/src/test_IntrumentManager.cpp
    matching-engine/tests/src/test_MatchingEngine.cpp
    matching-engine/tests/src/test_OrderBook.cpp
    matching-engine/tests/src/test_OrderContainer.cpp
    matching-engine/wscript
    trading-gateway/config/config.ini
    trading-gateway/config/FIX42.xml
    trading-gateway/include/Gateway_Application.h
    trading-gateway/src/Gateway_application.cpp
    trading-gateway/src/Gateway_main.cpp
    trading-gateway/tests/config/pom.xml
    trading-gateway/wscript
    waf_tools/__init__.py
    .travis.yml
    bootstrap.py
    LICENSE
    README.md
    waf
    wscript)

add_executable(matching_engine ${SOURCE_FILES})