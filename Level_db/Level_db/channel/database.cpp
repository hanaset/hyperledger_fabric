#include "database.h"
#include <iostream>
#include "leveldb/cache.h"
#include "leveldb/filter_policy.h"

#ifndef FALLTHROUGH_INTENDED
#define FALLTHROUGH_INTENDED do { } while (0)
#endif
uint32_t DecodeFixed32(const char* ptr) {
	return ((static_cast<uint32_t>(static_cast<unsigned char>(ptr[0])))
		| (static_cast<uint32_t>(static_cast<unsigned char>(ptr[1])) << 8)
		| (static_cast<uint32_t>(static_cast<unsigned char>(ptr[2])) << 16)
		| (static_cast<uint32_t>(static_cast<unsigned char>(ptr[3])) << 24));
}

uint32_t Hash(const char* data, size_t n, uint32_t seed) {
	// Similar to murmur hash
	const uint32_t m = 0xc6a4a793;
	const uint32_t r = 24;
	const char* limit = data + n;
	uint32_t h = seed ^ (n * m);

	// Pick up four bytes at a time
	while (data + 4 <= limit) {
		uint32_t w = DecodeFixed32(data);
		data += 4;
		h += w;
		h *= m;
		h ^= (h >> 16);
	}

	// Pick up remaining bytes
	switch (limit - data) {
	case 3:
		h += static_cast<unsigned char>(data[2]) << 16;
		FALLTHROUGH_INTENDED;
	case 2:
		h += static_cast<unsigned char>(data[1]) << 8;
		FALLTHROUGH_INTENDED;
	case 1:
		h += static_cast<unsigned char>(data[0]);
		h *= m;
		h ^= (h >> r);
		break;
	}
	return h;
}

static inline uint32_t HashSlice(leveldb::Slice s) {
	return Hash(s.data(), s.size(), 0);
}

static uint32_t Shard(uint32_t hash, int ShardBits) {
	return hash % (1 << ShardBits);
}

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
	disconnect();
}

bool LevelDB::connect(std::string conn, std::string dbname)
{
	for (int idx = 0; idx < (1 << kNumShardBits); idx++)
	{
		db_info temp;
		leveldb::Status status;
		leveldb::DB* _db_;
		leveldb::Options options;
		options.create_if_missing = true;
		options.block_cache = leveldb::NewLRUCache((1 + 9 + 8 + 1 + 4) * 800 * 10000 / (1 << kNumShardBits) * 2);  // (1+9+8+1+4) * 800 * 10000 / 32 bytes * 2 = 36.8mb * 2
		//options.block_cache = leveldb::NewLRUCache(1024 * 1024 * 1024);
		//options.filter_policy = leveldb::NewBloomFilterPolicy(10);
		temp.db = _db_;
		temp.batch = leveldb::WriteBatch();
		temp.status = leveldb::Status();
		temp.opts = options;


		std::string path = dbname + "/" + std::to_string(idx);
		temp.status = leveldb::DB::Open(temp.opts, path, &temp.db);
		if (!temp.status.ok())
		{
			std::cerr << status.ToString() << std::endl;
			return false;
		}
		_ShardDB.push_back(temp);
	}
	return true;
}

void LevelDB::disconnect()
{
	for (auto it = _ShardDB.begin(); it != _ShardDB.end(); it++)
	{
		if (it->db != nullptr)
			delete it->db;
		if (it->opts.block_cache != nullptr)
			delete it->opts.block_cache;
	}
}

bool LevelDB::is_connected()
{
	for (auto it = _ShardDB.begin(); it != _ShardDB.end(); it++)
	{
		if (!it->status.ok())
			return false;
	}
	return true;
}

void LevelDB::putState_Batch(std::string name_space, std::string key, const std::string value)
{
	if (!key.empty())
		return;

	char internal_key[name_space.size() + 1 + key.size()];
	constructCompositeKey(internal_key, name_space, key);
	leveldb::Slice i_key = leveldb::Slice(internal_key, sizeof(internal_key));
	int idx = Shard(HashSlice(i_key), kNumShardBits);
	_ShardDB[idx].batch.Put(i_key, value);

}

bool LevelDB::putState(std::string name_space, std::string key, const std::string value)
{
	if (key.empty())
		return false;

	char internal_key[name_space.size() + 1 + key.size()];
	constructCompositeKey(internal_key, name_space, key);
	leveldb::Slice i_key = leveldb::Slice(internal_key, sizeof(internal_key));
	int idx = Shard(HashSlice(i_key), kNumShardBits);
	_ShardDB[idx].status = _ShardDB[idx].db->Put(leveldb::WriteOptions(), i_key, value);

	if (!_ShardDB[idx].status.ok()) {
		return false;
	}
	return true;
}

bool LevelDB::applyBatch()
{
	for (auto it = _ShardDB.begin(); it != _ShardDB.end(); it++)
	{
		if (!(it->db->Write(leveldb::WriteOptions(), &it->batch)).ok())
			return false;
	}

	for (auto it = _ShardDB.begin(); it != _ShardDB.end(); it++)
		it->batch.Clear();

	return true;
}

bool LevelDB::deleteState(const::std::string name_space, std::string key)
{
	if (key.empty())
		return false;
	char internal_key[name_space.size() + 1 + key.size()];
	constructCompositeKey(internal_key, name_space, key);
	leveldb::Slice i_key = leveldb::Slice(internal_key, sizeof(internal_key));
	int idx = Shard(HashSlice(i_key), kNumShardBits);
	_ShardDB[idx].status = _ShardDB[idx].db->Delete(leveldb::WriteOptions(), i_key);

	if (!_ShardDB[idx].status.ok()) {
		return false;
	}
	return true;
}

void LevelDB::deleteState_Batch(const::std::string name_space, std::string key)
{
	if (key.empty())
		return;
	char internal_key[name_space.size() + 1 + key.size()];
	constructCompositeKey(internal_key, name_space, key);
	leveldb::Slice i_key = leveldb::Slice(internal_key, sizeof(internal_key));
	int idx = Shard(HashSlice(i_key), kNumShardBits);
	_ShardDB[idx].batch.Delete(i_key);
	return;
}



bool LevelDB::getState(std::string name_space, const std::string key, std::string & value)
{

	char internal_key[name_space.size() + 1 + key.size()];
	constructCompositeKey(internal_key, name_space, key);
	leveldb::Slice i_key = leveldb::Slice(internal_key, sizeof(internal_key));
	int idx = Shard(HashSlice(i_key), kNumShardBits);

	_ShardDB[idx].status = _ShardDB[idx].db->Get(leveldb::ReadOptions(), i_key, &value);

	if (!_ShardDB[idx].status.ok()) {
		return false;
	}
	return true;
}


