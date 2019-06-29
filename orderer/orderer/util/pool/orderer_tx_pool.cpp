/*
Copyright Medium Corp. 2019 All Rights Reserved.


creator : HAMA
*/

#include "orderer_tx_pool.h"

orderer_tx_pool* orderer_tx_pool::_pInstance_ = nullptr;
std::once_flag orderer_tx_pool::initInstanceFlag;


orderer_tx_pool::orderer_tx_pool()
{
    _bPause = false;
}


void orderer_tx_pool::Init()
{
    for (int i = 0; i < COLLECTED_TX_POOL_SIZE; i++)
        _pool.push_back(std::make_shared<CollectedTranactions>());
}

std::shared_ptr<CollectedTranactions> orderer_tx_pool::GetOrererTx()
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
void orderer_tx_pool::ReleaseOrdererTx(std::shared_ptr<CollectedTranactions> tx)
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
