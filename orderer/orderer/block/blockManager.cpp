/*
Copyright Medium Corp. 2019 All Rights Reserved.


creator : HAMA
*/
#include "blockManager.h"

#include <utility>
#include <cstring>
#include <iostream>
#include <algorithm>
#include <vector>
#include <openssl/sha.h>
#include <unistd.h>
#include "../util/StopWatch.h"
#include "../../../common/utils.h"


BlockManager::BlockManager(uint32_t  node_id, uint8_t* secrete)
{
    _node_id = node_id;
    _secret = secrete;
    _blockNum = 10;

    std::memset(_previous_block_hash, 1, 32);
}


BlockManager::~BlockManager()
{
}


Block * BlockManager::createBlock(std::shared_ptr<CollectedTranactions> ct)
{
   //block header size 
   size_t BLOCK_HEADER_SIZE = BLOCK_SIGN +
                                   BLOCK_UID +
                                   BLOCK_NUM +
                                   BLOCK_ORDERER_ID +
                                   BLOCK_DATA_LEN +
                                   BLOCK_TRANS_COUNT +
                                   BLOCK_CUR_TIME +
                                   BLOCK_PRIVIOUS_HASH;

   Block * block = new Block(BLOCK_HEADER_SIZE + ct->_data_len);
   uint8_t* pblock = block->_block;

   // =========================== BLOCK HEADER ===========================//
   
   int HEADER_SIZE_SIGN_HASH = 96;
   int HEADER_SIZE_EXCEPT_SIGN_HASH = 60;

   pblock += HEADER_SIZE_SIGN_HASH;
   //add block num  
   memcpy(pblock, &_blockNum, 8);
   pblock += 8;

   //block creator id (usually orderer id) 
   memcpy(pblock, &_node_id, 4);
   pblock += 4;
 
   //add block data length 
   memcpy(pblock, &ct->_data_len, 4);
   pblock += 4;

   //add block transaction count 
   memcpy(pblock, &ct->_trans_cnt, 4);
   pblock += 4;

   //add block time  
   time_t timestamp = (unsigned)time(NULL);
   memcpy(pblock, &timestamp, 8);
   pblock += 8;
   
   //add previous block hash  
   memcpy(pblock, _previous_block_hash, 32);
   pblock += 32;

   // =========================== BLOCK BODY ===========================//

   //add transactions 
   memcpy(pblock, ct->_transaction, ct->_data_len);
   pblock += ct->_data_len;
   
   // =========================== BLOCK SIGN ===========================//

   //add block uid  
   unsigned char digest[SHA256_DIGEST_LENGTH];
   memset(digest, 1, SHA256_DIGEST_LENGTH);
   SHA256(block->_block + 96, HEADER_SIZE_EXCEPT_SIGN_HASH + ct->_data_len, digest);
   memcpy(block->_block + 64, digest, 32);
   pblock += 32;
 

   //block sign 
   secp256k1_ecdsa_signature signature;
   _escc.sign(&digest[0], _secret, signature.data);
   memcpy(block->_block, &signature, 64);


   // ====================== for next block  ====================//
   // next block number = current block number + 1 
   _blockNum = _blockNum + 1;

   //calc current block hash ( for using next blokc's previous hash) 
   memset(_previous_block_hash, 1, SHA256_DIGEST_LENGTH);
   SHA256(pblock, BLOCK_HEADER_SIZE, _previous_block_hash);

   block->_size =  BLOCK_HEADER_SIZE + ct->_data_len;

   ct->reset();

   return block;
}

bool  BlockManager::test_blockParsing(Block * block) {
   
    uint8_t * pblock = block->_block;

    
    uint64_t  num;
    memcpy(&num, pblock + 96, 8);
    std::cout << num << std::endl;

    uint32_t  orid;
    memcpy(&orid, pblock + 96 + 8, 4);
    std::cout << orid << std::endl;

    uint32_t  dataLen;
    memcpy(&dataLen, pblock + 96 + 8 + 4, 4);
    std::cout << dataLen << std::endl;

    uint32_t  transLen;
    memcpy(&transLen, pblock + 96 + 8 + 4 + 4, 4);
    std::cout << transLen << std::endl;

    time_t  timestamp;
    memcpy(&timestamp, pblock + 96 + 8 + 4 + 4 + 4, 8);
    std::cout << timestamp  << std::endl;

    
    uint8_t   _previous_block_hash[32];

    memcpy(_previous_block_hash, pblock + 96 + 8 + 4 + 4 + 4 + 8, 32);
    std::cout << _previous_block_hash << std::endl;

    //블록 UID  
    unsigned char digest[SHA256_DIGEST_LENGTH];
    memcpy(digest, pblock + 64, 32);


    //블록 서명 검증 
    unsigned char signdata[64];
    memcpy(signdata, pblock, 64);

    std::string pubkeystr = "2EC9CAD448D190D8ED2C9A96D51F83CF4BBAE67BA4F7091DA99A10F2002932E50E9D52A121C05A1780A2011CAAA96664058D99414E7DF24E67658B1304424C18";
    uint8_t pubkey[64];
    hex2bin(pubkeystr, pubkey, 64);


    return _escc.verify(signdata, pubkey, digest);
     
}
