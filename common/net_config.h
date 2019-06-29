#pragma once

#include <string>
#include <map>
#include <memory>

struct CLIENT_INFO_NET
{
    int id;
    std::string ip;
	uint8_t skey[32];
    uint8_t pkey[64];
};
typedef std::shared_ptr<CLIENT_INFO_NET> sptr_client_info_net;

struct PEER_INFO_NET
{
    int id;
    std::string ip;
    std::string admin_listen;
    std::string endorser_listen;
    std::string chaincode_listen;
	bool bLeader;
	uint32_t e_thread;
	uint8_t skey[32];
	uint8_t pkey[64];
};
typedef std::shared_ptr<PEER_INFO_NET> sptr_peer_info_net;

struct ORDERER_INFO_NET
{
    int id;
    std::string ip;
	std::string listen_tx;
	std::string listen_bl;
	uint8_t skey[32];
	uint8_t pkey[64];
};

typedef std::shared_ptr<ORDERER_INFO_NET> sptr_orderer_info_net;

class net_config
{
public:
	static net_config* GetInstance()
	{
		static net_config* _pInstance_ = new net_config;
		return _pInstance_;
	}
	virtual ~net_config();

    bool load_config(std::string path);

private:
	net_config();

public:
    std::map<int, sptr_client_info_net> _clients;
    std::map<int, sptr_peer_info_net> _peers;
    std::map<int, sptr_orderer_info_net> _orderers;
};

