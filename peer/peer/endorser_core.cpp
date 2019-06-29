#include "endorser_core.h"
#include "../../common/net_config.h"

#include "../../common/escc.h"
#include <openssl/sha.h>

#define __TEST__
#define _TEST_CNT_  (200*1000)

#ifdef __TEST__
std::chrono::time_point<std::chrono::system_clock> send_start_time;
std::chrono::time_point<std::chrono::system_clock> send_end_time;
std::mutex _mtx_cnt_send_;
uint64_t _cnt_send_(0);
std::chrono::time_point<std::chrono::system_clock> recv_start_time;
std::chrono::time_point<std::chrono::system_clock> recv_end_time;
uint64_t _cnt_recv_(0);
#endif // __TEST __


endorser_core::endorser_core(uint32_t node_id)
{
    _node_id = node_id;
}


endorser_core::~endorser_core()
{
}

bool endorser_core::Start(std::string addr, std::string addr_cc, uint32_t e_thread)
{
    _addr = addr;

    // tx_pool
    _pool_tx = peer_tx_pool::GetInstance();
    _pool_tx->Init();

    // chaincode
    _chaincode_support = std::make_shared<chaincode_support>();
    _chaincode_support->Start(addr_cc);

    // worker thread pool
    _bStop = false;
    _pool_worker.resize(e_thread);
    for (int i = 0; i < (int)e_thread; i++)
        _pool_worker[i] = std::thread([&](int i) { endorser_worker(i); }, i);

    // start server
    _svr = std::make_shared<evpp_server>("EndorserServer", _node_id, _addr
        , std::bind(&endorser_core::RecvRequestMsg, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)
        , std::bind(&endorser_core::RecvCtrlMsg, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)
        );

    return _svr->Start();
}

void endorser_core::Stop()
{

}

void endorser_core::RecvRequestMsg(uint32_t id, void* msg, uint16_t sz)
{
    SPTR_PEER_TX tx = _pool_tx->GetPeerTx();
    net_config* conf = net_config::GetInstance();
    uint32_t client_id;

    tx->conn_id = id;
    tx->req.CopyFrom(msg, sz);

    memcpy((void*)&client_id, (void*)tx->req.creator, sizeof(uint32_t));
    memcpy(tx->client_pkey, conf->_clients.find(client_id)->second->pkey, 64);

    std::lock_guard<std::mutex> lg(_mtx_que_tx);
    _que_tx.push_back(tx);
    _cond_que_tx.notify_all();
}

void endorser_core::RecvCtrlMsg(uint32_t id, uint16_t type, void* msg, uint16_t sz)
{

}


void endorser_core::endorser_worker(int thrd_id)
{
    std::unique_lock<std::mutex> ul(_mtx_que_tx, std::defer_lock);

    escc es;
    net_config* conf = net_config::GetInstance();

    SPTR_PEER_TX tx;
    bool bret(false);
    std::string result;

    uint8_t peer_skey[32];
    memcpy(peer_skey, conf->_peers.find(_node_id)->second->skey, 32);

    std::list<SPTR_PEER_TX> que_tx;

    while (!_bStop)
    {
        ul.lock();
        if (_que_tx.empty())    _cond_que_tx.wait(ul);

        if (!_que_tx.empty()) {
            tx = _que_tx.front();
            _que_tx.pop_front();
        }
        else
            tx.reset();

//        que_tx.swap(_que_tx);
        ul.unlock();

        if (tx.get() != nullptr)
//        while (!que_tx.empty())
        {
//            tx = que_tx.front();
//            que_tx.pop_front();

            tx->res.sz_data = PRO_RES_SZ - 128;
            memcpy(tx->res.endorser, &_node_id, sizeof(uint32_t));

                    // verify
                    bret = es.verify(tx->req.creator_sign, tx->client_pkey, tx->req.tx_id);
                    bret = true;    // test
                    if (bret) // verify success
                    {   
                        uint8_t status  = _chaincode_support->Simulation(thrd_id, *tx->req.ch_id, *tx->req.cc_id, tx->req.cc_arg, result);
                        (*tx->res.status) = status;
                        memcpy(tx->res.result, result.data(), result.size());
                        tx->res.sz_data += (uint16_t)result.size();

                        if (status == RES_STATUS_ERROR)
                        {
                            std::cout << "Fail to Simulation..." << result << std::endl;
                        }
                    }
                    else
                    {
                        (*tx->res.status) = RES_STATUS_ERROR;
                        std::cout << "Fail to verify..." << std::endl;
                    }

                    if (bret && (*tx->res.status) == RES_STATUS_UPDATE)
                    {
                        // sign
                        SHA256((const uint8_t*)result.data(), result.size(), tx->res.proposal_hash);
                        bret = es.sign(tx->res.proposal_hash, peer_skey, tx->res.sign);
                    }
            
#ifdef __TEST__
            if ((_cnt_send_ % _TEST_CNT_) == 0)
                send_start_time = std::chrono::system_clock::now();
#endif // __TEST__


            // send response to client
            memcpy(tx->res.msg_uid, tx->req.msg_uid, sizeof(uint32_t));
            _svr->SendMessage(tx->conn_id, 1, tx->res.byte_buf, tx->res.sz_data);
            _pool_tx->ReleasePeerTx(tx);

#ifdef __TEST__
            std::unique_lock<std::mutex> ul_test(_mtx_cnt_send_);
            ++_cnt_send_;

            if ((_cnt_send_ % _TEST_CNT_) == 0)
            {
                send_end_time = std::chrono::system_clock::now();
                std::chrono::duration<double> diff = send_end_time - send_start_time;
                std::cout << "End of response 200*1000 tx to client [ " << diff.count() << "s ]" << std::endl;
            }
            ul_test.unlock();
#endif // __TEST__
        }
    }
}
