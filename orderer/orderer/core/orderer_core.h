#pragma once
#include "../../../network/evpp_server.h"


class BlockManager;
class KafkaConsensus;
class Orderer_config;
class orderer_core
{
public:
    orderer_core();
	orderer_core(uint32_t node_id, uint8_t* secrete, std::string txPort, std::string blPort, std::shared_ptr<Orderer_config>  config);
	virtual ~orderer_core();

    bool start();


    void OnRecvTx(uint32_t conn_id, void* msg, uint16_t sz);
    void OnRecvRequestBlock(uint32_t conn_id, uint16_t type, void* msg, uint16_t sz);


private:

    void init(uint32_t node_id, uint8_t* secrete, std::string txPort, std::string blPort);

    void initailClientServer();
    void initailCommitterServer();
    

private:

    uint32_t _node_id;
    uint8_t*    _secret;
    std::string _txPort;
    std::string _blPort;

    // connection with remote peer
    sptr_evpp_server _tx_svr;
    sptr_evpp_server _commit_svr;

    KafkaConsensus*  _consensus;

    std::shared_ptr<Orderer_config> _config;


    ///////////  test  ////////////
    void test_kafka1();
    void test_kafka2();
};

