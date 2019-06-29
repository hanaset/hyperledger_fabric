#pragma once

#include <vector>
#include <list>
#include <string>
#include <mutex>
#include <condition_variable>
#include <thread>

#include "../../../evpp_client_manager.h"

class test_core
{
public:
	test_core();
	virtual ~test_core();

	bool Start();

private:
	void send_300();
	void send_937();
	void requset_block();


	void OnRecvMsg(uint32_t conn_id, void* msg, uint16_t sz);
private:
	std::vector<std::string> _vect_data;

	std::mutex _mtx_que;
	std::condition_variable _cond_que;
	std::list<std::string> _que;


	std::thread _thrd_send;

	sptr_evpp_client_mgr _client_mgr;



	// test result time
	std::chrono::time_point<std::chrono::system_clock> _send_start_time;
	std::chrono::time_point<std::chrono::system_clock> _send_end_time;

	std::chrono::time_point<std::chrono::system_clock> _recv_start_time;
	std::chrono::time_point<std::chrono::system_clock> _recv_end_time;
};

