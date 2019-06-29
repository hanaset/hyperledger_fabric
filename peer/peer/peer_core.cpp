#include "peer_core.h"
#include "channel/channel.h"

#include "../../common/net_config.h"
#include "../../common/escc.h"

#include <openssl/sha.h>


peer_core::peer_core(const uint32_t node_id)
    : _node_id(node_id), _endorser_core(node_id), _committer_core(node_id)
{
}

peer_core::~peer_core()
{
}

bool peer_core::Start()
{
    net_config* pnet_config = net_config::GetInstance();

    // channel db
    std::string dbname("wsdb_");
    dbname += std::to_string(MEETUP_CH);
    if (!channel_manager::GetInstance()->create_channel_db(MEETUP_CH, dbname))
    {
        printf("We cannot open channel db [%d : %s]\n", MEETUP_CH, dbname.c_str());
        return false;
    }

    // endorser service
    auto itr = pnet_config->_peers.find(_node_id);
    if (itr == pnet_config->_peers.end())
    {
        return false;
    }

    std::string ipport;
    ipport += itr->second->endorser_listen;
    std::string ipport_cc = "127.0.0.1:";
    ipport_cc += itr->second->chaincode_listen;
    _endorser_core.Start(ipport, ipport_cc, itr->second->e_thread);


    // committer service
    _committer_core.Start();

    return true;
}

void peer_core::Stop()
{
}

