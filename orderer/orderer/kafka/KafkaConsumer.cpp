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

#ifndef _MSC_VER
#include <sys/time.h>
#endif

#ifdef _MSC_VER
#include "../win32/wingetopt.h"
#include <atltime.h>
#elif _AIX
#include <unistd.h>
#else
#include <getopt.h>
#include <unistd.h>
#endif


#include <librdkafka/rdkafkacpp.h>
#include "../config/orderer_config.h"
#include "KafkaCunsumer.h"


KafkaConsumer::KafkaConsumer(kafka_config& config):_config(config)
{

}
KafkaConsumer::~KafkaConsumer()
{
	stop();

	delete _conf;
	delete _tconf;
	delete _topic_handler;
	delete _consumer;

	RdKafka::wait_destroyed(5000);
}

void KafkaConsumer::stop() {
	_consumer->stop(_topic_handler, _config._partition);
	_consumer->poll(1000);

}

void KafkaConsumer::get(int * status, int *size, void *buf)
{
	RdKafka::Message *msg = _consumer->consume(_topic_handler, _config._partition, 100);

	if (msg) {
		if (msg->err() == RdKafka::ERR__PARTITION_EOF || msg->len() == 0)
			*status = 1;
		else
			memcpy(buf, msg->payload(), msg->len());
		*size = msg->len();
		delete msg;
	}
	_consumer->poll(0);

}


int KafkaConsumer::open()
{
	std::string _errstr;
		
	void * retmsg;

	_conf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);
	_tconf = RdKafka::Conf::create(RdKafka::Conf::CONF_TOPIC);

	_conf->set("metadata.broker.list", _config._brokers, _errstr);
	_conf->set("default_topic_conf", _tconf, _errstr);

	// 이벤트로그성 작업은 테스트 용으로만. 
	//_conf->set("event_cb", &_ex_event_cb, _errstr);

	_consumer = RdKafka::Consumer::create(_conf, _errstr);
	if (!_consumer) {
		std::cerr << "Failed to create consumer: " << _errstr << std::endl;
		exit(1);
	}

	std::cout << "[orderer] Created consumer " << _consumer->name() << std::endl;

	_topic_handler = RdKafka::Topic::create(_consumer, _config._topic , _tconf, _errstr);
	if (!_topic_handler) {
		std::cerr << "Failed to create topic: " << _errstr << std::endl;
		exit(1);
	}

	RdKafka::ErrorCode resp = _consumer->start(_topic_handler, _config._partition, _config._start_offset);
	if (resp != RdKafka::ERR_NO_ERROR) {
		std::cerr << "Failed to start consumer: " <<RdKafka::err2str(resp) << std::endl;
		exit(1);
	}

	return 0;
}
