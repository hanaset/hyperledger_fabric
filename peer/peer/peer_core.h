#pragma once

#include "../../data_define/data_def.h"
#include "endorser_core.h"
#include "committer_core.h"


#include <mutex>
#include <condition_variable>

class peer_core
{
public:
	peer_core(const uint32_t node_id);
	virtual ~peer_core();

    bool Start();
    void Stop();

private:
    uint32_t _node_id;

    // endorser svr
    endorser_core _endorser_core;

    // committer client
    committer_core _committer_core;
};
