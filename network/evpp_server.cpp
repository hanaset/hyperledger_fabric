#include "evpp_server.h"


#define EVPP_THREAD_NUM	5

evpp_server::evpp_server(std::string svr_name, uint32_t my_id, std::string listen_addr
	, CB_RECV_MSG cb_recv_msg, CB_RECV_CTRL cb_recv_ctrl)
	: _tcp_svr(_loop.loop(), listen_addr, svr_name, EVPP_THREAD_NUM)
{
    _svr_name = svr_name;
    _my_id = my_id;

    _listen_ipport = listen_addr;

    _cb_recv_msg = cb_recv_msg;
	_cb_recv_ctrl = cb_recv_ctrl;

    _tcp_svr.SetMessageCallback(std::bind(&evpp_server::OnMessage, this, std::placeholders::_1, std::placeholders::_2));
    _tcp_svr.SetConnectionCallback(std::bind(&evpp_server::OnConnection, this, std::placeholders::_1));
}

evpp_server::~evpp_server()
{
}

bool evpp_server::Start()
{
	_cur_conn_id = 0;
	_cnt_req_block = 0;

	bool bret(false);
	if (_tcp_svr.Init())
		if (_tcp_svr.Start())
		{
			_loop.Start();
			bret = true;
			std::cout << "Started Server... " << _svr_name << ":" << _listen_ipport << "]" << std::endl;
		}

	return bret;
}

void evpp_server::Stop()
{

}



void evpp_server::OnConnection(const evpp::TCPConnPtr& conn)
{
    std::string addr = conn->remote_addr();

    if (conn->IsConnected())
    {
		std::lock_guard<std::mutex> lg_id(_mtx_conn_id);

        sptr_evpp_tcpconn pConn =
            std::make_shared<evpp_tcpConn>(conn, _cur_conn_id,
                _cb_recv_msg,
                std::bind(&evpp_server::OnRecvRequestBlock, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)
                );

        std::lock_guard<std::mutex> lg(_mtx_map_connect);
        _map_conn.insert(std::make_pair(conn, pConn));
		_map_connid.insert(std::make_pair(_cur_conn_id, pConn));

		std::cout << "Accepted Client ... [" << _cur_conn_id << ":" << addr << "]" << std::endl;

		++_cur_conn_id;

    }
    else
    {
		std::lock_guard<std::mutex> lg(_mtx_conn_id);

		auto itr = _map_conn.find(conn);
		uint16_t id = (uint16_t)itr->second->GetID();

		_map_connid.erase(id);
		_map_conn.erase(conn);

        std::cout << "Disconnected Client ... [" << id << ":" << addr << "]" << std::endl;
    }
}

void evpp_server::OnMessage(const evpp::TCPConnPtr& conn, evpp::Buffer* msg)
{
    //	std::lock_guard<std::mutex> lg(_mtx_map_connect);
    auto itr = _map_conn.find(conn);
    if (itr != _map_conn.end())
    {
        itr->second->OnRecvMessage(msg);
    }
}

bool evpp_server::SendMessage(uint32_t conn_id, uint16_t type, uint8_t* msg, uint16_t sz)
{
	bool bret(true);

	if (type == 1)
	{
		auto itr = _map_connid.find(conn_id);
		if (itr == _map_connid.end())
			return false;

		return itr->second->SendMessage(type, msg, sz);
	}
	else
	{
		for (auto item : _map_connid)
			item.second->SendMessage(type, msg, sz);
	}
	return bret;
}

int evpp_server::Broadcast_Message(uint16_t type, void* msg, uint16_t sz)
{
	int cnt(0);
	for (auto itr : _map_connid)
	{
		if (itr.second->SendMessage(type, msg, sz))
			++cnt;
	}

	return cnt;
}

void evpp_server::OnRecvRequestBlock(uint32_t conn_id, uint16_t type, void* msg, uint16_t sz)
{
	++_cnt_req_block;
	if (_cnt_req_block >= _map_connid.size())
	{
		_cnt_req_block = 0;
		_cb_recv_ctrl(conn_id, type, msg, sz);
	}
}
