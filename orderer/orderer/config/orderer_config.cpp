
/*
Copyright Medium Corp. 2019 All Rights Reserved.


creator : HAMA
*/

#include "../../../common/config-util.h"
#include "orderer_config.h"


Orderer_config::Orderer_config()
{
}


Orderer_config::~Orderer_config()
{
}

bool Orderer_config::load_config(std::string path)
{
	config::json_conf conf;
	conf.loadConfig(path);


    //orderer config
    auto orderer_item = conf["orderer_config"];
    _orderer_config._block_create = orderer_item["block_create"].asString();



    // kafka config
	auto kafka_item = conf["kafka_config"];


	_kafka_config._brokers = kafka_item["brokers"].asString();
	_kafka_config._partition = kafka_item["partition"].asInt();
	_kafka_config._topic = kafka_item["topic"].asString();
	_kafka_config._polling_time = kafka_item["polling_time"].asInt();
	_kafka_config._queue_buffer_max_messages = kafka_item["queue_buffer_max_messages"].asString();
	_kafka_config._error_processing = kafka_item["error_processing"].asString();
	std::string start_offset = kafka_item["start_offset"].asString();

	if (start_offset == "offset_beginning")
		_kafka_config._start_offset = RdKafka::Topic::OFFSET_BEGINNING;
	else if (start_offset == "offset_stored")
		_kafka_config._start_offset = RdKafka::Topic::OFFSET_STORED;
	else if (start_offset == "offset_end")
		_kafka_config._start_offset = RdKafka::Topic::OFFSET_END;
	else if (start_offset == "offset_invalid")
		_kafka_config._start_offset = RdKafka::Topic::OFFSET_INVALID;
	else
		_kafka_config._start_offset = RdKafka::Topic::OFFSET_END;

    _kafka_config._message_max_bytes = kafka_item["message_max_bytes"].asString();
    _kafka_config._acks = kafka_item["acks"].asString();
    
	return true;
}
