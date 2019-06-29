#include "client_core.h"
#include "../../common/net_config.h"
#include "workload.h"
#include "../../common/utils.h"

#include <iostream>

#define __TEST__
#define _TEST_CNT_  (400*1000)

#ifdef __TEST__
std::chrono::time_point<std::chrono::system_clock> send_start_time;
std::chrono::time_point<std::chrono::system_clock> send_end_time;
uint64_t _cnt_send_(0);
std::chrono::time_point<std::chrono::system_clock> recv_start_time;
std::chrono::time_point<std::chrono::system_clock> recv_end_time;
uint64_t _cnt_recv_(0);
std::chrono::time_point<std::chrono::system_clock> orderer_start_time;
std::chrono::time_point<std::chrono::system_clock> orderer_end_time;
uint64_t _cnt_orderer_(0);
#endif // __TEST __

#define ENDORSEMENT_POLICY	3
#define TEST_CH 10  
#define TEST_CC 9 // 9: vtcc, 11: mtcc
#define SZ_WORK_DATA    (400*1000)

client_core::client_core(uint32_t node_id, uint32_t mode)
    : _node_id(node_id), _mode(mode)
{
}

client_core::~client_core()
{
}

void client_core::ready_tx_data(uint8_t* skey, uint8_t* pkey)
{
    // load tx data
    //workload wl(skey);
    //wl.make_test_data(_node_id, TEST_CH, TEST_CC, _tx_data, SZ_WORK_DATA);

    std::string file("client1_workload");
    std::string file_mrc("client1_workload_mrc1");
    std::string formatA("_A.dat");
    std::string formatB("_B.dat");

    file[6] += _node_id - 1;
    file_mrc[6] += _node_id - 1;

    std::string pathA, pathB;

    workload_vt wl(skey, pkey, _node_id);

    int num = 0;
    
    switch (_mode) {
        case Mode::TRANSFER:
            pathA = file + formatA;
            pathB = file + formatB;
            break;
        case Mode::CHAINCODE1:
            pathA = file_mrc + formatA;
            pathB = file_mrc + formatB;
            break;
        case Mode::CHAINCODE2:
            file_mrc[20] += 1;
            pathA = file_mrc + formatA;
            pathB = file_mrc + formatB;
            break;
    }

    num = wl.first_load_workload(pathA, _tx_data);
    wl.second_load_workload(pathB, _tx_data, num);

    // make tx index
    SPTR_TX_INDEX tx_index;
    _tx_data_index.resize(_tx_data.size());
    for (uint32_t i = 0; i < _tx_data.size(); i++)
    {
        tx_index = std::make_shared<TX_INDEX>();
        tx_index->msg_uid = i;
        memcpy(tx_index->tx_id, _tx_data[i]->tx_id, 32);
        tx_index->status = 0;
        tx_index->cnt_send = 0;
        _tx_data_index[i] = tx_index;

        _que_send_stanby.push_back(i);
    }

    // response pool
    for (size_t i = 0; i < _tx_data.size() * 5; i++)
        _pool_respons.push_back(std::make_shared<PROPOSAL_RES>());
}

bool client_core::Start()
{
    bool bret(true);
	_bStop = false;

    net_config* conf = net_config::GetInstance();
    auto client_info = conf->_clients.find(_node_id);
    
    ready_tx_data(client_info->second->skey, client_info->second->pkey);

    std::string ipport;
    size_t cnt(0);

    _thrd_recv_endorser = std::thread([&]() { thrd_recv_endorser(); });

    // orderer connecting
    _orderer_mgr = std::make_shared<evpp_client_manager>(_node_id);
    for (auto itr : conf->_orderers)
    {
        ipport = itr.second->listen_tx;
        cnt = _orderer_mgr->ConnectTo(itr.first, ipport, std::bind(&client_core::cb_recv_orderer, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3), nullptr);
    }
    _orderer_mgr->Start();

    std::cout << "Connected Orderer Server [count : " << cnt << "]" << std::endl;

    // endorser connecting
    _endorser_mgr = std::make_shared<evpp_client_manager>(_node_id);
    for (auto itr : conf->_peers)
    {
        ipport = itr.second->endorser_listen;
        cnt = _endorser_mgr->ConnectTo(itr.first, ipport, std::bind(&client_core::cb_recv_endorser, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3), nullptr);
    }

    _endorser_mgr->Start();
    std::cout << "Connected Endorser Server [count : " << cnt << "]" << std::endl;

    if (cnt == 0)   return false;

    // Start Sending Proposal to Peers
    std::cout << "Ready to Send Proposals [Peers : " << cnt << "] !!!" << std::endl;
    getchar();

    _thrd_send_orderer = std::thread([&]() {   thrd_send_orderer(); });
    _thrd_send_endorser = std::thread([&]() {   thrd_send_endorser(); });

    return bret;
}

