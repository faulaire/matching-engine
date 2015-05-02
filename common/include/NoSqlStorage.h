#pragma once

#include <leveldb/db.h>

#include <Logger.h>

#include <string>

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

namespace exchange
{
    namespace common
    {

        template <typename ObjectType, typename UnderlyingStorage>
        class NoSqlStorage
        {
        public:

            using key_type = typename UnderlyingStorage::key_type;

            using key_extractor_type = std::function < const std::string &(const ObjectType &) >;

        public:

            NoSqlStorage(const std::string & DBFilePath, const key_extractor_type & key_extractor)
                :m_KeyExtractor(key_extractor), m_UdrStorage(DBFilePath), m_DBFilePath(DBFilePath)
            {}

            ~NoSqlStorage()
            {
                m_UdrStorage.Close();
            }

            template <typename TCallback>
            bool Load(TCallback & callback);

            bool Write(const ObjectType & object, bool bSync = true);

        protected:

            bool InitializeDB();

        private:
            key_extractor_type m_KeyExtractor;
            UnderlyingStorage  m_UdrStorage;
            std::string        m_DBFilePath;
        };

        template <typename ObjectType, typename UnderlyingStorage>
        bool NoSqlStorage<ObjectType, UnderlyingStorage>::InitializeDB()
        {
            return m_UdrStorage.InitializeDB();
        }

        template <typename ObjectType, typename UnderlyingStorage>
        template <typename TCallback>
        bool NoSqlStorage<ObjectType, UnderlyingStorage>::Load(TCallback & callback)
        {
            if (InitializeDB())
            {
                try
                {
                    auto object_handler = [&callback](const auto & value)
                    {
                        std::stringstream ss(value.ToString());
                        boost::archive::text_iarchive ia(ss);

                        ObjectType object;
                        ia >> object;

                        callback(object);
                    };

                    m_UdrStorage.Iterate(object_handler);
                    return true;
                }
                catch (const boost::archive::archive_exception & e)
                {
                    EXERR("NoSqlStorage::Load boost archive_exception : " << e.what());
                }
                catch (...)
                {
                    EXERR("NoSqlStorage::Load Unknown exception raised");
                }
            }
            return false;
        }

        template <typename ObjectType, typename UnderlyingStorage>
        bool NoSqlStorage<ObjectType, UnderlyingStorage>::Write(const ObjectType & object, bool bSync)
        {
            if (InitializeDB())
            {
                std::stringstream stringstream;
                boost::archive::text_oarchive oa(stringstream);

                oa << object;

                // TODO : Add a stuff to know how to get the key from the object
                const key_type  key     = object.GetName();
                const auto      svalue  = std::move(stringstream.str());

                const key_type  value   = svalue;

                if( m_UdrStorage.IsExistingKey(key) )
                {
                    EXERR("NoSqlStorage::Write : Already Existing Key");
                    return false;
                }
 
                std::string error_msg;
                if (!m_UdrStorage.DoWrite(bSync, key, value, error_msg))
                {
                    EXERR("NoSqlStorage::Write : Failed to write to DB. Reason[" << error_msg << "]");
                    return false;
                }
                else
                {
                    EXINFO("NoSqlStorage::Write : Product[" << object.GetName() << "] written in database");
                    return true;
                }
            }
            return false;
        }

        class LevelDBStorage
        {
            public:
                
                using key_type = leveldb::Slice;

            public:

                LevelDBStorage(const std::string & DBPath)
                    :m_DBFilePath(DBPath), m_db(nullptr)
                {}

                bool InitializeDB();

                template <typename TCallBack>
                void Iterate(const TCallBack & callback);

                bool IsExistingKey(const key_type & key);
                bool DoWrite(bool bSync, const key_type & key, const key_type & value, std::string & status_msg);

                void Close();

            private:

                const std::string & m_DBFilePath;
                leveldb::DB*        m_db;
        };

        template <typename TCallBack>
        void LevelDBStorage::Iterate(const TCallBack & callback)
        {
            leveldb::ReadOptions options;
            options.snapshot = m_db->GetSnapshot();

            auto release_at_exit = common::make_scope_exit([this, &options]() { m_db->ReleaseSnapshot(options.snapshot); });

            leveldb::Iterator* it = m_db->NewIterator(leveldb::ReadOptions());

            for (it->SeekToFirst(); it->Valid(); it->Next())
            {
                callback(it->value());
            }

            delete it;
        }

    }
}
