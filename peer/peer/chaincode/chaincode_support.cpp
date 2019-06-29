#include "chaincode_support.h"
#include "../channel/channel.h"
#include "../../../common/json.h"
#include "../../../data_define/data_def.h"
#include "../../../common/net_config.h"

uint8_t vtcc::simulation(int thrdId, uint8_t channelId, uint8_t ccid, const std::string& ccarg, std::string& result)
{
    Json::Reader reader;
    Json::FastWriter writer;
    Json::Value j_root, j_item;
    std::string to, from, balance;
    int amount, nb_to, nb_from;

    uint8_t status = RES_STATUS_ERROR;
    if (!ccarg.empty())
    {
        idatabase* pdb = channel_manager::GetInstance()->get_channel_db(channelId);
        if (pdb != nullptr)
        {
            j_root.clear();
            j_item.clear();

            reader.parse(ccarg, j_root);
            if (j_root["trans"].isArray())
            {
                to = j_root["trans"][0].asString();
                from = j_root["trans"][1].asString();
                amount = atoi(j_root["trans"][2].asString().c_str());
                j_root.clear();

                // get value from db
                if (!pdb->getState("", to, balance))
                {
                    balance = "500";
                    pdb->putState("", to, "500", true);
                }
                nb_to = atoi(balance.c_str());

                if (!pdb->getState("", from, balance))
                {
                    balance = "500";
                    pdb->putState("", from, "500", true);
                }
                nb_from = atoi(balance.c_str());

                // simulateion
                if (nb_from >= amount)
                {
                    j_item["k"] = to;
                    j_item["v"] = std::to_string(nb_to);
                    j_root["r"].append(j_item);
                    j_item.clear();

                    j_item["k"] = from;
                    j_item["v"] = std::to_string(nb_from);
                    j_root["r"].append(j_item);
                    j_item.clear();

                    j_item["k"] = to;
                    j_item["v"] = std::to_string(nb_to + amount);
                    j_root["w"].append(j_item);
                    j_item.clear();

                    j_item["k"] = from;
                    j_item["v"] = std::to_string(nb_from - amount);
                    j_root["w"].append(j_item);
                    j_item.clear();

                    result = writer.write(j_root);
                    status = RES_STATUS_UPDATE;
                }//simul
            }// trans
        }// db
    }// ccarg

    return status;
}


/////////////////////////////////////////////////////////////

chaincode_support::chaincode_support()
{
}

chaincode_support::~chaincode_support()
{
}

bool chaincode_support::Start(const std::string& addr)
{
    _service = ChaincodeSupportService::getInstance();
    _server = new ChaincodeSupportServer(_service);
    _server->start(addr);
    return true;
}

void chaincode_support::Stop()
{
}

uint8_t chaincode_support::Simulation(int thrdId, uint8_t channelId, uint8_t ccid, const std::string& ccarg, std::string& result)
{
    if (ccid == 9)
    {
        return _vtcc.simulation(thrdId, channelId, ccid, ccarg, result);
    }
    else
    {
        return _service->simulation(thrdId, channelId, ccid, ccarg, result);
    }
}
