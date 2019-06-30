#include <cstdio>
#include <stdint.h>
#include <stdlib.h>

#include "../../common/net_config.h"
#include "client_core.h"

#define NET_CONFIG_PATH ""
int main(int argc, char* argv[])
{
    if (argc < 3)
    {
        printf("We need args [node_id] [mode(0:transfer/1:chaincode)]\n");
        return 0;
    }

    net_config* pnet_config = net_config::GetInstance();
    pnet_config->load_config(NET_CONFIG_PATH);

    uint32_t node_id = atoi(argv[1]);
    auto itr = pnet_config->_clients.find(node_id);
    if (itr == pnet_config->_clients.end())
    {
        printf("We cannot find client-info [node_id : %d]\n", node_id);
        return 0;
    }

    int mode = atoi(argv[2]);

    client_core core(node_id, mode);
    core.Start();


    while (1)
        sleep(10);

    return 0;
}
