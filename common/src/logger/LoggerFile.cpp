/*
* Copyright (C) 2016, Fabien Aulaire
* All rights reserved.
*/

#include <LoggerFile.h>

namespace exchange
{

    namespace common
    {
        namespace logger
        {

            LoggerFile::LoggerFile() :
                m_facet(new boost::posix_time::time_facet("%Y%m%d %H:%M:%S.%f"))
            {}

            LoggerFile::~LoggerFile()
            {
                if (!m_File.good())
                {
                    return;
                }
                m_File << "\n  ===============================================\n"
                    << "    End Output log "
                    << "\n  ===============================================\n\n";
                m_File.flush();
                m_File.close();
            }

            bool LoggerFile::Init(const parser_type & Config)
            {
                parser_result Node = Config.get_child_optional("Log.FileName");

                try
                {
                    if (Node)
                    {
                        m_FilePath = Node->get_value<std::string>();
                    }
                    else
                    {
                        std::cerr << "Could not found Log.FileName path. Aborting..." << std::endl;
                        return false;
                    }

                }
                catch (...)
                {
                    std::cerr << "Parsing error" << std::endl;
                    return false;
                }

                boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();
                std::string sSuffix = "_" + boost::posix_time::to_simple_string(now) + ".log";
                std::replace(sSuffix.begin(), sSuffix.end(), ' ', '_');
                std::replace(sSuffix.begin(), sSuffix.end(), ':', '.');

                m_FilePath += sSuffix;

                m_File.open(m_FilePath.c_str(), std::ios::out);
                m_File.seekp(std::ios::beg);

                if (!m_File.good())
                {
                    return false;
                }

                m_File.imbue(std::locale(m_File.getloc(), m_facet));

                m_File << "  ===============================================\n"
                    << "    Begin Output log  "
                    << "\n  ===============================================\n\n";
                m_File.flush();

                return true;
            }

            void LoggerFile::write(const std::string& msg)
            {
                m_File << boost::posix_time::microsec_clock::universal_time() << " " << msg;
                m_File.flush();
            }

        }
    }

}
