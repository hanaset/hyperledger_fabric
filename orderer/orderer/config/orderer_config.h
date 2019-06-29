#pragma once
#include <librdkafka/rdkafkacpp.h>
#include <string>
#include <map>
#include <memory>


struct kafka_config
{
	// common 
	std::string _brokers;
	std::string _topic;
	int32_t _partition = 0;

	// producer
	int32_t _polling_time = 0;
	std::string _queue_buffer_max_messages = "1000000";
	std::string _error_processing = "retry";
    std::string _message_max_bytes = "1000000";
    std::string _acks = "all";

	// consumer
	int64_t _start_offset = RdKafka::Topic::OFFSET_BEGINNING;
  
};

struct orderer_config
{
    std::string _block_create = "sync";
};


class Orderer_config
{
public:
		
	bool load_config(std::string path);
	Orderer_config();
	~Orderer_config();

	kafka_config& get_kafka_config() { return _kafka_config; }
    orderer_config& get_orderer_config() { return _orderer_config; }
	
private:
	kafka_config _kafka_config;
    orderer_config _orderer_config;
};


