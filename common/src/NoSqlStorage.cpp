#include <ScopedExit.h>
#include <NoSqlStorage.h>

namespace exchange
{
    namespace common
    {
        bool LevelDBStorage::InitializeDB()
        {
            if (m_db == nullptr)
            {
                leveldb::Options options;
                options.create_if_missing = true;

                leveldb::Status status = leveldb::DB::Open(options, m_DBFilePath, &m_db);

                if (!status.ok())
                {
                    EXERR("InstrumentManager::InitializeDB : Failed to initialize DB. Reason[" << status.ToString() << "]");
                    return false;
                }
            }
            return true;
        }

        bool LevelDBStorage::IsExistingKey(const key_type & key)
        {
            std::string dummy_res;
            return m_db->Get(leveldb::ReadOptions(), key, &dummy_res).ok();
        }

        bool LevelDBStorage::DoWrite(bool bSync, const key_type & key, const key_type & value, std::string & status_msg)
        {
            leveldb::WriteOptions write_options;
            write_options.sync = bSync;

            auto status = m_db->Put(write_options, key, value);
            if (!status.ok())
            {
                status_msg = status.ToString();
                return false;
            }
            return true;
        }

        void LevelDBStorage::Close()
        {
            delete m_db;
        }
    }
}