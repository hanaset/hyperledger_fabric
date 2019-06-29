/*
Copyright Medium Corp. 2019 All Rights Reserved.


creator : HAMA
*/

#include <cstdlib>
#include <cstddef>
#include <cstdint>
#include <unistd.h>
#include "../util/StopWatch.h"
#include "../kafka/KafkaCunsumer.h"
#include "collectedTXPooll.h"
#include "transactionCollectorWorker.h"


TransactionCollectorWorker::TransactionCollectorWorker( TransactionCollector* tc)
{
    _collected_trans_queue = new  MED::ConcurrentQueue< std::shared_ptr<CollectedTranactions> >();
    _stop = false;
    _tc = tc;
}


TransactionCollectorWorker::~TransactionCollectorWorker()
{
  
    _stop = true;
    if (_thread.joinable()) _thread.join();

    delete _collected_trans_queue;
    delete _tc;

}
void TransactionCollectorWorker::start()
{
  
    _stop = false;
    _thread = std::thread(&TransactionCollectorWorker::run, this);
}

void TransactionCollectorWorker::stop()
{
    _stop = true;
    CollectedTXPool::ReleaseInstance();
}



std::shared_ptr<CollectedTranactions> TransactionCollectorWorker::getCollectedTrasnactions() {

    return  _collected_trans_queue->pop();
}

void TransactionCollectorWorker::run()
{
    while (!_stop) {
        auto queue_size = _collected_trans_queue->size();
        if (queue_size > 4) {
            sleep(0.1);
            continue;
        }

        std::shared_ptr<CollectedTranactions> ct = _tc->getOrderedTransactions();
        _collected_trans_queue->push(ct);

    }
}


