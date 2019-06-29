#pragma once
#include <map>
#include <mutex>
#include <string>
#include <vector>

#include <leveldb/db.h>
#include <leveldb/slice.h>
#include <leveldb/write_batch.h>


class idatabase
{
public:
    virtual bool connect(std::string conn, std::string dbname) = 0;
    virtual void disconnect() = 0;
    virtual bool is_connected() = 0;

    virtual bool putState(const::std::string name_space, const std::string key, const std::string value, bool last) = 0;
    virtual bool getState(const::std::string name_space, const std::string key, std::string& value) = 0;

};

class LevelDB : public idatabase
{
private:
    void constructCompositeKey(char* buf, std::string ns, std::string key);
public:
    LevelDB();
    virtual ~LevelDB();

    virtual bool connect(std::string conn, std::string dbname);
    virtual void disconnect();
    virtual bool is_connected();

    virtual bool putState(const::std::string name_space, std::string key, const std::string value, bool last);

    virtual bool putState_LOCK(const::std::string name_space, std::string key, const std::string value, bool last);

    virtual bool getState(const::std::string name_space, const std::string key, std::string& value);

    virtual bool deleteState(const::std::string name_space, std::string key, bool last);
	
    virtual bool deleteState_LOCK(const::std::string name_space, std::string key, bool last);
private:
    leveldb::DB* _db;
    leveldb::Status _status;
    leveldb::WriteBatch _batch;
    std::mutex _batchMutex;
    leveldb::WriteOptions w_opts;
};