void client_core::Stop()
{

}

void client_core::thrd_send_endorser()
{
    SPTR_TRANSACTION tx;
    SPTR_TX_INDEX tx_idx;
    SPTR_PROPOSAL_REQ req = std::make_shared<PROPOSAL_REQ>();

    std::unique_lock<std::mutex> ul(_mtx_send_stanby, std::defer_lock);
    uint32_t idx;
	while (!_bStop)
	{
        ul.lock();
        if (_que_send_stanby.empty())
            _cond_send_stanby.wait(ul);
        if (_que_send_stanby.empty())
        {
            ul.unlock();
            continue;
        }
        idx = _que_send_stanby.front();
        _que_send_stanby.pop_front();
        ul.unlock();


        tx = _tx_data[idx];
		if (tx.get() == nullptr)	continue;
        tx_idx = _tx_data_index[idx];

        memset(req->byte_buf, 0x00, PRO_REQ_SZ);

        memcpy(&req->sz_data, tx->sz_proposal, 2);
        memcpy((void*)req->msg_uid, (void*)&tx_idx->msg_uid, sizeof(uint32_t));
        memcpy((void*)req->sz_proposal, (void*)tx->sz_proposal, req->sz_data + 2);
        memcpy((void*)req->creator, (void*)&_node_id, sizeof(uint32_t));
        req->sz_data += (uint16_t)(2+4);


#ifdef __TEST__
        if ((_cnt_send_ % _TEST_CNT_) == 0)
            send_start_time = std::chrono::system_clock::now();
#endif // __TEST__

        // send
        tx_idx->status = 1;
        tx_idx->cnt_send = _endorser_mgr->Broadcast_Message(req->byte_buf, req->sz_data);
        for (int i = 0; i < 5000; i++);
        
        if (tx_idx->cnt_send == 0)
        {   //  failed to send
            tx_idx->status = 0;
            ul.lock();
            _que_send_stanby.push_back(tx_idx->msg_uid);
            ul.unlock();
            _cond_send_stanby.notify_all();
        }

#ifdef __TEST__
        else {
            ++_cnt_send_;

            if ((_cnt_send_ % _TEST_CNT_) == 0)
            {
                _cnt_send_ = 0;
                send_end_time = std::chrono::system_clock::now();
                std::chrono::duration<double> diff = send_end_time - send_start_time;
                std::cout << "End of Broadcasting 400*1000 tx to Server [ " << diff.count() << "s ]" << std::endl;

//                getchar();
            }
        }
#endif // __TEST__
    }
}

