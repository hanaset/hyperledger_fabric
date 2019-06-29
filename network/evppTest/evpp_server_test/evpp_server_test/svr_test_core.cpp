#include "svr_test_core.h"



test_core::test_core()
{
	_cnt_recv_msg = 0;
}


test_core::~test_core()
{
}

bool test_core::Start()
{
	_svr_evpp = std::make_shared<evpp_server>("test_server", 11, "0.0.0.0:51050"
		, std::bind(&test_core::OnRecvMsg, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)
		, std::bind(&test_core::OnRecvCtrl, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)
		);

	_svr_evpp->Start();

	return true;
}

void test_core::OnRecvMsg(uint32_t conn_id, void* msg, uint16_t sz)
{
	if (_cnt_recv_msg % (200 * 1000) == 0)
		_recv_start_time = std::chrono::system_clock::now();

	++_cnt_recv_msg;

	if (_cnt_recv_msg % (200 * 1000) == 0)
	{
		_recv_end_time = std::chrono::system_clock::now();
		std::chrono::duration<double> diff = _recv_end_time - _recv_start_time;

		std::cout << "OnRecvMsg : conn_id[" << conn_id << "] cnt[" << _cnt_recv_msg << " : " << diff.count() << " s]" << std::endl;
//		std::cout << "msg [" << (char*)msg << "] sz [" << sz << "]" << std::endl;
	}
}

void test_core::OnRecvCtrl(uint32_t conn_id, uint16_t type, void* msg, uint16_t sz)
{
	if (type == 10)
	{	// request block data

		// make block data
		uint32_t sz_block = 200 * 1024 * 1024;
		uint8_t* pblock = new uint8_t[sz_block];
		uint8_t* p_pblock = pblock;
		memset(pblock, 'B', sz_block);

		// Start send block size
//		sz_block = 100 * 1024 * 1024;

		_svr_evpp->SendMessage(conn_id, 11, (uint8_t*)&sz_block, 4);

		// middle send block data
		uint32_t sz;
		while (sz_block > 0)
		{
			sz = (sz_block > 4096) ? 4096 : sz_block;
			_svr_evpp->SendMessage(conn_id, 12, p_pblock, sz);
			sz_block -= sz;
			p_pblock += sz;
		}
		
		// End send block signal
		_svr_evpp->SendMessage(conn_id, 13, nullptr, 0);

		delete[] pblock;
	}
}
