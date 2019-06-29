/*
Copyright Medium Corp. 2019 All Rights Reserved.


creator : HAMA
*/


#include "collectedTXPooll.h"


void CollectedTranactions::reset() {
    memset(_transaction, 0x00, BLOCK_BUFFER);
    _data_len = 0;
    _trans_cnt = 0;

}

CollectedTXPool* CollectedTXPool::_pInstance_ = nullptr;
std::once_flag CollectedTXPool::initInstanceFlag;

CollectedTXPool::CollectedTXPool()
{
    _bPause = false;
}


void CollectedTXPool::Init()
{
    for (int i = 0; i < COLLECTED_TX_POOL_SIZE; i++)
        _pool.push_back(std::make_shared<CollectedTranactions>());
}

std::shared_ptr<CollectedTranactions> CollectedTXPool::GetCollectedTx()
{
    std::unique_lock<std::mutex> ul(_mtx);
    while (1)
    {
        if (_pool.empty())  _cond.wait(ul);

        if (!_pool.empty())
        {
            std::shared_ptr<CollectedTranactions> tx = _pool.front();
            _pool.pop_front();
            return tx;
        }
    }

    return nullptr;
}
void CollectedTXPool::ReleaseCollectedTx(std::shared_ptr<CollectedTranactions> tx)
{
    tx->reset();

    std::lock_guard<std::mutex> lg(_mtx);

    if (_pool.empty())  _bPause = true;

    _pool.push_back(tx);

    if (_bPause && _pool.size() > 0)
    {
        _cond.notify_all();
        _bPause = false;
    }
}
