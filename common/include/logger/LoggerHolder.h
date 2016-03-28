/*
* Copyright (C) 2015, Fabien Aulaire
* All rights reserved.
*/

#pragma once

#include <vector>
#include <string>
#include <sstream>
#include <queue>
#include <thread>
#include <atomic>
#include <condition_variable>

#include <ILogger.h>

#include <CSingleton.h>

#include <boost/fusion/algorithm/iteration/for_each.hpp>
#include <boost/fusion/include/for_each.hpp>
#include <boost/fusion/container/vector.hpp>

#include <boost/filesystem.hpp>
#include <boost/format.hpp>


#include <iostream>

namespace exchange
{

    namespace common
    {

        namespace logger
        {

            struct log_type
            {
                log_type(const std::string & iDesc) :m_desc(iDesc){}
                const std::string & GetDescription() const { return m_desc; }
                std::string m_desc;
            };

            struct header_info_t : log_type { header_info_t() : log_type("I"){}; };

            struct header_warn_t : log_type { header_warn_t() : log_type("W"){}; };

            struct header_err_t : log_type { header_err_t() : log_type("E"){}; };

            struct header_panic_t : log_type { header_panic_t() : log_type("P"){}; };

            struct eos_t
            {
                explicit eos_t(){}
            };

            static const header_info_t  header_info;
            static const header_warn_t  header_warn;
            static const header_err_t   header_err;
            static const header_panic_t header_panic;

            static const eos_t eos;

            template <class THolder, class TMsg>
            struct LoggerHelper
            {
                void operator()(THolder & iHolder, const TMsg & iMsg) const
                {
                    iHolder.Push(iMsg);
                }
            };

            template <class THolder>
            struct LoggerHelper<THolder, eos_t>
            {
                void operator()(THolder & iHolder, const eos_t &) const
                {
                    iHolder.Flush();
                }
            };

            struct Initializer
            {
                const boost::property_tree::ptree & m_Config;

                Initializer(const boost::property_tree::ptree & Config)
                    :m_Config(Config)
                {}

                template<typename TLogger>
                void operator()(TLogger & iLogger) const
                {
                    iLogger.Init(m_Config);
                }

            };

            struct LoggerWriter
            {
                const std::string & m_Msg;

                LoggerWriter(const std::string & iMsg) :m_Msg(iMsg)
                {}

                template<typename TLogger>
                void operator()(TLogger & iLogger) const
                {
                    iLogger.write(m_Msg);
                }
            };

            typedef enum
            {
                LOW = 0,
                MEDIUM,
                HIGH,
                INSANE
            } Verbosity;

            class LogCategory
            {
            public:

                LogCategory(){}

                LogCategory(const std::string iName, Verbosity DefaultVerbosity)
                    :m_Name(iName), m_Verbosity(DefaultVerbosity)
                {}

                inline Verbosity GetVerbosity() const { return m_Verbosity; }

                void SetVerbosity(std::uint8_t iVerbosity)
                {
                    if (iVerbosity <= INSANE)
                    {
                        m_Verbosity = static_cast<Verbosity>(iVerbosity);
                    }
                    else
                    {
                        std::cerr << "SetVerbosity : Invalid verbosity for " << m_Name << std::endl;
                        std::cerr << "Setting verbosity to " << m_Verbosity << std::endl;
                    }
                }

                const std::string & GetName() const { return m_Name; }

            private:
                std::string m_Name;
                Verbosity   m_Verbosity;
            };

            template <class Types>
            class LoggerHolder : public common::CSingleton<LoggerHolder<Types> >
            {

                friend class common::CSingleton<LoggerHolder<Types> >;

            private:

                /* Temporary Buffer */
                std::ostringstream m_oss;

                /* Boost.Fusion vector of logger */
                Types m_Loggers;

                /* Categories */
                std::vector<LogCategory>    m_Categories;

                /* Thread related members */
                std::queue<std::string>     m_WriteQueue;
                std::queue<std::string>     m_ReadQueue;
                std::thread                 m_WriterThread;
                std::mutex                  m_mutex;
                std::condition_variable     m_cond;
                std::atomic<bool>           m_Stopped;

