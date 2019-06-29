#pragma once
#include <condition_variable>
#include <map>
#include <mutex>
#include <string>
#include <vector>

#include <leveldb/db.h>
#include <leveldb/slice.h>
#include <leveldb/write_batch.h>
#include <leveldb/options.h>

class idatabase
{
public:
	virtual bool connect(std::string conn, std::string dbname) = 0;
	virtual void disconnect() = 0;
	virtual bool is_connected() = 0;
	virtual void putState_Batch(std::string name_space, std::string key, const std::string value) = 0;
	virtual bool putState(std::string name_space, const std::string key, const std::string value) = 0;
	virtual bool applyBatch() = 0;
	virtual bool deleteState(const::std::string name_space, std::string key) = 0;
	virtual void deleteState_Batch(const::std::string name_space, std::string key) = 0;
	virtual bool getState(std::string name_space, const std::string key, std::string& value) = 0;

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

	virtual void putState_Batch(std::string name_space, std::string key, const std::string value);

	virtual bool putState(const::std::string name_space, std::string key, const std::string value);

	virtual bool applyBatch();

	virtual bool deleteState(const::std::string name_space, std::string key);

	virtual void deleteState_Batch(const::std::string name_space, std::string key);

	virtual bool getState(std::string name_space, const std::string key, std::string& value);





private:
	struct db_info {
		leveldb::DB * db;
		leveldb::WriteBatch batch;
		leveldb::Status status;
		leveldb::Options opts;
		bool is_cwaiting = false;
	};
	std::vector<db_info> _ShardDB;
	int kNumShardBits = 4;
};

