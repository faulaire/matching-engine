/*
* Copyright (C) 2014, Fabien Aulaire
* All rights reserved.
*/


#include <MariaDB_Connector.h>

#include <Logger.h>

#include <my_global.h>
#include <mysql.h>

namespace exchange 
{
    namespace common
    {
    
        /**
        *  \brief Constructor of a DataBaseConnector
        *
        */
        DataBaseConnector::DataBaseConnector():
            m_bConnected(false), m_cnx(nullptr)
        {
        }
        
        /**
        *  \brief Destructor of a DataBaseConnector
        *
        *   If a connection is opened, close it.  
        */
        DataBaseConnector::~DataBaseConnector() 
        {
            if ( IsConnected() )
            {
                mysql_close(m_cnx);
            }
        }
        
        /**
        * \brief Load server credentials from the configuration file
        *
        * \param[in] iConfig : Configuration tree
        *
        * If any parameter is missing or is ill-formated return false 
        */
        bool DataBaseConnector::Configure( boost::property_tree::ptree & iConfig )
        {
            try 
            {
                m_Username = iConfig.get<std::string>("Database.username");
                m_Password = iConfig.get<std::string>("Database.password");
                m_Server = iConfig.get<std::string>("Database.hostname");
                m_Name = iConfig.get<std::string>("Database.name");
                
                EXINFO("DataBaseConnector::Configure : Connector sucessfully configured");
                
                return true;
            }
            
            catch ( const boost::property_tree::ptree_error & Error)
            {
                EXERR("DataBaseConnector::Configure : Error while reading a node : " << Error.what() );
                return false;
            }
            catch ( const std::exception & Error)
            {
                EXERR("DataBaseConnector::Configure : Unknow error : " << Error.what() );
                return false;
            }
        }
        
        /**
        * \brief Connect to the MariaDB database
        * \return true : The connection is sucessfully opened
        * \return false : Error during the connection process
        */
        bool DataBaseConnector::Connect()
        {
            if (IsConnected())
            {
                return true;
            }
            
            m_cnx = mysql_init(NULL);
            EXINFO("MySQL client version: " << mysql_get_client_info());

            
            if (mysql_real_connect(m_cnx, m_Server.c_str(), m_Username.c_str(), m_Password.c_str(), 
                m_Name.c_str(), 0, NULL, 0) == NULL) 
            {
                EXERR( mysql_error(m_cnx) );
                mysql_close(m_cnx);
                m_bConnected = false;
                return false;
            }
            else
            {
                m_bConnected = true;
                return true;
            }
        }
        
        /**
        * \brief Disconnect from MariaDB database
        * \return true : Diconnected from database
        * \return false : No disconnection performed ( Maybe already disconnected )
        */
        bool DataBaseConnector::Disconnect()
        {
            if ( IsConnected() )
            {
                mysql_close(m_cnx);
                m_bConnected = false;
                return true;
            }
            return false;
        }
        
        /**
        *  \brief Execute a SQL query
        *
        *  Execute a sql query and store the result into a two dimensionnal array of strings
        *  \param[in] iQuery : SQL Request
        *  \param[out] oResults : Results of the request
        *
        * \return true : Request successfully executed
        * \return false: Request execution failed
        */
        bool DataBaseConnector::Query( const std::string & iQuery, ResultArray & oResults)
        {
            if (!IsConnected())
            {
                return false;
            }

            if ( mysql_query( m_cnx, iQuery.c_str() ) ) 
            {
                EXERR( mysql_error(m_cnx) );
                return false;
            }
            
            MYSQL_RES *result = mysql_store_result(m_cnx);
            
            if (result == NULL) 
            {
                EXERR( mysql_error(m_cnx) );
                return false;
            }
            
            std::uint64_t num_fields  = mysql_num_fields(result);
            std::uint64_t num_rows    = mysql_num_rows(result);
            
            oResults = ResultArray(num_rows, std::vector<std::string>(num_fields));
            
            MYSQL_ROW row;
            
            std::uint32_t RowIndex = 0;
            while ((row = mysql_fetch_row(result))) 
            { 
                for (std::uint32_t Field = 0; Field < num_fields; ++Field)
                { 
                    if ( row[Field] )
                    {
                        oResults[RowIndex][Field] = row[Field];
                    }
                } 
                ++RowIndex;
            }
            
            mysql_free_result(result);
            
            return true;
        }
    
    }
}