void client_core::cb_recv_endorser(uint32_t endorser_id, void* msg, uint16_t sz_msg)
{
#ifdef __TEST__
    if ((_cnt_recv_ % _TEST_CNT_) == 0)
        recv_start_time = std::chrono::system_clock::now();
#endif // __TEST__

    SPTR_PROPOSAL_RES res;

    std::unique_lock<std::mutex> ul_pool_res(_mtx_pool_respons);
    if (_pool_respons.size() == 0)
        res = std::make_shared<PROPOSAL_RES>();
    else
    {
        res = _pool_respons.front();
        _pool_respons.pop_front();
    }
    ul_pool_res.unlock();

    res->CopyFrom(msg, sz_msg);

    std::unique_lock<std::mutex> ul_que_res(_mtx_recv_endorser);
    _que_recv_endorser.push_back(res);
    _cond_recv_endorser.notify_all();

#ifdef __TEST__
    //std::cout << "cb_recv_endorser: msg_uid: " << *res->msg_uid << ", endorser: " << *res->endorser << ", status: " << (uint32_t)*res->status << ", result: " << res->result << std::endl;
    
    ++_cnt_recv_;
    if ((_cnt_recv_ % (_TEST_CNT_/4)) == 0)
    {
        _cnt_recv_ = 0;
        recv_end_time = std::chrono::system_clock::now();
        std::chrono::duration<double> diff = recv_end_time - recv_start_time;
        std::cout << "End of recv response 200*1000 to Server [ " << diff.count() << "s ]" << std::endl;
    }
#endif // __TEST__
}


void client_core::thrd_recv_endorser()
{
    SPTR_TRANSACTION tx;
    SPTR_TX_INDEX tx_idx;

    SPTR_PROPOSAL_RES res;
    std::unique_lock<std::mutex> ul_que_res(_mtx_recv_endorser, std::defer_lock);
    std::list<SPTR_PROPOSAL_RES> que_recv;

    while (!_bStop)
    {
        ul_que_res.lock();
        if (_que_recv_endorser.empty())
            _cond_recv_endorser.wait(ul_que_res);

        que_recv.swap(_que_recv_endorser);
        ul_que_res.unlock();

        while (!que_recv.empty())
        {
            res = que_recv.front();
            que_recv.pop_front();

            if (res.get() == nullptr)   continue;


            if ((*res->msg_uid) >= _tx_data_index.size())
            {
                std::cout << "recv msg_uid is failed [ " << _cnt_recv_ << ":"<< *res->msg_uid << " ]" << std::endl;
                std::lock_guard<std::mutex> lg(_mtx_pool_respons);
                _pool_respons.push_back(res);
                continue;
            }

            tx_idx = _tx_data_index[*res->msg_uid];
            tx = _tx_data[tx_idx->msg_uid];
            if (tx_idx.get() != nullptr && tx.get() != nullptr && tx_idx->status == 1)
            {
                --tx_idx->cnt_send;
                if ((*res->status) == 1)
                {
                    if (tx_idx->list_response.empty())
                    {
                        memcpy(tx->response_hash, res->proposal_hash, 32);
                        memcpy(tx->response, res->result, 128);
                        tx_idx->list_response.push_back(res);
                        res.reset();
                    }
                    else
                    {
                        if (compare_bytes(tx->response_hash, res->proposal_hash, 32)
                            && compare_bytes((uint8_t*)tx->response, (uint8_t*)res->result, 128))
                        {
                            tx_idx->list_response.push_back(res);
                            res.reset();
                        }
                    }

                    if (tx_idx->list_response.size() == ENDORSEMENT_POLICY)
                    {
                        tx_idx->status = 2;

                        // push to send to orderer
                        std::lock_guard<std::mutex> lg(_mtx_send_orderer);
                        _que_send_orderer.push_back(tx_idx->msg_uid);
                        _cond_send_orderer.notify_all();
                    }
                }

                if (tx_idx->cnt_send <= 0 && tx_idx->status == 1)
                {   // failed transaction
                    std::unique_lock<std::mutex> ul_pool_res(_mtx_pool_respons);
                    while (!tx_idx->list_response.empty())
                    {
                        _pool_respons.push_back(tx_idx->list_response.front());
                        tx_idx->list_response.pop_front();
                    }
                    ul_pool_res.unlock();
                    tx_idx->list_response.clear();

                    tx_idx->cnt_send = 0;
                    tx_idx->status = 0;

                    std::unique_lock<std::mutex> ul_send_stanby(_mtx_send_stanby);
                    _que_send_stanby.push_back(tx_idx->msg_uid);
                    _cond_send_stanby.notify_all();

                    std::cout << "Failed Endorserment of tx [" << tx_idx->msg_uid << "]" << std::endl;
                }
            }

            if (res.get() != nullptr)
            {
                std::lock_guard<std::mutex> lg(_mtx_pool_respons);
                _pool_respons.push_back(res);
            }
        }   // while (que_recv)
    }   // while (_bStop)
}