            private:

                LoggerHolder() :
                    m_Stopped(false)
                {
                    m_WriterThread = std::thread(boost::bind(&LoggerHolder<Types>::Write, this));
                }

            private:

                void Write()
                {
                    while (!m_Stopped)
                    {
                        {
                            std::unique_lock<std::mutex> lock(m_mutex);
                            if (m_WriteQueue.empty())
                            {
                                m_cond.wait(lock);
                            }
                            std::swap(m_WriteQueue, m_ReadQueue);
                        }

                        while (!m_ReadQueue.empty())
                        {
                            LoggerWriter aWriter(m_ReadQueue.front());
                            boost::fusion::for_each(m_Loggers, aWriter);
                            m_ReadQueue.pop();
                        }
                    }
                }

            public:

                ~LoggerHolder(void)
                {
                    {
                        std::unique_lock<std::mutex> lock(m_mutex);
                        m_Stopped = true;
                        m_cond.notify_one();
                    }
                    m_WriterThread.join();
                }

                void Init(const ILogger::parser_type & iConfig)
                {
                    Initializer aInitializer(iConfig);
                    boost::fusion::for_each(m_Loggers, aInitializer);

                    ILogger::parser_result Node = iConfig.get_child_optional("Log.Configuration");

                    std::string LogConfigurationFilePath = "LogConfiguration.ini";

                    if (Node)
                    {
                        LogConfigurationFilePath = Node->get_value<std::string>();
                    }
                    UpdateVerbosity(LogConfigurationFilePath);
                }

                void UpdateVerbosity(const std::string & FilePath)
                {
                    boost::property_tree::ptree aConfig;
                    if (!boost::filesystem::exists(FilePath))
                    {
                        std::ofstream outfile(FilePath.c_str());
                        if (outfile.good())
                        {
                            outfile.close();
                        }
                        else
                        {
                            std::cerr << "UpdateVerbosity : Could not open " << FilePath << " for writting "
                                << "all verbosity will be set to default" << std::endl;
                            return;
                        }
                    }

                    boost::property_tree::ini_parser::read_ini(FilePath, aConfig);

                    bool bChange = false;
                    auto It = m_Categories.begin();
                    for (; It != m_Categories.end(); ++It)
                    {
                        if (!(*It).GetName().empty())
                        {
                            std::string Path = "Verbosity." + (*It).GetName();
                            auto Node = aConfig.get_child_optional(Path);
                            if (Node)
                            {
                                It->SetVerbosity(Node->get_value<std::uint8_t>());
                            }
                            else
                            {
                                bChange = true;
                                aConfig.put(Path, It->GetVerbosity());
                            }
                        }
                    }

                    if (bChange)
                    {
                        boost::property_tree::ini_parser::write_ini(FilePath, aConfig);
                    }
                }

                void AddCategory(std::uint16_t Id, const std::string & iName, Verbosity DefaultVerbosity = INSANE)
                {
                    if (Id >= m_Categories.size())
                    {
                        m_Categories.resize(Id + 1);
                    }
                    m_Categories[Id] = LogCategory(iName, DefaultVerbosity);
                }

                bool IsReporting(std::uint16_t Id, Verbosity iVerb)
                {
                    if (Id < m_Categories.size())
                    {
                        return iVerb >= m_Categories[Id].GetVerbosity();
                    }
                    else
                    {
                        return false;
                    }
                }

                void Flush()
                {
                    m_oss << std::endl;
                    std::unique_lock<std::mutex> lock(m_mutex);
                    m_WriteQueue.push(m_oss.str());
                    m_oss.str("");
                    m_cond.notify_one();
                }

                template <class U>
                void Push(const U & message)
                {
                    m_oss << message;
                }

                template <class U>
                LoggerHolder<Types> & operator<<(const U& message)
                {
                    LoggerHelper<LoggerHolder<Types>, U>()(*this, message);
                    return *this;
                }
            };

            inline std::ostream& operator<< (std::ostream& stream, const log_type & log_type)
            {
                stream << "[" << log_type.GetDescription() << "] : ";
                return stream;
            }
        }
    }

}
