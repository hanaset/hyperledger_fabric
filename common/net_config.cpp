#include "net_config.h"
#include "utils.h"
#include "config-util.h"


net_config::net_config()
{
}


net_config::~net_config()
{
}

bool net_config::load_config(std::string path)
{
    config::json_conf conf;
    conf.loadConfig(path);

    std::string key;
    for (auto item : conf["clients"])
    {
        sptr_client_info_net info = std::make_shared<CLIENT_INFO_NET>();

        info->id = item["id"].asInt();
        info->ip = item["ip"].asString();

		key = item["skey"].asString();
		memset(info->skey, 0x00, 32);
		hex2bin(key, info->skey, 32);

        key = item["pkey"].asString();
        memset(info->pkey, 0x00, 64);
        hex2bin(key, info->pkey, 64);

        _clients.insert(std::make_pair(info->id, info));
    }

    for (auto item : conf["peers"])
    {
        sptr_peer_info_net info = std::make_shared<PEER_INFO_NET>();
        info->id = item["id"].asInt();
        info->ip = item["ip"].asString();
        info->admin_listen = item["admin_listen"].asString();
        info->endorser_listen = item["endorsor_listen"].asString();
        info->chaincode_listen = item["chaincode_listen"].asString();
		info->bLeader = item["leader"].asBool();
		info->e_thread = item["e-thread"].asInt();

		key = item["skey"].asString();
		memset(info->skey, 0x00, 32);
		hex2bin(key, info->skey, 32);

        key = item["pkey"].asString();
        memset(info->pkey, 0x00, 64);
        hex2bin(key, info->pkey, 64);

        _peers.insert(std::make_pair(info->id, info));
    }

    for (auto item : conf["orderers"])
    {
        sptr_orderer_info_net info = std::make_shared<ORDERER_INFO_NET>();
        info->id = item["id"].asInt();
        info->ip = item["ip"].asString();
		info->listen_tx = item["listen_tx"].asString();
		info->listen_bl = item["listen_bl"].asString();

		key = item["skey"].asString();
		memset(info->skey, 0x00, 32);
		hex2bin(key, info->skey, 32);

        key = item["pkey"].asString();
        memset(info->pkey, 0x00, 64);
        hex2bin(key, info->pkey, 64);

        _orderers.insert(std::make_pair(info->id, info));
    }

    return true;
}
