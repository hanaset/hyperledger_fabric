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
#include "tranactionCollector.h"


TranactionCollector::TranactionCollector(KafkaConsumer * consumer, MED::ConcurrentQueue < std::shared_ptr<CollectedTranactions> > * collected_trans_queue)
{
    _collected_trans_queue = collected_trans_queue;
    _consumer = consumer;
    _stop = false;

}


TranactionCollector::~TranactionCollector()
{
    _stop = true;
    if (_thread.joinable()) _thread.join();
}
void TranactionCollector::start()
{
    CollectedTXPool::GetInstance()->Init();
    _stop = false;
    _thread = std::thread(&TranactionCollector::run, this);
}

void TranactionCollector::stop()
{
    _stop = true;
}

// 현재는 MAX_TRANSACTION_NUN 까지 모아서 블록으로 만들지만 
// 추후에는 blockcutter 클래스를 통해서 n초당 1개 or n개당 1개식의 설정에 따라서 만들어 줘야 한다.
void TranactionCollector::run()
{
    while (!_stop) {
        auto queue_size = _collected_trans_queue->size();
        if (queue_size > 4) {
            sleep(0.1);
            continue;
        }

        MED::StopWatch stopWatch;
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

        _collected_trans_queue->push(ct);

    }
}
