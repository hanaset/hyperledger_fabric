/*
Copyright Medium Corp. 2019 All Rights Reserved.


creator : HAMA
*/

#include <iostream>
#include <string>
#include <cstdlib>
#include <cstdio>
#include <csignal>
#include <cstring>
#include <unistd.h>

#include <librdkafka/rdkafkacpp.h>
#include "../config/orderer_config.h"
#include "KafkaEventCb.h"
#include "KafkaProducer.h"

KafkaProducer::KafkaProducer(kafka_config& config):_config(config)
{

}

KafkaProducer::~KafkaProducer()
{
	delete _conf;
	delete _tconf;
	delete _producer;

	RdKafka::wait_destroyed(5000);

}
	
void KafkaProducer::stop() {
		
}


int  KafkaProducer::send(void * msg, int msg_size, int index)
{
	std::string  errorProcessing = _config._error_processing;
	bool send = false;
	int retry = 0;

	while (!send){
		RdKafka::ErrorCode resp = _producer->produce(_config._topic, _config._partition,
			RdKafka::Producer::RK_MSG_COPY,
			/* Value */
			msg, msg_size,
			/* Key */
			NULL, 0,
			/* Timestamp (defaults to now) */
			0,
			/* Message headers, if any */
			NULL,
			/* Per-message opaque value passed to
				* delivery report */
			NULL);

		if (resp == RdKafka::ERR__QUEUE_FULL) {
			std::cerr << "[orderer] Produced failed: " << RdKafka::err2str(resp) << std::endl;

			if (errorProcessing != "retry") {
				return -1;
			}
			std::cerr << "retry: " << ++retry << std::endl;
			sleep(1);
		}
		else if (resp != RdKafka::ERR_NO_ERROR) {
			std::cerr << "[orderer] Produced failed: " << RdKafka::err2str(resp) << std::endl;
			return -1;
		}
		else {
			//std::cerr << "[MEDIUM] Produced succeed: (" << index << ")" << std::endl;
			send = true;
		}
	}

	return 1;
}


void KafkaProducer::open()
{

	std::string _errstr;


	_conf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);
	_tconf = RdKafka::Conf::create(RdKafka::Conf::CONF_TOPIC);

	_conf->set("metadata.broker.list", _config._brokers, _errstr);
	_conf->set("default_topic_conf", _tconf, _errstr);
	_conf->set("queue.buffering.max.messages", _config._queue_buffer_max_messages, _errstr);
    _conf->set("message.max.bytes", _config._message_max_bytes, _errstr);
    _conf->set("acks", _config._acks, _errstr);

	// 이벤트로그성 작업은 테스트 용으로만. 
	//_conf->set("event_cb", &_ex_event_cb, _errstr);
	//_conf->set("dr_cb", &_ex_dr_cb, _errstr);		
	//_tconf->set("partitioner_cb", &_hash_partitioner, errstr)

	_producer = RdKafka::Producer::create(_conf, _errstr);
	if (!_producer) {
		std::cerr << "Failed to create producer: " << _errstr << std::endl;
		exit(1);
	}

	_producer->poll(_config._polling_time);

	std::cout << "[orderer] Created producer " << _producer->name() << std::endl;
}
