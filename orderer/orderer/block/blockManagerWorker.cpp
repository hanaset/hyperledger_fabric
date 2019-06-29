/*
Copyright Medium Corp. 2019 All Rights Reserved.


creator : HAMA
*/
#include "blockManagerWorker.h"

#include <utility>
#include <cstring>
#include <iostream>
#include <algorithm>
#include <vector>
#include <openssl/sha.h>
#include <unistd.h>
#include "../util/StopWatch.h"
#include "../util/ConcurrentQueue.h"
#include "../../../common/utils.h"
#include "blockManager.h"
#include "../transaction/transactionCollectorWorker.h"


BlockManagerWorker::BlockManagerWorker( BlockManager* blockManager, TransactionCollectorWorker* tc )
{
    _block_queue = new MED::ConcurrentQueue<Block*>();
  
    _stop = false;
    _blockManager = blockManager;
    _tc = tc;

}


BlockManagerWorker::~BlockManagerWorker()
{
    _stop = true;
    if (_thread.joinable()) _thread.join();

    delete _blockManager;
    delete _block_queue;
}

void BlockManagerWorker::start()
{
    _stop = false;
    _thread = std::thread(&BlockManagerWorker::run, this);
}

void BlockManagerWorker::stop()
{
    _stop = true;
}

Block* BlockManagerWorker::getBlock() {
    return  _block_queue->pop();
}

void BlockManagerWorker::run()
{
    while (!_stop) {
       auto queue_size = _block_queue->size();
       if (queue_size > 4) {
            sleep(0.1);
            continue;
       }

       std::shared_ptr<CollectedTranactions>  ct = _tc->getCollectedTrasnactions();
       //std::cout << "[ORDERER]========================================//" << std::endl;
       MED::StopWatch stopWatch;

       Block * block = _blockManager->createBlock(ct);
       _block_queue->push(block);

       //std::cout << "[ORDERER] BLOCK-M : " << stopWatch.check() << std::endl;
       //std::cout << "[ORDERER]========================================//" << std::endl;

       CollectedTXPool::GetInstance()->ReleaseCollectedTx(ct);
    }
}

