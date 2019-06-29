#include "database.h"
#include <iostream>

void LevelDB::constructCompositeKey(char* internal_key, std::string ns, std::string key)
{
    int ns_size = ns.size();
    int key_size = key.size();

    strncpy(internal_key, ns.data(), ns_size);
    internal_key[ns_size] = ':';
    strncpy(internal_key + ns_size + 1, key.data(), key_size);
    internal_key[ns_size + 1 + key_size] = 0x00;

}

LevelDB::LevelDB()
{
}
LevelDB::~LevelDB()
{
    if (_db != nullptr)
        delete _db;
}

bool LevelDB::connect(std::string conn, std::string dbname)
{
    leveldb::Options options;
    options.create_if_missing = true;
    _status = leveldb::DB::Open(options, dbname, &_db);

    if (!_status.ok()) {
        std::cerr << _status.ToString() << std::endl;
        return false;
    }

    return true;
}

void LevelDB::disconnect()
{
    delete _db;
}

bool LevelDB::is_connected()
{
    if (_status.ok())   return true;
    return false;
}

bool LevelDB::putState(const::std::string name_space, std::string key, const std::string value, bool last)
{
    char internal_key[name_space.size() + 1 + key.size()];
    constructCompositeKey(internal_key, name_space, key);

    _batch.Put(leveldb::Slice(internal_key, sizeof(internal_key)), value);

    if (last)
    {
        _status = _db->Write(w_opts, &_batch);
        if (!_status.ok()) {
            return false;
        }
        _batch.Clear();
    }

    return true;
}

bool LevelDB::putState_LOCK(const::std::string name_space, std::string key, const std::string value, bool last)
{
    std::unique_lock<std::mutex> ul(_batchMutex, std::defer_lock);
    char internal_key[name_space.size() + 1 + key.size()];
    constructCompositeKey(internal_key, name_space, key);

    ul.lock();
    _batch.Put(leveldb::Slice(internal_key, sizeof(internal_key)), value);

    if (last)
    {
        _status = _db->Write(w_opts, &_batch);
        if (!_status.ok()) {
            ul.unlock();
            return false;
        }
        _batch.Clear();
    }
    ul.unlock();

    return true;
}

bool LevelDB::deleteState(const::std::string name_space, std::string key, bool last)
{
    char internal_key[name_space.size() + 1 + key.size()];
    constructCompositeKey(internal_key, name_space, key);

    _batch.Delete(leveldb::Slice(internal_key, sizeof(internal_key)));

    if (last)
    {
        _status = _db->Write(w_opts, &_batch);
        if (!_status.ok()) {
            return false;
        }
        _batch.Clear();
    }

    return true;
}

bool LevelDB::deleteState_LOCK(const::std::string name_space, std::string key, bool last)
{
    std::unique_lock<std::mutex> ul(_batchMutex, std::defer_lock);
    char internal_key[name_space.size() + 1 + key.size()];
    constructCompositeKey(internal_key, name_space, key);

    ul.lock();
    _batch.Delete(leveldb::Slice(internal_key, sizeof(internal_key)));

    if (last)
    {
        _status = _db->Write(w_opts, &_batch);
        if (!_status.ok()) {
            ul.unlock();
            return false;
        }
        _batch.Clear();
    }
    ul.unlock();

    return true;
}

bool LevelDB::getState(const::std::string name_space, const std::string key, std::string& value)
{
    char internal_key[name_space.size() + 1 + key.size()];
    constructCompositeKey(internal_key, name_space, key);
    _status = _db->Get(leveldb::ReadOptions(), leveldb::Slice(internal_key, sizeof(internal_key)), &value);
    if (!_status.ok()) {
        return false;
    }

    return true;
}

