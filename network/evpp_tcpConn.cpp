#include "evpp_tcpConn.h"



evpp_tcpConn::evpp_tcpConn(const evpp::TCPConnPtr& conn, uint32_t id, CB_RECV_MSG cb_recv, CB_RECV_CTRL cb_recv_ctrl)
    :_conn(conn), _id(id)
{
    _conn = conn;

    _r_len = 0;
    _cb_recv = cb_recv;
    _cb_recv_ctrl = cb_recv_ctrl;
}

evpp_tcpConn::~evpp_tcpConn()
{
}

bool evpp_tcpConn::SendMessage(uint16_t type, void* msg, uint16_t sz)
{
    if (sz > EVPP_TCP_BUF_SIZE) return false;

    std::lock_guard<std::mutex> lg(_mtx_s);
//    uint16_t type = 1;      // user msg type
    memcpy(_s_buf, &type, 2);
    memcpy(_s_buf+2, &sz, 2);

	if (msg != nullptr)
		memcpy(_s_buf + 4, msg, sz);
	else
		sz = 0;

    _conn->Send(_s_buf, 4+sz);

    return true;
}

void evpp_tcpConn::OnRecvMessage(evpp::Buffer* msg)
{
    std::lock_guard<std::mutex> lg(_mtx_r);
    evpp::Slice s;
    int need_len;
    uint16_t sz_msg;
    uint16_t status;
    while (msg->length() > 0)
    {
        // read msg size
        if (_r_len < EVPP_TCP_HEADE_SIZE)
        {
            need_len = EVPP_TCP_HEADE_SIZE - _r_len;
            s = msg->Next(need_len);
            memcpy(_r_buf + _r_len, s.data(), (int)s.size());
            _r_len += (int)s.size();
        }
        if (_r_len < EVPP_TCP_HEADE_SIZE)	return;

        status = *(uint16_t*)_r_buf; 
        sz_msg = *(uint16_t*)(_r_buf + 2);

        // read msg
        need_len = (EVPP_TCP_HEADE_SIZE + sz_msg) - _r_len;
        if (need_len > 0)
        {
            s = msg->Next(need_len);
            memcpy(_r_buf + _r_len, s.data(), (int)s.size());
            _r_len += (int)s.size();
        }

        if ((uint16_t)_r_len == (EVPP_TCP_HEADE_SIZE + sz_msg))
        {
            if (status == 1)  // user msg
            {
                _cb_recv(_id, _r_buf + EVPP_TCP_HEADE_SIZE, sz_msg);
            }
            else    // network Control msg
            {
                _cb_recv_ctrl(_id, status, _r_buf + EVPP_TCP_HEADE_SIZE, sz_msg);
            }

            _r_len = 0;
        }
    }
}
