#pragma once

#include <string>
#include <mutex>
#include <map>
#include <thread>

#include <evpp/event_loop_thread.h>

#include "evpp_client.h"

class evpp_client_manager
{
public:
	evpp_client_manager(uint32_t my_id);
	virtual ~evpp_client_manager();

    int ConnectTo(uint32_t remote_id, std::string remote_addr, CB_RECV_MSG cb_recv_func, CB_RECV_LMSG cb_recv_lfunc);
    void DisconnectAll();

	void Start();
	void Stop();

	// client <=> endorser
	int Broadcast_Message(void* msg, uint16_t sz);

	// client => orderer
	bool Send_Message(void* msg, uint16_t sz);

	// committer <=> orderer
	bool RequestBlockData();

private:
    uint32_t _my_id;

    // Clients
    std::mutex _mtx_map_client;
    std::map<uint32_t, sptr_evpp_client> _map_client;
    evpp::EventLoop _client_loop;

	// thrd
	std::thread _thrd_loop;

	// large msg
	CB_RECV_LMSG _cb_lmsg;
};
typedef std::shared_ptr<evpp_client_manager> sptr_evpp_client_mgr;

