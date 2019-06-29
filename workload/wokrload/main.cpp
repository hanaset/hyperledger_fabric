#include <cstdio>
#include <stdint.h>
#include <stdlib.h>

#include "../../common/net_config.h"
#include "workload.h"
#include "../../common/escc.h"

#define NET_CONFIG_PATH "/home/medium/medium-meetup-poc-2/config/net_deploy.conf.json"
#define Test_PATH "net_deploy.conf.json"
int main(int argc, char* argv[])
{

    net_config* pnet_config = net_config::GetInstance();
    pnet_config->load_config(Test_PATH);

    workload_vt client[4] = {   workload_vt (pnet_config->_clients.find(1)->second.get()->skey, pnet_config->_clients.find(1)->second.get()->pkey, 1),
                                workload_vt (pnet_config->_clients.find(2)->second.get()->skey, pnet_config->_clients.find(2)->second.get()->pkey, 2), 
                                workload_vt (pnet_config->_clients.find(3)->second.get()->skey, pnet_config->_clients.find(3)->second.get()->pkey, 3),
                                workload_vt (pnet_config->_clients.find(4)->second.get()->skey, pnet_config->_clients.find(4)->second.get()->pkey, 4)
                            };


    if (argc != 2) {

        std::vector<SPTR_TRANSACTION> vect_tx;

        std::string file("client1_workload");
        std::string file_mrc("client1_workload_mrc1");
        std::string formatA("_A.dat");
        std::string formatB("_B.dat");

        int num = 0;

        for (int i = 0; i < 4; i++) {

            num = client[i].first_load_workload(file + formatA, vect_tx);
            client[i].second_load_workload(file + formatB, vect_tx, num);

            //client[i].load_workload(file + formatA, vect_tx);
            //client[i].load_workload(file + formatB, vect_tx);
            //client[i].load_workload(file_mrc + formatA, vect_tx);
            //client[i].load_workload(file_mrc + formatB, vect_tx);

            file[6] += 1;
            file_mrc[6] += 1;

        }

    }
    else {

        int num = atoi(argv[1]);

        std::string file("client1_workload");
        std::string file_mrc("client1_workload_mrc1");

        int to, from;
        for (int i = 0; i < 4; i++) {

            to =    (i+1) * 1000000 + 1;
            from =  (i+1) * 10000000 + (i + 1) * 1000000 + 1;

            client[i].create_workload(to, from, num, file);
            client[i].create_workload_mrc20(to, from, num, file_mrc, 0);

            file_mrc[20] += 1;
            client[i].create_workload_mrc20(to, from, num, file_mrc, 1);

            file[6] += 1;       // file client num
            file_mrc[6] += 1;   // mrc file client num
            file_mrc[20] -= 1;  // mrc file mode num

        }
    }

    return 0;
}
