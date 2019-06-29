#include "test_core.h"



test_core::test_core()
{
}


test_core::~test_core()
{

}

bool test_core::Start()
{
	_client_mgr = std::make_shared<evpp_client_manager>(1);
	if (0 > _client_mgr->ConnectTo(11, "127.0.0.1:51050", std::bind(&test_core::OnRecvMsg, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)))
		return false;

	_client_mgr->Start();


	getchar();

//	send_300();
//	send_937();

	requset_block();

	return true;
}

void test_core::OnRecvMsg(uint32_t conn_id, void* msg, uint16_t sz)
{

}


void test_core::send_300()
{
	uint8_t msg[300];
	memset(msg, 'A', 300);
	msg[299] = '\0';

	std::cout << "Starting to Broadcast 300 byte to Server" << std::endl;

	_send_start_time = std::chrono::system_clock::now();

	for (int i = 0; i < (200 * 1000); i++)
	{
		_client_mgr->Broadcast_Message(msg, 300);
	}

	_send_end_time = std::chrono::system_clock::now();
	std::chrono::duration<double> diff = _send_end_time - _send_start_time;
	std::cout << "End of Broadcasting 300 byte to Server [" << diff.count() << "s ]"<< std::endl;
}

void test_core::send_937()
{
	uint8_t msg[937];
	memset(msg, 'A', 937);
	msg[936] = '\0';


	while (1)
	{
		std::cout << "Starting to Broadcast 937 byte to Server" << std::endl;
		_send_start_time = std::chrono::system_clock::now();

		for (int i = 0; i < (2000 * 1000); i++)
		{
			_client_mgr->Broadcast_Message(msg, 937);
		}

		_send_end_time = std::chrono::system_clock::now();
		std::chrono::duration<double> diff = _send_end_time - _send_start_time;
		std::cout << "End of Broadcasting 937 byte to Server [" << diff.count() << "s ]" << std::endl;

		getchar();
	}
}

void test_core::requset_block()
{
	uint32_t sz = 200 * 1024 * 1024;
	uint32_t sz_block;
	uint8_t* pblock = new uint8_t[sz];
	while (1)
	{
		getchar();

		std::cout << "Starting to request block data to Server" << std::endl;
		_send_start_time = std::chrono::system_clock::now();

		sz_block = sz;
		_client_mgr->RequestBlockData(pblock, &sz_block);

		_send_end_time = std::chrono::system_clock::now();
		std::chrono::duration<double> diff = _send_end_time - _send_start_time;
		std::cout << "End of request block data to Server [" << diff.count() << "s ]" << std::endl;
		std::cout << "block : " << sz_block << std::endl;
	}

}