#pragma once

#include "../../network/evpp_client_manager.h"
#include "../../data_define/data_def.h"
#include "../../common/escc.h"

#include <thread>
#include <vector>

struct BLOCK_DATA
{
    uint32_t szblock;
    std::vector<uint8_t> block;
    std::vector<uint8_t> val_result;
};
typedef std::shared_ptr<BLOCK_DATA> sptr_block_data;


class pool_block
{
public:
    pool_block();
    virtual ~pool_block();
    void init(int max_get);
    sptr_block_data getBlockBuf();
    void releaseBlockBuf(sptr_block_data pblock);

private:
    int _max_get;
    std::mutex _mtx_pool;
    std::condition_variable _cond_pool;
    std::list<sptr_block_data> _pool;
};


class committer_core
{
public:
	committer_core(uint32_t node_id);
	virtual ~committer_core();

    bool Start();
    void Stop();

    void thrd_validate();
    bool thrd_verify(int idx);
    void thrd_commite();
    void thrd_request_block();

    void StartVerifyThread();
    void start_verify();
    void OnRecvBlock(uint32_t id, uint16_t type, void* pbuf, uint32_t sz);

private:
    uint32_t _node_id;

    // evpp to orderer
    sptr_evpp_client_mgr _orderer_mgr;

    // thread request block
    bool _bStop;
    std::thread _thrd_req_block;
    std::thread _thrd_commite;


    // thread pool for verify
    std::condition_variable _cond_start_verify;
    std::condition_variable _cond_end_verify;
    std::vector<std::thread> _vect_thrd_verify;

    std::mutex _mtx_ended_thrd_verify;
    std::list<uint32_t> _ended_thrd_verify;


    // pool buf for block
    pool_block _pool_block;

    // queue for block data
    std::condition_variable _cond_que_block;
    std::list<sptr_block_data> _que_block;

    // queue for validating block data
    std::thread _thrd_validate;
    std::mutex _mtx_que_validate;
    std::condition_variable _cond_que_validate;
    std::list<sptr_block_data> _que_validate;



};

