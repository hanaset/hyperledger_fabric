/*
Copyright Medium Corp. 2019 All Rights Reserved.


creator : HAMA
*/

#include <openssl/sha.h>

#include "../../../common/utils.h"
#include "../../../common/net_config.h"
#include "../../../common/escc.h"
#include "../config/orderer_config.h"
#include "../kafka/KafkaCunsumer.h"
#include "../kafka/KafkaProducer.h"
#include "../util/ConcurrentQueue.h"
#include "../block/block.h"
#include "../block/blockManagerWorker.h"
#include "../block/blockManager.h"
#include "../transaction/transactionCollector.h"
#include "../transaction/transactionCollectorWorker.h"

#include "kafkaConsensus.h"


KafkaConsensus::KafkaConsensus(std::shared_ptr<Orderer_config> & config, uint32_t node_id, uint8_t* secrete) 
{
    _node_id = node_id;
    _secret = secrete;
    _config = config;

    _producer = new KafkaProducer(_config->get_kafka_config());
    _consumer = new KafkaConsumer(_config->get_kafka_config());

    _producer->open();
    _consumer->open();

    _transactionCollector = new TransactionCollector(_consumer);
    _blockManager = new BlockManager(_node_id, _secret);

    if (_config->get_orderer_config()._block_create == "async") {
     
        _stop = false;

        _transactionCollectorWorker = new TransactionCollectorWorker( _transactionCollector);
        _blockManagerWorker = new BlockManagerWorker(_blockManager, _transactionCollectorWorker);

        start();
    }
   
}

KafkaConsensus::~KafkaConsensus()
{
    if (_config->get_orderer_config()._block_create == "sync") {
        delete _transactionCollector;
        delete _blockManager;
    }
    else
    {
        stop();

        delete _transactionCollectorWorker;
        delete _blockManagerWorker;
    }

    delete _producer;
    delete _consumer;
  
}

void KafkaConsensus::start()
{
    int offset = 0;
    _stop = false;

   _transactionCollectorWorker->start();
   _blockManagerWorker->start();
   
}

void KafkaConsensus::stop()
{
    _stop = true;

    _transactionCollectorWorker->stop();
    _blockManagerWorker->stop();
}

int KafkaConsensus::addTranaction(void* msg, uint16_t sz)
{
    _index++;
    return _producer->send(msg, sz, _index);
}

Block * KafkaConsensus::getBlock() {
    if (_config->get_orderer_config()._block_create == "sync") {
        std::shared_ptr<CollectedTranactions> ct = _transactionCollector->getOrderedTransactions();
        Block * block = _blockManager->createBlock(ct);
        CollectedTXPool::GetInstance()->ReleaseCollectedTx(ct);
        return block;
    }
    else {
        return _blockManagerWorker->getBlock();
    }
  
}

bool KafkaConsensus::test_block(Block* block) {
   _blockManager->test_blockParsing(block);
}