void client_core::thrd_send_orderer()
{
    uint32_t idx;
    SPTR_TRANSACTION tx;
    SPTR_TX_INDEX tx_idx;
    SPTR_PROPOSAL_RES res;

    std::unique_lock<std::mutex> ul_send_stanby(_mtx_send_stanby, std::defer_lock);
    std::unique_lock<std::mutex> ul_pool_respons(_mtx_pool_respons, std::defer_lock);
    std::unique_lock<std::mutex> ul_send_orderer(_mtx_send_orderer, std::defer_lock);
    std::list<uint32_t> que_orderer;

    int i;
    while (!_bStop)
    {
        ul_send_orderer.lock();

        if (_que_send_orderer.empty())
            _cond_send_orderer.wait(ul_send_orderer);

        que_orderer.swap(_que_send_orderer);
        ul_send_orderer.unlock();

        while (!que_orderer.empty())
        {
            idx = que_orderer.front();
            que_orderer.pop_front();

            tx = _tx_data[idx];
            tx_idx = _tx_data_index[idx];

            i = 1;
            while (!tx_idx->list_response.empty())
            {
                res = tx_idx->list_response.front();
                tx_idx->list_response.pop_front();

                if (i == 1)
                {
                    memcpy(tx->endorsor_1_id, res->endorser, 4); // To-do : tx->endorsor 배열로 
                    memcpy(tx->endorsor_1_sign, res->sign, 64);



                    //memcpy(tx->endorsor_2_id, res->endorser, 4);
                    //memcpy(tx->endorsor_2_sign, res->sign, 64);
                    //
                    //memcpy(tx->endorsor_3_id, res->endorser, 4);
                    //memcpy(tx->endorsor_3_sign, res->sign, 64);
                }
                else if (i == 2)
                {
                    memcpy(tx->endorsor_2_id, res->endorser, 4);
                    memcpy(tx->endorsor_2_sign, res->sign, 64);
                }
                else if (i == 3)
                {
                    memcpy(tx->endorsor_3_id, res->endorser, 4);
                    memcpy(tx->endorsor_3_sign, res->sign, 64);
                }
                ++i;

                ul_pool_respons.lock();
                _pool_respons.push_back(res);
                ul_pool_respons.unlock();
            }
#ifdef __TEST__
            if ((_cnt_orderer_ % _TEST_CNT_) == 0)
                orderer_start_time = std::chrono::system_clock::now();
#endif // __TEST__

            _orderer_mgr->Send_Message(tx->byte_buf, tx->sz_data);
#ifdef __TEST__
            //std::cout << "cb_recv_endorser: msg_uid: " << *res->msg_uid << ", endorser: " << *res->endorser << ", status: " << (uint32_t)*res->status << ", result: " << res->result << std::endl;

            ++_cnt_orderer_;
            if ((_cnt_orderer_ % _TEST_CNT_) == 0)
            {
                _cnt_orderer_ = 0;
                orderer_end_time = std::chrono::system_clock::now();
                std::chrono::duration<double> diff = orderer_end_time - orderer_start_time;
                std::cout << "End of send tx 400*1000 to orderer [ " << diff.count() << "s ]" << std::endl;
            }
#endif // __TEST__

            memset((void*)tx->endorsor_1_id, 0x00, SZ_TRANSACTION - OFFSET_ENDORSER);

            tx_idx->cnt_send = 0;
            tx_idx->status = 0;
            tx_idx->list_response.clear();

            ul_send_stanby.lock();
            _que_send_stanby.push_back(tx_idx->msg_uid);
            ul_send_stanby.unlock();
            _cond_send_stanby.notify_all();
        }
    }
}

void client_core::cb_recv_orderer(uint32_t endorser_id, void* msg, uint16_t sz_msg)
{
    std::cout << "Recieve msg from orderer ..." << std::endl;
}
