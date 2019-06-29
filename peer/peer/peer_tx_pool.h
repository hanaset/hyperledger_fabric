#pragma once

#include "../../data_define/data_def.h"

#include <list>
#include <mutex>
#include <condition_variable>

class peer_tx_pool
{
public:
    static peer_tx_pool* GetInstance()
    {
        static peer_tx_pool* _pInstance_ = new peer_tx_pool;
        return _pInstance_;
    }
	virtual ~peer_tx_pool();

    void Init();
    SPTR_PEER_TX GetPeerTx();
    void ReleasePeerTx(SPTR_PEER_TX tx);

private:
    peer_tx_pool();

    std::mutex _mtx;
    bool _bPause;
    std::condition_variable _cond;
    std::list<SPTR_PEER_TX> _pool;
};

