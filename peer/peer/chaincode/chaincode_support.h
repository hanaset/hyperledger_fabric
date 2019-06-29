#pragma once

#include <memory>
#include <vector>
#include <thread>
#include <functional>

#include "../channel/channel.h"
#include "../../../data_define/data_def.h"
#include "../../../chaincode/chaincode_support/chaincode_support/chaincode_support_service.h"

class vtcc
{
public:
    vtcc() {}
    ~vtcc() {}

    uint8_t simulation(int thrdId, uint8_t channelId, uint8_t ccid, const std::string& ccarg, std::string& result);
};
typedef std::shared_ptr<vtcc>   sptr_vtcc;

class chaincode_support
{
public:
    chaincode_support();
    virtual ~chaincode_support();

    bool Start(const std::string& addr);
    void Stop();

    uint8_t Simulation(int thrdId, uint8_t channelId, uint8_t ccid, const std::string& ccarg, std::string& result);

private:
    vtcc _vtcc;
    ChaincodeSupportService* _service;
    ChaincodeSupportServer* _server;
};
typedef std::shared_ptr<chaincode_support>   sptr_cc_support;

