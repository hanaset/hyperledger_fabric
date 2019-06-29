#pragma once

#include <mutex>
#include <functional>
#include <evpp/tcp_conn.h>

typedef std::function<void(uint32_t, void*, uint16_t)>	CB_RECV_MSG;
typedef std::function<void(uint32_t, uint16_t, void*, uint16_t)>	CB_RECV_CTRL;
#define EVPP_TCP_BUF_SIZE   4096
#define EVPP_TCP_HEADE_SIZE (2+2)

class evpp_tcpConn
{
public:
    evpp_tcpConn(const evpp::TCPConnPtr& conn, uint32_t id, CB_RECV_MSG cb_recv, CB_RECV_CTRL cb_recv_ctrl);
	virtual ~evpp_tcpConn();

    // send
    bool SendMessage(uint16_t type, void* msg, uint16_t sz);
    void OnRecvMessage(evpp::Buffer* msg);

	uint32_t GetID() { return _id; }

private:
    uint32_t _id;
    evpp::TCPConnPtr _conn;

    CB_RECV_MSG _cb_recv;
    CB_RECV_CTRL _cb_recv_ctrl;

    // read msg
    std::mutex _mtx_r;
    int _r_len;
    uint8_t _r_buf[EVPP_TCP_HEADE_SIZE + EVPP_TCP_BUF_SIZE];

    // send msg
    std::mutex _mtx_s;
    uint8_t _s_buf[EVPP_TCP_HEADE_SIZE + EVPP_TCP_BUF_SIZE];
};
typedef std::shared_ptr<evpp_tcpConn> sptr_evpp_tcpconn;
