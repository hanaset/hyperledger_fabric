#include <cstdio>
#include <stdlib.h>

#include "../../data_define/path_def.h"
#include "../../common/net_config.h"

#include "peer_core.h"

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        printf("We need args\n");
        return 0;
    }

    net_config* pnet_config = net_config::GetInstance();
    pnet_config->load_config(PATH_NET_CONFIG);

    uint32_t node_id = atoi(argv[1]);
    auto itr = pnet_config->_peers.find(node_id);
    if (itr == pnet_config->_peers.end())
    {
        printf("We cannot find peer-info [node_id : %d]\n", node_id);
        return 0;
    }

    peer_core core(node_id);
    core.Start();

    while (1)
    {
        sleep(10);
    }
    return 0;
}
