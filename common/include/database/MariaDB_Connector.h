/*
* Copyright (C) 2015, Fabien Aulaire
* All rights reserved.
*/

#pragma once

#include <string>
#include <vector>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

typedef struct st_mysql MYSQL;

namespace exchange
{
    namespace common
    {
        class DataBaseConnector
        {
        
            public:
            
                typedef std::vector< std::vector<std::string> > ResultArray;
            
            public:
            
                DataBaseConnector();
                
                ~DataBaseConnector();
                
                
                bool Configure( boost::property_tree::ptree & iConfig );
                
                bool IsConnected() const { return m_bConnected; }
                
                bool Connect();
                bool Disconnect();
                
                bool Query( const std::string & iQuery, ResultArray & oResults);
            
            public:
            
                inline const std::string & GetServer() const { return m_Server; };
                inline const std::string & GetUserName() const { return m_Username; };
                inline const std::string & GetPassword() const { return m_Password; };
                inline const std::string & GetName() const { return m_Name; };
                
                inline void SetServer(const std::string & iServer) { m_Server = iServer; };
                inline void SetUserName(const std::string & iUserName) { m_Username = iUserName; };
                inline void SetPassword(const std::string & iPassword) { m_Password = iPassword; };
                inline void SetName(const std::string & iDatabaseName) { m_Name = iDatabaseName; };
            
            protected:
            
                std::string m_Server;
                std::string m_Username;
                std::string m_Password;
                std::string m_Name;
                
                bool   m_bConnected;
                MYSQL *m_cnx;
        };
    }
}
