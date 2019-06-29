#pragma once

#include <evpp/tcp_server.h>

#include "evpp_tcpConn.h"


class evpp_server
{
public:
	evpp_server(std::string svr_name, uint32_t my_id, std::string listen_addr
		, CB_RECV_MSG cb_recv_msg, CB_RECV_CTRL cb_recv_ctrl);
	virtual ~evpp_server();

    bool Start();
    void Stop();

	bool SendMessage(uint32_t conn_id, uint16_t type, uint8_t* msg, uint16_t sz);
	int Broadcast_Message(uint16_t type, void* msg, uint16_t sz);

private:
	void OnMessage(const evpp::TCPConnPtr& conn, evpp::Buffer* msg);
    void OnConnection(const evpp::TCPConnPtr& conn);
	void OnRecvRequestBlock(uint32_t conn_id, uint16_t type, void* msg, uint16_t sz);

//    void OnControlMsg(const evpp::TCPConnPtr& conn, uint16_t type, void* msg, uint16_t sz);

private:
    std::string _svr_name;
    uint32_t _my_id;
    std::string _listen_ipport;
    CB_RECV_MSG _cb_recv_msg;

    // evpp
    evpp::EventLoopThread _loop;
    evpp::TCPServer _tcp_svr;

    // conn map
	std::mutex _mtx_conn_id;
	uint16_t _cur_conn_id;

    std::mutex _mtx_map_connect;
    std::map<evpp::TCPConnPtr, sptr_evpp_tcpconn> _map_conn;
    std::map<uint32_t, sptr_evpp_tcpconn> _map_connid;

	// for orderer
	CB_RECV_CTRL _cb_recv_ctrl;

	// request block
	int _cnt_req_block;
};

typedef std::shared_ptr<evpp_server>	sptr_evpp_server;
