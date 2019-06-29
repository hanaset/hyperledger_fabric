#pragma once

#include <thread>
#include "block.h"
#include <cstdint>
#include "../../../common/utils.h"
#include "../../../common/net_config.h"
#include "../../../common/escc.h""

#include "../util/ConcurrentQueue.h"
#include "../transaction/collectedTXPooll.h"

class CollectedTranactions;
class BlockManager;
class TransactionCollectorWorker;

class BlockManagerWorker 
{
public:
    BlockManagerWorker(BlockManager* blockManager, TransactionCollectorWorker* tc );
    ~BlockManagerWorker();

    void start();
    void stop();
    
    Block* getBlock();
private:
    std::thread _thread;
    void run();
    bool _stop;

    MED::ConcurrentQueue<std::shared_ptr<CollectedTranactions>> * _collected_trans_queue;
    MED::ConcurrentQueue<Block*> *  _block_queue;

    BlockManager* _blockManager; 
    TransactionCollectorWorker* _tc;
};
