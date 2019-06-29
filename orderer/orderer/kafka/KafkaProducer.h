#pragma once 
#include <librdkafka/rdkafkacpp.h>
#include "KafkaEventCb.h"

class ExampleDeliveryReportCb : public RdKafka::DeliveryReportCb {
public:
	void dr_cb(RdKafka::Message &message) {
		std::string status_name;
		switch (message.status())
		{
		case RdKafka::Message::MSG_STATUS_NOT_PERSISTED:
			status_name = "NotPersisted";
			break;
		case RdKafka::Message::MSG_STATUS_POSSIBLY_PERSISTED:
			status_name = "PossiblyPersisted";
			break;
		case RdKafka::Message::MSG_STATUS_PERSISTED:
			status_name = "Persisted";
			break;
		default:
			status_name = "Unknown?";
			break;
		}
		std::cout << "Message delivery for (" << message.len() << " bytes): " <<
			status_name << ": " << message.errstr() << std::endl;
		if (message.key())
			std::cout << "Key: " << *(message.key()) << ";" << std::endl;
	}
};

/* Use of this partitioner is pretty pointless since no key is provided
	* in the produce() call. */
class MyHashPartitionerCb : public RdKafka::PartitionerCb {
public:
	int32_t partitioner_cb(const RdKafka::Topic *topic, const std::string *key,
		int32_t partition_cnt, void *msg_opaque) {
		return djb_hash(key->c_str(), key->size()) % partition_cnt;
	}
private:

	static inline unsigned int djb_hash(const char *str, size_t len) {
		unsigned int hash = 5381;
		for (size_t i = 0; i < len; i++)
			hash = ((hash << 5) + hash) + str[i];
		return hash;
	}
};

class kafka_config;
class KafkaProducer
{
public:
	KafkaProducer(kafka_config& config);
	~KafkaProducer();

	int send(void * msg, int msg_size,int index);
	void open();
	void stop();

private:
	RdKafka::Producer * _producer;
    kafka_config& _config;
	
	RdKafka::Conf *_conf;
	RdKafka::Conf *_tconf;

	ExampleDeliveryReportCb _ex_dr_cb;
	MyHashPartitionerCb _hash_partitioner;
	KafkaEventCb _ex_event_cb;
};
