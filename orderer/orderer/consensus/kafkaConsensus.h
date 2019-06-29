#pragma once

#include "../util/ConcurrentQueue.h"


class CollectedTranactions;
class KafkaConsumer;
class KafkaProducer;
class Block;
class TransactionCollector;
class TransactionCollectorWorker;
class BlockManager;
class BlockManagerWorker;
class KafkaConsensus

{
public:
    KafkaConsensus(std::shared_ptr<Orderer_config> & config, uint32_t node_id, uint8_t* secrete);
    ~KafkaConsensus();

    void start();
    void stop();

    Block * getBlock();

    int addTranaction(void* msg, uint16_t sz);
    
private:
    bool _stop;

    MED::ConcurrentQueue< std::shared_ptr<CollectedTranactions> > * _collected_trans_queue;
    MED::ConcurrentQueue<Block*> * _block_queue;

    uint32_t    _node_id;
    uint8_t*    _secret;

    TransactionCollector* _transactionCollector;
    TransactionCollectorWorker* _transactionCollectorWorker;

    BlockManager* _blockManager;
    BlockManagerWorker* _blockManagerWorker;

    KafkaProducer *_producer;
    KafkaConsumer *_consumer;
    int64_t _index = 0;
    std::shared_ptr<Orderer_config>  _config;

public:
    ////////// test ////////////
    bool test_block(Block* block);
};



