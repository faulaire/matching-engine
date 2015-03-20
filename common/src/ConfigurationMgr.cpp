/*
* Copyright (C) 2015, Fabien Aulaire
* All rights reserved.
*/

#include <ConfigurationMgr.h>

namespace exchange
{

    namespace common
    {

        bool ConfigurationMgr::Init(common::DataBaseConnector & iConnector)
        {
            if (iConnector.Connect())
            {
                common::DataBaseConnector::ResultArray Entries;
                iConnector.Query("SELECT * from configuration", Entries);

                for (auto & Entry : Entries)
                {
                    EXINFO("Configuration::Init : Inserting entry : " << Entry[1] << "<=>" << Entry[2] <<
                        " into ConfigurationMgr");
                    m_KeyMap.emplace(Entry[1], Entry[2]);
                }
                m_IsConfigured = true;
                return true;
            }
            else
            {
                EXERR("Configuration::Init : Could not configure DataBase connector");
                return false;
            }
        }
    }
}
