#include "evpp_client_manager.h"

evpp_client_manager::evpp_client_manager(uint32_t my_id)
{
    _my_id = my_id;
}

evpp_client_manager::~evpp_client_manager()
{
	
}

int evpp_client_manager::ConnectTo(uint32_t remote_id, std::string remote_addr, CB_RECV_MSG cb_recv_func, CB_RECV_LMSG cb_recv_lfunc)
{
    sptr_evpp_client client = std::make_shared<evpp_client>(_my_id, remote_id, remote_addr,
        //		_loop.loop(), std::bind(&nt_manager::OnMessage, this, std::placeholders::_1));
        &_client_loop, cb_recv_func);

	if (cb_recv_lfunc != nullptr)
		client->SetCallbackLargeMsg(cb_recv_lfunc);

    _map_client.insert(std::make_pair<>(remote_id, client));

    return (int)_map_client.size();
}

void evpp_client_manager::DisconnectAll()
{
    for (auto itr : _map_client)
    {
        itr.second->Disconnect();
    }
    _map_client.clear();
}

void evpp_client_manager::Start()
{
	_thrd_loop = std::thread([&]() {	 	_client_loop.Run(); });

	for (auto itr : _map_client)
		itr.second->Connect();
}

void evpp_client_manager::Stop()
{
	if (_thrd_loop.joinable())	_thrd_loop.join();
}

int evpp_client_manager::Broadcast_Message(void* msg, uint16_t sz)
{
	int cnt(0);
	for (auto itr : _map_client)
	{
		if (itr.second->SendMessage(msg, sz))
			++cnt;
	}

	return cnt;
}

bool evpp_client_manager::Send_Message(void* msg, uint16_t sz)
{
	bool bret(false);
	if (_map_client.empty())	return false;

	for (auto client : _map_client)
	{
		if ((bret = client.second->SendMessage(msg, sz)) == true)
			break;
	}

	return bret;
}

bool evpp_client_manager::RequestBlockData()
{
	if (_map_client.empty())	return false;

	_map_client.begin()->second->RequestBlockData();
	return true;
}