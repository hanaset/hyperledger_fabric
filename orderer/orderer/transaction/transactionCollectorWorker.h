#pragma once

#include <thread>
class KafkaConsumer;

#include "../util/ConcurrentQueue.h"
#include "collectedTXPooll.h"
#include "transactionCollector.h"

class TransactionCollectorWorker 
{
public:
    TransactionCollectorWorker( TransactionCollector* tc);
	~TransactionCollectorWorker();

    void start();
    void stop();
 
    std::shared_ptr<CollectedTranactions> getCollectedTrasnactions();

private:
    std::thread _thread;
    void run();
    bool _stop;
  
    MED::ConcurrentQueue<std::shared_ptr<CollectedTranactions>> * _collected_trans_queue;

    TransactionCollector* _tc;
};

