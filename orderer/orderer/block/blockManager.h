#pragma once

#include <thread>
#include "block.h"
#include <cstdint>
#include "../../../common/utils.h"
#include "../../../common/net_config.h"
#include "../../../common/escc.h""

#include "../transaction/collectedTXPooll.h"
#include "../transaction/transactionCollector.h"


class BlockManager
{
public:
    BlockManager(uint32_t  node_id, uint8_t* secrete);
    virtual ~BlockManager();

    Block* createBlock(std::shared_ptr<CollectedTranactions> ct);

private:

    uint32_t    _node_id;
    uint8_t*    _secret;

    static const uint32_t BLOCK_SIGN = 64;
    static const uint32_t BLOCK_UID = 32;
    static const uint32_t BLOCK_NUM = 8;
    static const uint32_t BLOCK_ORDERER_ID = 4;
    static const uint32_t BLOCK_DATA_LEN = 4;
    static const uint32_t BLOCK_TRANS_COUNT = 4;
    static const uint32_t BLOCK_CUR_TIME = 8;
    static const uint32_t BLOCK_PRIVIOUS_HASH = 32;

    uint64_t  _blockNum;
    uint8_t   _previous_block_hash[32]; // uint8_t == unsigned char 

    escc      _escc;

public:
    ////////////// test //////////
    bool test_blockParsing(Block * block);
};
