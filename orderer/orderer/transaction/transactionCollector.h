
#pragma once
#include "collectedTXPooll.h"

class KafkaConsumer;
class TransactionCollector
{
public:
    TransactionCollector(KafkaConsumer * consumer);
    virtual ~TransactionCollector();

    std::shared_ptr<CollectedTranactions> getOrderedTransactions();

protected:

    KafkaConsumer *_consumer;

    static const uint32_t TRANACTION_BUFFER = 1000;
    static const uint32_t MAX_TRANSACTION_NUN = 200000;
};

