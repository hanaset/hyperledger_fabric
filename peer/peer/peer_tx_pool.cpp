#include "peer_tx_pool.h"

#define PEER_TX_POOL_SIZE   (200*1000)

peer_tx_pool::peer_tx_pool()
{
    _bPause = false;
}

peer_tx_pool::~peer_tx_pool()
{
}

void peer_tx_pool::Init()
{
    for (int i = 0; i < PEER_TX_POOL_SIZE; i++)
        _pool.push_back(std::make_shared<PEER_TX>());
}

SPTR_PEER_TX peer_tx_pool::GetPeerTx()
{
    std::unique_lock<std::mutex> ul(_mtx);
    while (1)
    {
        if (_pool.empty())  _cond.wait(ul);

        if (!_pool.empty())
        {
            SPTR_PEER_TX tx = _pool.front();
            _pool.pop_front();
            return tx;
        }
    }

    return nullptr;
}
void peer_tx_pool::ReleasePeerTx(SPTR_PEER_TX tx)
{
    memset(tx->res.byte_buf, 0x00, PRO_RES_SZ);

    std::lock_guard<std::mutex> lg(_mtx);

    if (_pool.empty())  _bPause = true;

    _pool.push_back(tx);

    if (_bPause && _pool.size() > 10000)
    {
        _cond.notify_all();
        _bPause = false;
    }
}
