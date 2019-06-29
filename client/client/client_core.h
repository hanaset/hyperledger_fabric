#pragma once

#include "../../data_define/data_def.h"
#include "../../network/evpp_client_manager.h"

#include <condition_variable>
#include <map>
#include <thread>
#include <vector>

enum Mode {
    TRANSFER = 0,
    CHAINCODE1,
    CHAINCODE2
};

class client_core
{
public:
	client_core(uint32_t node_id, uint32_t mode);
	virtual ~client_core();

    bool Start();
    void Stop();

	void thrd_send_endorser();
    void thrd_recv_endorser();
    void thrd_send_orderer();

    void cb_recv_endorser(uint32_t endorser_id, void* msg, uint16_t sz_msg);
    void ready_tx_data(uint8_t*  skey, uint8_t* pkey);

    void cb_recv_orderer(uint32_t endorser_id, void* msg, uint16_t sz_msg);
private:
	bool _bStop;
    const uint32_t _node_id;
    const uint32_t _mode;

	// transaction data
    std::vector<SPTR_TRANSACTION> _tx_data;
    std::vector<SPTR_TX_INDEX> _tx_data_index;

    // send-stanby que
    std::mutex _mtx_send_stanby;
    std::condition_variable _cond_send_stanby;
    std::list<uint32_t> _que_send_stanby;

	// send proposal to endorser peer
    std::thread _thrd_send_endorser;
    std::thread _thrd_recv_endorser;

	std::mutex _mtx_recv_endorser;
    std::condition_variable _cond_recv_endorser;
	std::list<SPTR_PROPOSAL_RES> _que_recv_endorser;

	// response pool
	std::mutex _mtx_pool_respons;
	std::list<SPTR_PROPOSAL_RES> _pool_respons;

    // evpp to endorser
    sptr_evpp_client_mgr _endorser_mgr;

    // evpp to orderer
    sptr_evpp_client_mgr _orderer_mgr;

    // thrd sendo to orderer
    std::thread _thrd_send_orderer;

    std::mutex _mtx_send_orderer;
    std::condition_variable _cond_send_orderer;
    std::list<uint32_t> _que_send_orderer;
};
