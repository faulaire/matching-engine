/*
* Copyright (C) 2014, Fabien Aulaire
* All rights reserved.
*/

#pragma once

#include <MariaDB_Connector.h>
#include <CSingleton.h>
#include <Logger.h>
#include <Types.h>

#include <boost/lexical_cast.hpp>

#include <unordered_map>
#include <string>

namespace exchange
{

    namespace common
    {

        class ConfigurationMgr : public CSingleton<ConfigurationMgr>
        {
            friend class CSingleton<ConfigurationMgr>;

            public:

                typedef std::string                             KeyType;

                typedef std::unordered_map<KeyType, KeyType>    KeyMap;
                typedef KeyMap::const_iterator                  KeyConstIterator;
                typedef KeyMap::value_type                      KeyValueType;

            public:

                bool Init(common::DataBaseConnector & iConnector);
                inline void Reset();

            public:

                inline bool IsConfigured() const { return m_IsConfigured;  }

                template <class T>
                bool GetField(const KeyType & iSection, const KeyType & iKey, T & oValue);

            private:

                KeyMap                      m_KeyMap;
                bool                        m_IsConfigured;

            private:

                ConfigurationMgr() :m_IsConfigured(false)
                {}

                ConfigurationMgr(const ConfigurationMgr &);
                ConfigurationMgr & operator=(const ConfigurationMgr &);
        };

        template <class T>
        struct GetFieldHelper
        {
            typedef ConfigurationMgr::KeyType KeyType;
            typedef ConfigurationMgr::KeyMap  KeyMap;

            bool GetFieldAsString(const KeyType & iSection, const KeyType & iKey, const KeyMap & iMap, std::string & oValue)
            {
                KeyType sKey = iSection + "." + iKey;

                auto It = iMap.find(sKey);

                if (It != iMap.end())
                {
                    oValue = It->second;
                    return true;
                }
                else
                {
                    EXERR("GetFieldHelper : Required entry " << iSection << "." << iKey <<
                        " doesn't exist");
                    return false;
                }

            }

            bool operator()(const KeyType & iSection, const KeyType & iKey, const KeyMap & iMap, T & oValue)
            {
                oValue = T();

                std::string sValue;

                if (GetFieldAsString(iSection, iKey, iMap, sValue))
                {
                    try
                    {
                        oValue = boost::lexical_cast<T>(sValue);
                        return true;
                    }
                    catch (boost::bad_lexical_cast & e)
                    {
                        EXERR("GetFieldHelper : Required conversion for entry " << iSection << "." << iKey <<
                            " is not available");
                        return false;
                    }
                }
                else
                {
                    return false;
                }
            }
        };

        template <class T>
        bool ConfigurationMgr::GetField(const KeyType & iSection, const KeyType & iKey, T & ovalue)
        {
            GetFieldHelper<T> aHelper;
            return aHelper(iSection, iKey, m_KeyMap, ovalue);
        }

        inline void ConfigurationMgr::Reset()
        {
            m_IsConfigured = false;
            m_KeyMap.clear();
        }
    }
}
