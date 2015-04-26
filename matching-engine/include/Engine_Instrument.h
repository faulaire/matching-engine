/*
* Copyright (C) 2015, Fabien Aulaire
* All rights reserved.
*/

#pragma once

#include "leveldb/db.h"

#include <sstream>

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

namespace exchange
{
    namespace engine
    {

        template <typename TOrder>
        class Instrument;

        template <typename TOrder>
        std::ostream& operator<< (std::ostream& o, const Instrument<TOrder> & x);


        template <typename TOrder>
        class Instrument
        {
            friend std::ostream& operator<< <> (std::ostream& o, const Instrument<TOrder> & x);
            
            using price_type = typename TOrder::price_type;

        public:

            Instrument() = default;

            Instrument(const std::string & name, const std::string & isin, const std::string & currency, int id, std::uint64_t closeprice)
                :m_name(name), m_isin(isin), m_currency(currency), m_closeprice(closeprice), m_productid(id)
            {}

            const std::string & GetName() const { return m_name; }
            const std::string & GetCurrency() const { return m_currency; }
            const std::string & GetIsin() const { return m_isin; }
            price_type          GetClosePrice() const { return m_closeprice; }
            int                 GetProductId() const { return m_productid; }

        private:

            friend class boost::serialization::access;

            template<class Archive>
            void serialize(Archive & ar, const unsigned int /* version */)
            {
                ar & m_name;
                ar & m_isin;
                ar & m_currency;
                ar & m_closeprice;
                ar & m_productid;
            }

        private:

            std::string        m_name;
            std::string        m_isin;
            std::string        m_currency;
            price_type         m_closeprice;
            std::uint32_t      m_productid;
                   
        };

        template <typename TOrder>
        std::ostream & operator<<(std::ostream &os, const Instrument<TOrder> &gp)
        {
            return os << "Name[" << gp.m_name << "] Isin[" << gp.m_isin << "] Currency[" << gp.m_currency << "] "
                << "ClosePrice[" << gp.m_closeprice << "] ProductId[" << gp.m_productid << "]";
        }
        

        template <typename TOrder>
        class InstrumentManager
        {
            private:

                using instrument_type = Instrument < TOrder > ;

            public:

                InstrumentManager(const std::string & LevelDBFilePath);
                ~InstrumentManager();

                template <typename TCallback>
                bool Load(TCallback & callback);

                bool Write(const instrument_type & Instrument, bool bSync = true);

            private:

                bool InitializeDB();

            private:
                
                std::string  m_LevelDBFilePath;
                leveldb::DB* m_db;
        };


        template <typename TOrder>
        InstrumentManager<TOrder>::InstrumentManager(const std::string & LevelDBFilePath)
            :m_LevelDBFilePath(LevelDBFilePath), m_db(nullptr)
        {}

        template <typename TOrder>
        InstrumentManager<TOrder>::~InstrumentManager()
        {
            delete m_db;
        }

        template <typename TOrder>
        bool InstrumentManager<TOrder>::InitializeDB()
        {
            if (m_db == nullptr)
            {
                leveldb::Options options;
                options.create_if_missing = true;

                leveldb::Status status = leveldb::DB::Open(options, m_LevelDBFilePath, &m_db);

                if (!status.ok())
                {
                    EXERR("InstrumentManager::InitializeDB : Failed to initialize DB. Reason[" << status.ToString() << "]");
                    return false;
                }
            }
            return true;
        }

        template <typename TOrder>
        template <typename TCallback>
        bool InstrumentManager<TOrder>::Load(TCallback & callback)
        {
            if (InitializeDB())
            {
                try
                {
                    leveldb::ReadOptions options;
                    options.snapshot = m_db->GetSnapshot();
                    
                    leveldb::Iterator* it = m_db->NewIterator(leveldb::ReadOptions());
                    for (it->SeekToFirst(); it->Valid(); it->Next())
                    {
                        leveldb::Slice value = it->value();

                        std::stringstream ss(value.ToString());
                        boost::archive::text_iarchive ia(ss);

                        instrument_type instrument;
                        ia >> instrument;

                        callback(instrument);
                    }

                    m_db->ReleaseSnapshot(options.snapshot);

                    delete it;
                    return true;
                }
                catch (...)
                {
                    EXERR("InstrumentManager::Load Unknown exception raised");
                    return false;
                }
            }
            return false;
        }

        template <typename TOrder>
        bool InstrumentManager<TOrder>::Write(const instrument_type & Instrument, bool bSync)
        {
            if (InitializeDB())
            {
                std::stringstream stringstream;
                boost::archive::text_oarchive oa(stringstream);

                oa << Instrument;

                const leveldb::Slice  key = Instrument.GetName();
                const leveldb::Slice  value = stringstream.str();

                std::string dummy_res;

                if (m_db->Get(leveldb::ReadOptions(), key, &dummy_res).ok())
                {
                    EXERR("InstrumentManager::Write : Already Existing Key");
                    return false;
                }
                
                leveldb::WriteOptions write_options;
                write_options.sync = bSync;

                auto status = m_db->Put(write_options, key, value);
                if (!status.ok())
                {
                    EXERR("InstrumentManager::Write : Failed to write to DB. Reason[" << status.ToString() << "]");
                    return false;
                }
                else
                {
                    EXINFO("InstrumentManager::Write : Product[" << Instrument.GetName() << "] with ProductID[" << Instrument.GetProductId() << "] written in database");
                    return true;
                }
            }
            return false;
        }

    }
}