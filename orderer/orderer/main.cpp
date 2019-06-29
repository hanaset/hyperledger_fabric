#include <cstdio>

#include "../../data_define/path_def.h"
#include "../../common/net_config.h"
#include "./core/orderer_core.h"
#include "./config/orderer_config.h"

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        printf("We need args\n");
        return 0;
    }

    //NETWORK CONFIG
    net_config* pnet_config = net_config::GetInstance();
    pnet_config->load_config(PATH_NET_CONFIG);

    uint32_t node_id = atoi(argv[1]);
    auto itr = pnet_config->_orderers.find(node_id);
    if (itr == pnet_config->_orderers.end())
    {
        printf("We cannot find orderer-info [node_id : %d]\n", node_id);
        return 0;
    }

    uint8_t* secretKey = itr->second->skey;
    auto txPort = itr->second->listen_tx;
    auto blPort = itr->second->listen_bl;

    //ORDERER CONFIG
    std::shared_ptr<Orderer_config> orderer_config = std::make_shared<Orderer_config>();
    orderer_config->load_config(PATH_ORDERER_CONFIG);

    orderer_core core(node_id, secretKey, txPort, blPort, orderer_config);
    core.start();


    // ADD SIGNAL EXIT
    while (1)
    {
        sleep(100);
    }

    return 0;
}
