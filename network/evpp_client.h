#pragma once
#include <string>

#include <evpp/tcp_client.h>
#include <evpp/tcp_conn.h>
#include <evpp/buffer.h>

#include "evpp_tcpConn.h"
#include "condition_variable"

#include <list>

typedef std::function<void(uint32_t, uint16_t, void*, uint32_t)>	CB_RECV_LMSG;
#define SZ_LARGE_BUF	(200*1000*600+1024)  // 600byte 20¸¸tx, header, extra

struct NT_LBUF
{
	uint32_t sz;
	uint8_t* pcur;
	uint8_t pbuf[SZ_LARGE_BUF];
};
typedef std::shared_ptr<NT_LBUF>	sptr_nt_lbuf;

class pool_nt_lbuf
{
public:
	static pool_nt_lbuf* GetInstance()
	{
		static pool_nt_lbuf* _pinstance = new pool_nt_lbuf;
		return _pinstance;
	}
	~pool_nt_lbuf();

	sptr_nt_lbuf get_buf();
	void release_buf(sptr_nt_lbuf pbuf);

private:
	pool_nt_lbuf();
	std::mutex _mtx;
	std::condition_variable _cond;
	std::list<sptr_nt_lbuf> _pool;
};

class evpp_client
{
public:
    evpp_client(uint32_t my_id, uint32_t remote_id, std::string remote_addr,
        evpp::EventLoop* loop, CB_RECV_MSG cb_recv_msg)
        : _evpp_client(loop, remote_addr, "TCPClient")
    {
        _id = my_id;
        _remote_id = remote_id;
        _remote_addr = remote_addr;

        _tcp_conn = nullptr;
        _cb_recv_msg = cb_recv_msg;

        _evpp_client.SetMessageCallback(std::bind(&evpp_client::OnMessage, this, std::placeholders::_1, std::placeholders::_2));
        _evpp_client.SetConnectionCallback(std::bind(&evpp_client::OnConnection, this, std::placeholders::_1));
    }
	virtual ~evpp_client();

    void Connect();
    void Disconnect();
	bool SendMessage(void* msg, uint16_t sz);

	// for orderer
	void SetCallbackLargeMsg(CB_RECV_LMSG cbfunc);
	bool RequestBlockData();

private:
    void OnConnection(const evpp::TCPConnPtr& conn);
    void OnMessage(const evpp::TCPConnPtr& conn, evpp::Buffer* msg);
    void OnControlMsg(uint32_t id, uint16_t type, void* msg, uint16_t sz);

private:
    uint32_t _id;
    uint32_t _remote_id;
    std::string _remote_addr;

    // evpp
    evpp::TCPClient _evpp_client;
    sptr_evpp_tcpconn _tcp_conn;
    CB_RECV_MSG _cb_recv_msg;

	// large msg
	CB_RECV_LMSG _cb_lmsg;
	pool_nt_lbuf* _pPool;
	sptr_nt_lbuf _plbuf;
};

typedef std::shared_ptr<evpp_client> sptr_evpp_client;
