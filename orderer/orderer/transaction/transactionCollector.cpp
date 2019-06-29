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
#include "transactionCollector.h"


TransactionCollector::TransactionCollector(KafkaConsumer * consumer)
{
    _consumer = consumer;
    CollectedTXPool::GetInstance()->Init();
}


TransactionCollector::~TransactionCollector()
{
    CollectedTXPool::ReleaseInstance();
}

std::shared_ptr<CollectedTranactions> TransactionCollector::getOrderedTransactions()
{
    //MED::StopWatch stopWatch;
    //std::cout << "[ORDERER]========================================//" << std::endl;

    uint8_t buff[TRANACTION_BUFFER];
    int status = 0;
    int dsize;
    uint32_t data_len = 0;
    uint32_t trans_cnt = 0;

    std::shared_ptr<CollectedTranactions> ct = CollectedTXPool::GetInstance()->GetCollectedTx();

    uint8_t  *ptransaction = ct->_transaction;

    while (1) {

        dsize = 0;
        _consumer->get(&status, &dsize, buff);

        if ((dsize > 0) || (status != 1)) {
            memcpy(ptransaction, buff, dsize);
            ptransaction += dsize;
            ct->_data_len += dsize;
            ct->_trans_cnt++;
        }
        else {
            sleep(0.1);
        }

        if (ct->_trans_cnt >= MAX_TRANSACTION_NUN) {
            break;
        }

        //if (_trans_time >= MAX_TRANSACTION_TIME) {
        //    break;
        //}
    }
    //std::cout << "[ORDERER] KAFKA-C: " << stopWatch.check() << std::endl;
    //std::cout << "[ORDERER]========================================//" << std::endl;
    return ct;
}
