/*
Copyright Medium Corp. 2019 All Rights Reserved.


creator : HAMA
*/

#include "orderer_core.h"

#include "../config/orderer_config.h"
#include "../../../common/net_config.h"
#include "../block/block.h"
#include "../consensus/kafkaConsensus.h"
#include "../util/StopWatch.h"

#define __TEST__
#define _TEST_CNT_	(200*1000)

#ifdef __TEST__
std::chrono::time_point<std::chrono::system_clock> recv_start_time;
std::chrono::time_point<std::chrono::system_clock> recv_end_time;
uint64_t _cnt_recv_(0);
#endif // __TEST__

orderer_core::orderer_core(uint32_t node_id, uint8_t* secrete, std::string txPort, std::string blPort, std::shared_ptr<Orderer_config> config)
{
    _node_id = node_id;
    _secret = secrete;
    _txPort = txPort;
    _blPort = blPort;
    _config = config;
}


orderer_core::~orderer_core()
{
    delete _consensus;
}

void orderer_core::initailClientServer()
{
    std::string ipport = _txPort;

    _tx_svr = std::make_shared<evpp_server>("tx_svr", _node_id, ipport
        , std::bind(&orderer_core::OnRecvTx, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)
        , nullptr);


    _tx_svr->Start();
}

void orderer_core::initailCommitterServer()
{
    std::string ipport = _blPort;
   
    _commit_svr = std::make_shared<evpp_server>("commit_svr", _node_id, ipport
        , nullptr
        , std::bind(&orderer_core::OnRecvRequestBlock, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)
        );

    _commit_svr->Start();
}

bool orderer_core::start()
{
    //consensus start 
    _consensus = new KafkaConsensus(_config, _node_id, _secret);

    // peer communication start
    initailClientServer();
    initailCommitterServer();

    //test_kafka1();
    //test_kafka2();

}

void orderer_core::test_kafka1()
{
    _config->get_kafka_config()._topic = "mediumcoin-test-lsh";
    _config->get_kafka_config()._queue_buffer_max_messages = "20000000";
    _config->get_kafka_config()._acks = "0";


    _consensus = new KafkaConsensus(_config, _node_id, _secret);
    MED::StopWatch stopWatch;

    std::vector<char> vec(600, 'a');

    for (int j = 0; j < 3; j++) {
        for (int i = 0; i < 200000; i++)
        {
            int error = _consensus->addTranaction((void*)&vec[0], 600);
          
        }
        std::cout << "producing : " << stopWatch.check() << std::endl;
    }

    std::cout << "producing total : " << stopWatch.stop() << std::endl;

    for (int j = 0; j < 3; j++) {
        std::cout << "rotation: " << j << std::endl;

        Block* block = _consensus->getBlock();
        delete block;

        std::cout << "consuming : " << stopWatch.check() << std::endl;
    }
    std::cout << "consuming total : " << stopWatch.stop() << std::endl;

}


void orderer_core::test_kafka2()
{

    _config->get_kafka_config()._topic = "mediumcoin-test-lsh";
    _config->get_kafka_config()._queue_buffer_max_messages = "20000000";
    _config->get_kafka_config()._acks = "0";


    _consensus = new KafkaConsensus(_config, _node_id, _secret);

    for (int j = 0; j < 100; j++) {
        std::cout << "[ORDERER]========================================//" << std::endl;
        MED::StopWatch stopWatch;
        Block* block = _consensus->getBlock();
        _consensus->test_block(block);
        std::cout << "[ORDERER] Request Block Time : " << stopWatch.check() << std::endl;
        std::cout << "[ORDERER]========================================//" << std::endl;
    }

}


void orderer_core::OnRecvTx(uint32_t conn_id, void* msg, uint16_t sz)
{
    // push to kafka
#ifdef __TEST__
	if ((_cnt_recv_ % _TEST_CNT_) == 0)
		recv_start_time = std::chrono::system_clock::now();
#endif __TEST__
    _consensus->addTranaction(msg, sz);

#ifdef __TEST__
    ++_cnt_recv_;

    if ((_cnt_recv_ % _TEST_CNT_) == 0)
    {
        _cnt_recv_ = 0;
        recv_end_time = std::chrono::system_clock::now();
        std::chrono::duration<double> diff = recv_end_time - recv_start_time;
        std::cout << "End of Recv 400*1000 tx from Client [ " << diff.count() << "s ]" << std::endl;
    }
#endif // __TEST__
}


void orderer_core::OnRecvRequestBlock(uint32_t conn_id, uint16_t type, void* msg, uint16_t sz)
{
  
    if (type == 10)
    {	
        std::cout << "[ORDERER] conn_id: " << conn_id << std::endl;
        std::cout << "[ORDERER]========================================//" << std::endl;
        MED::StopWatch stopWatch;
        Block* block = _consensus->getBlock();
        uint8_t* pblock = block->_block;

        // Start send block signal with block-size
        _commit_svr->Broadcast_Message(11, (uint8_t*)&block->_size, sizeof(uint32_t));

        // middle s block data
        uint32_t sz;
        uint32_t blocksize = block->_size;
        while (block->_size > 0)
        {
            sz = (block->_size > 4096) ? 4096 : block->_size;
            _commit_svr->Broadcast_Message(12, pblock, sz);
            block->_size -= sz;
            pblock += sz;
        }

        // End send block signal
        _commit_svr->Broadcast_Message(13, nullptr, 0);

        std::cout << "[ORDERER] Request Block Time : " << stopWatch.check() << std::endl;
        std::cout << "[ORDERER]========================================//" << std::endl;

        delete block;
        
    }
  
   
}

