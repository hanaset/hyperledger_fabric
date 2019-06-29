#pragma once


#include <list>
#include <mutex>
#include <condition_variable>
#include <string.h>

class CollectedTranactions
{
public:
    CollectedTranactions() {
        _transaction = (uint8_t*)malloc(BLOCK_BUFFER);
        _data_len = 0;
        _trans_cnt = 0;
    }
    ~CollectedTranactions() {
        free(_transaction);
    }

    void reset();

    uint8_t* _transaction;

    uint32_t _data_len;
    uint32_t _trans_cnt;

    static const size_t   BLOCK_BUFFER = 300000000; // 300MB

};


class CollectedTXPool
{
public:
    static CollectedTXPool* GetInstance()
    {
        std::call_once(initInstanceFlag, &CollectedTXPool::initSingleton);
        return _pInstance_;
    }
    static void ReleaseInstance() {
        delete _pInstance_;
    }
  
    void Init();
    std::shared_ptr<CollectedTranactions> GetCollectedTx();
    void ReleaseCollectedTx(std::shared_ptr<CollectedTranactions> tx);

private:
    CollectedTXPool();
    ~CollectedTXPool() = default;
    CollectedTXPool(const CollectedTXPool&) = delete;
    CollectedTXPool & operator=(const CollectedTXPool&) = delete;


    static CollectedTXPool* _pInstance_;
    static std::once_flag initInstanceFlag;

    static void initSingleton() {
        _pInstance_ = new CollectedTXPool;
    }

    std::mutex _mtx;
    bool _bPause;
    std::condition_variable _cond;
    std::list<std::shared_ptr<CollectedTranactions>> _pool;

    static const size_t   COLLECTED_TX_POOL_SIZE = 5;

};




