#pragma once
#include "../../data_define/data_def.h"
#include "../../network/evpp_server.h"
#include "peer_tx_pool.h"

#include "chaincode/chaincode_support.h"
#include <condition_variable>

class endorser_core
{
public:
	endorser_core(uint32_t node_id);
	virtual ~endorser_core();

    bool Start(std::string addr, std::string addr_cc, uint32_t e_thread);
    void Stop();

    typedef std::function<void(uint32_t, void*, uint16_t)>	CB_RECV_MSG;

    void RecvRequestMsg(uint32_t id, void* msg, uint16_t sz);
    void RecvCtrlMsg(uint32_t id, uint16_t type, void* msg, uint16_t sz);



    void endorser_worker(int thrd_id);

private:
    uint32_t _node_id;
    std::string _addr;

    sptr_evpp_server _svr;
    peer_tx_pool* _pool_tx;






    sptr_cc_support _chaincode_support;

    // que tx
    std::mutex _mtx_que_tx;
    std::condition_variable _cond_que_tx;
    std::list<SPTR_PEER_TX> _que_tx;

    // worker thread
    bool _bStop;
    std::vector<std::thread> _pool_worker;

};

