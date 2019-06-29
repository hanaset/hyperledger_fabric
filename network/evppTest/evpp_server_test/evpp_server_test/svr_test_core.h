#pragma once

#include "../../../evpp_server.h"

#include <vector>
#include <list>
#include <string>
#include <mutex>
#include <condition_variable>
#include <thread>

class test_core
{
public:
	test_core();
	virtual ~test_core();

	bool Start();

private:
	void OnRecvMsg(uint32_t conn_id, void* msg, uint16_t sz);
	void OnRecvCtrl(uint32_t conn_id, uint16_t type, void* msg, uint16_t sz);

	uint64_t _cnt_recv_msg;


private:
	std::vector<std::string> _vect_data;

	std::mutex _mtx_que;
	std::condition_variable _cond_que;
	std::list<std::string> _que;


	std::thread _thrd_send;

	sptr_evpp_server _svr_evpp;

	// test result time
	std::chrono::time_point<std::chrono::system_clock> _send_start_time;
	std::chrono::time_point<std::chrono::system_clock> _send_end_time;

	std::chrono::time_point<std::chrono::system_clock> _recv_start_time;
	std::chrono::time_point<std::chrono::system_clock> _recv_end_time;
};

