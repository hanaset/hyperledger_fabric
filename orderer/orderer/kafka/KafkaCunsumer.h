#pragma once 

#include <librdkafka/rdkafkacpp.h>
#include "KafkaEventCb.h"

class kafka_config;
class KafkaConsumer
{
public:
	KafkaConsumer(kafka_config& config);
	~KafkaConsumer();

	void get(int* status, int * size, void *buf);
	int open();
	void stop();
private:
	RdKafka::Consumer *_consumer;
    kafka_config& _config;

	RdKafka::Topic *_topic_handler;

	RdKafka::Conf *_conf;
	RdKafka::Conf *_tconf;
		
	KafkaEventCb _ex_event_cb;
};
