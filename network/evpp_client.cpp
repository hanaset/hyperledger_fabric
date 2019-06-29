#include "evpp_client.h"



/////////////////////////////////////////////////////
// pool_nt_lbuf

pool_nt_lbuf::pool_nt_lbuf()
{
	for (int i = 0; i < 3; i++)
	{
		_pool.push_back(std::make_shared<NT_LBUF>());
	}
}
pool_nt_lbuf::~pool_nt_lbuf()
{
	_pool.clear();
}

sptr_nt_lbuf pool_nt_lbuf::get_buf()
{
	sptr_nt_lbuf pbuf;
	std::unique_lock<std::mutex> ul(_mtx, std::defer_lock);

	while (pbuf.get() == nullptr)
	{
		ul.lock();
		if (_pool.empty())		_cond.wait(ul);
		if (!_pool.empty())
		{
			pbuf = _pool.front();
			_pool.pop_front();
		}
	}

	return pbuf;
}

void pool_nt_lbuf::release_buf(sptr_nt_lbuf pbuf)
{
	std::lock_guard<std::mutex> lg(_mtx);
	_pool.push_back(pbuf);
	_cond.notify_all();
}






/////////////////////////////////////////////////////
//
evpp_client::~evpp_client()
{
	_pPool = nullptr;
}

void evpp_client::Connect()
{
    _evpp_client.Connect();
}

void evpp_client::Disconnect()
{
    _evpp_client.Disconnect();
    _tcp_conn.reset();
}

void evpp_client::OnConnection(const evpp::TCPConnPtr& conn)
{
    if (conn->IsConnected())
    {
        _tcp_conn = std::make_shared<evpp_tcpConn>(conn, _id,
            _cb_recv_msg,
            std::bind(&evpp_client::OnControlMsg, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)
            );

        std::cout << "Connected to Server ..." << std::endl;
    }
    else
    {
		_evpp_client.Disconnect();
        _tcp_conn = nullptr;
        std::cout << "DIsconnected to Server ..." << std::endl;
    }
}

void evpp_client::OnMessage(const evpp::TCPConnPtr& conn, evpp::Buffer* msg)
{
    if (_tcp_conn.get() != nullptr)
        _tcp_conn->OnRecvMessage(msg);
}

void evpp_client::OnControlMsg(uint32_t id, uint16_t type, void* msg, uint16_t sz)
{
	if (type == 11)
	{	// start recv block data 
		_plbuf = _pPool->get_buf();
		_plbuf->sz = 0;
		_plbuf->pcur = _plbuf->pbuf;

		uint32_t szblock;
		memcpy(&szblock, msg, 4);
	}
	else if (type == 12)
	{	// middle recv block data
		if (_plbuf.get() != nullptr)
		{
			memcpy(_plbuf->pcur, msg, sz);
			_plbuf->pcur += sz;
			_plbuf->sz += sz;
		}
	}
	else if (type == 13)
	{	// final recv block data

		sptr_nt_lbuf pbuf = _plbuf;
		_plbuf.reset();

		if (pbuf.get() != nullptr && sz > 0)
		{
			memcpy(pbuf->pcur, msg, sz);
			pbuf->sz += sz;
		}

		if (_cb_lmsg != nullptr)	_cb_lmsg(id, type, pbuf->pbuf, pbuf->sz);
		_pPool->release_buf(pbuf);
	}
}

bool evpp_client::SendMessage(void* msg, uint16_t sz)
{
    if (_tcp_conn.get() != nullptr)
        return _tcp_conn->SendMessage(1, msg, sz);

    return false;
}

void evpp_client::SetCallbackLargeMsg(CB_RECV_LMSG cbfunc)
{
	_pPool = pool_nt_lbuf::GetInstance();
	_cb_lmsg = cbfunc;
}

bool evpp_client::RequestBlockData()
{
	_tcp_conn->SendMessage(10, nullptr, 0);	// 10 : request block data
}
