#pragma once

#include <thread>
class KafkaConsumer;

#include "../util/ConcurrentQueue.h"
#include "collectedTXPooll.h"

//class CollectedTranactions
//{
//public:
//    CollectedTranactions() {
//        _transaction = (uint8_t*)malloc(BLOCK_BUFFER);
//        _data_len = 0;
//        _trans_cnt = 0;
//    }
//    ~CollectedTranactions() {
//        free(_transaction);
//    }
//
//    uint8_t* _transaction;
//
//    uint32_t _data_len;
//    uint32_t _trans_cnt;
//
//    static const size_t   BLOCK_BUFFER = 300000000; // 300MB
// 
//};


class TranactionCollector
{
public:
	TranactionCollector(KafkaConsumer * consumer, MED::ConcurrentQueue<std::shared_ptr<CollectedTranactions>> * ordered_trans_queue);
	~TranactionCollector();

   
    void start();
    void stop();

private:
    std::thread _thread;
    void run();

    bool _stop;
  
    KafkaConsumer *_consumer;
    MED::ConcurrentQueue<std::shared_ptr<CollectedTranactions>> * _collected_trans_queue;

    static const uint32_t TRANACTION_BUFFER = 1000;
    static const uint32_t MAX_TRANSACTION_NUN = 200000;
};

