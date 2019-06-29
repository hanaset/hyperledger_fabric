#include "committer_core.h"
#include "../../data_define/path_def.h"
#include "../../common/net_config.h"
#include "../../common/json.h"
#include "channel/channel.h"

#include <iostream>
#include <vector>

#define BLOCK_SIZE (200*1000*600+1024)  // 600byte 20만tx, header, extra

#define __TEST__

#ifdef __TEST__
std::chrono::time_point<std::chrono::system_clock> block_start_time;
std::chrono::time_point<std::chrono::system_clock> block_end_time;

std::chrono::time_point<std::chrono::system_clock> block_start_validate;
std::chrono::time_point<std::chrono::system_clock> block_end_validate;

std::chrono::time_point<std::chrono::system_clock> block_start_time_10_b;
std::chrono::time_point<std::chrono::system_clock> block_end_time_10_b;
static uint32_t idx_10b = 0;
#endif // __TEST__


/////////////////////////////////////////////////////////
//

/////////////////////////////////////////////////////////
//
pool_block::pool_block()
{
}
pool_block::~pool_block()
{
}

#define SZ_POOL_BLOCK   5
void pool_block::init(int max_get)
{
    sptr_block_data pblock;
    _max_get = max_get;

    for (int i = 0; i < SZ_POOL_BLOCK; i++)
    {
        pblock = std::make_shared<BLOCK_DATA>();
        pblock->block.resize(BLOCK_SIZE);
        pblock->szblock = pblock->block.size();
        _pool.push_back(pblock);
    }
}
sptr_block_data pool_block::getBlockBuf()
{
    sptr_block_data pblock;

    std::unique_lock<std::mutex> ul(_mtx_pool);

    while (pblock.get() == nullptr)
    {
        if (_pool.size() <= (SZ_POOL_BLOCK - _max_get))
            _cond_pool.wait(ul);

        if (_pool.size() <= (SZ_POOL_BLOCK - _max_get))
            continue;

        pblock = _pool.front();
        _pool.pop_front();
        ul.unlock();

        pblock->szblock = pblock->block.size();
        break;
    }
    return pblock;
}

void pool_block::releaseBlockBuf(sptr_block_data pblock)
{
    if (pblock.get() != nullptr)
    {
        memset(&pblock->block[0], 0x00, pblock->block.size());
        std::lock_guard<std::mutex> lg(_mtx_pool);
        _pool.push_back(pblock);
        _cond_pool.notify_all();
    }
}



/////////////////////////////////////////////////////////
//
committer_core::committer_core(uint32_t node_id)
{
    _node_id = node_id;
}

committer_core::~committer_core()
{
}

#define SZ_VERI_THREAD  40

void committer_core::StartVerifyThread()
{
    _vect_thrd_verify.resize(SZ_VERI_THREAD);

    for (int i = 0; i < SZ_VERI_THREAD; i++)
    {
        _vect_thrd_verify[i] = std::thread([&](int idx) {thrd_verify(idx); }, i);
    }
}

bool committer_core::Start()
{
    net_config* conf = net_config::GetInstance();

    std::string ipport;
    size_t cnt(0);

    // endorser connecting
    _orderer_mgr = std::make_shared<evpp_client_manager>(_node_id);
    for (auto itr : conf->_orderers)
    {
        ipport = itr.second->listen_bl;
        cnt = _orderer_mgr->ConnectTo(itr.first, ipport, nullptr
            , std::bind(&committer_core::OnRecvBlock, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)
        );

    }
    std::cout << "Connecting orderer Server [count : " << cnt << "]" << std::endl;

    _orderer_mgr->Start();

    _bStop = false;
    StartVerifyThread();
    _thrd_validate = std::thread([&]() { thrd_validate(); });
    _thrd_commite = std::thread([&]() { thrd_commite(); });
    _thrd_req_block = std::thread([&]() {thrd_request_block(); });

    return true;
}

void committer_core::Stop()
{

}

void committer_core::thrd_request_block()
{
    net_config* conf = net_config::GetInstance();

    bool real_req(false);

    auto peer_info = conf->_peers.find(_node_id);
    real_req = peer_info->second->bLeader;

    if (real_req)   _pool_block.init(3);
    else            _pool_block.init(5);

    getchar();
    sptr_block_data pblock;

    std::mutex mtx;
    std::unique_lock<std::mutex> ul(mtx);
    while (!_bStop)
    {
        _orderer_mgr->RequestBlockData();
        _cond_que_block.wait(ul);
    }
}

void committer_core::OnRecvBlock(uint32_t id, uint16_t type, void* pbuf, uint32_t sz)
{
#ifdef __TEST__
    if (idx_10b == 0)   block_start_time_10_b = std::chrono::system_clock::now();
#endif // __TEST __

    sptr_block_data pblock = _pool_block.getBlockBuf();
    pblock->szblock = sz;
    memcpy(&pblock->block[0], pbuf, sz);

    _que_block.push_back(pblock);
    pblock.reset();
    _cond_que_block.notify_all();
}

void committer_core::start_verify()
{
    // start verify
    _ended_thrd_verify.clear();
    _cond_start_verify.notify_all();

    // wait ended verify all
    std::unique_lock<std::mutex> ul(_mtx_ended_thrd_verify);
    while (1)
    {
        _cond_end_verify.wait(ul);
        if (_ended_thrd_verify.size() == SZ_VERI_THREAD)
            break;
    }
}

void committer_core::thrd_commite()
{
    net_config* conf = net_config::GetInstance();
    BLOCK_HEADER block_header;

    uint32_t sz;
    bool bret(true);
    std::mutex mtx_block;
    std::unique_lock<std::mutex> ul(mtx_block, std::defer_lock);
    std::unique_lock<std::mutex> ul_validate(_mtx_que_validate, std::defer_lock);
    sptr_block_data pblock;
    // escc
    escc escc;

    while (!_bStop)
    {
        ul.lock();
        if (_que_block.empty())
            _cond_que_block.wait(ul);

        if (_que_block.empty())
        {
            ul.unlock();
            continue;
        }
        ul.unlock();
#ifdef __TEST__
        block_start_time = std::chrono::system_clock::now();
#endif // __TEST__

        pblock = _que_block.front();

        if (block_header.parse_header(&pblock->block[0], pblock->szblock))
        {
            if (!escc.verify(block_header.block_sign, conf->_orderers.find(*block_header.orderer_id)->second->pkey, block_header.block_hash))
            {
                std::cout << "Failed verify block sign [No : " << *block_header.block_num << "]" << std::endl;
                continue;
            }

            pblock->val_result.resize(*block_header.tx_cnt);
            memset(&pblock->val_result[0], 0x00, pblock->val_result.size());

            // verify
            start_verify();

#ifdef __TEST__
            block_end_time = std::chrono::system_clock::now();
            std::chrono::duration<double> diff = block_end_time - block_start_time;
            std::cout << "End of verify one block [ " << diff.count() << "s ]" << std::endl;
#endif // __TEST__

            // push to validate que
            ul_validate.lock();
            _que_validate.push_back(pblock);
            ul_validate.unlock();
            _cond_que_validate.notify_all();

            pblock.reset();
        }
        else
            std::cout << "Failed parsing block header" << std::endl;

        ul.lock();
        _que_block.pop_front();
        ul.unlock();

        if (pblock.get() != nullptr)
            _pool_block.releaseBlockBuf(pblock);

    }   // while (!_bStop)
}

bool committer_core::thrd_verify(int idx)
{
    net_config* conf = net_config::GetInstance();

    SPTR_TRANSACTION tx = std::make_shared<TRANSACTION>();
    sptr_block_data pblock;
    uint8_t* pos_tx;
    uint32_t sz;
    uint32_t idx_result;


    bool bret(true);
    bool bcritical(false);

    // escc
    escc escc;

    std::mutex mtx;
    std::unique_lock<std::mutex> ul(mtx);
    while (!_bStop)
    {
        // wait start signal
        _cond_start_verify.wait(ul);

        // start work
        pblock = _que_block.front();
        pos_tx = &pblock->block[0];
        sz = pblock->szblock;
        pos_tx += SZ_BLOCK_HEADER;
        sz -= SZ_BLOCK_HEADER;

        pos_tx += (idx * SZ_TRANSACTION);
        sz -= (idx * SZ_TRANSACTION);

        idx_result = idx;
        while (sz > 0)
        {
            bret = false;
            if (tx->parse_tx(pos_tx, sz))
            {
                if (escc.verify(tx->creator_sign, conf->_clients.find(*tx->creator)->second->pkey, tx->tx_id)
                    && escc.verify(tx->endorsor_1_sign, conf->_peers.find(*tx->endorsor_1_id)->second->pkey, tx->response_hash)
                    && escc.verify(tx->endorsor_2_sign, conf->_peers.find(*tx->endorsor_2_id)->second->pkey, tx->response_hash)
                    && escc.verify(tx->endorsor_3_sign, conf->_peers.find(*tx->endorsor_3_id)->second->pkey, tx->response_hash)
                )
                {
                    bret = true;
                }
                else 
                    std::cout << "Failed verify sign in tx" << std::endl;
            }
            else
            {
                bcritical = true;
                std::cout << "Failed parsing tx in block" << std::endl;
                break;  // critical error
            }

            pblock->val_result[idx_result] = (uint8_t)bret;

            if (sz < (SZ_VERI_THREAD * SZ_TRANSACTION))
                sz = 0;
            else
            {
                pos_tx += (SZ_VERI_THREAD * SZ_TRANSACTION);
                sz -= (SZ_VERI_THREAD * SZ_TRANSACTION);
                idx_result += SZ_VERI_THREAD;
            }
        }   // while (sz > 0)

        std::unique_lock<std::mutex> ul(_mtx_ended_thrd_verify);
        _ended_thrd_verify.push_back(idx);
        ul.unlock();
        _cond_end_verify.notify_all();
    }

    return bcritical;
}


void committer_core::thrd_validate()
{
    BLOCK_HEADER block_header;
    SPTR_TRANSACTION tx = std::make_shared<TRANSACTION>();
    sptr_block_data pblock;
    uint8_t* pos_tx;
    uint32_t sz;

    bool bret;
    Json::Value root, item;
    Json::Reader j_reader;
    std::string key, value, get_value;
    idatabase* pdb = channel_manager::GetInstance()->get_channel_db(MEETUP_CH);
    FILE* pf;
    std::string block_file;

    std::unique_lock<std::mutex> ul(_mtx_que_validate, std::defer_lock);
    while (!_bStop)
    {
        ul.lock();
        if (_que_validate.empty())
            _cond_que_validate.wait(ul);

        if (_que_validate.empty())
        {
            ul.unlock();
            continue;
        }

        pblock = _que_validate.front();
        _que_validate.pop_front();
        ul.unlock();

#ifdef __TEST__
        block_start_validate = std::chrono::system_clock::now();
#endif // __TEST__

        pos_tx = &pblock->block[0];
        sz = pblock->szblock;

        if (block_header.parse_header(&pblock->block[0], pblock->szblock))
        {
            pos_tx += SZ_BLOCK_HEADER;
            sz -= SZ_BLOCK_HEADER;

            for (int i = 0; i < pblock->val_result.size(); i++)
            {
                bret = false;
                if (pblock->val_result[i] == 1 && (tx->parse_tx(pos_tx, sz)))
                {
                    j_reader.parse(tx->response, root);
                    if (j_reader.good())
                    {
                        bret = true;
                        item = root["r"];
                        if (item.isArray())
                        {
                            for (int idx = 0; bret && (idx < (int)item.size()); idx++)
                            {
                                key = item[idx]["k"].asString();
                                value = item[idx]["v"].asString();
                                if ((!pdb->getState("", key, get_value)) || (get_value.compare(value) != 0))
                                {
#ifdef __TEST__
                                    bret = true;
#else
                                    bret = false;
#endif
                                }
                            }
                        }

                        item.clear();
                        item = root["w"];
                        if (bret && item.isArray())
                        {
                            for (int idx = 0; bret && (idx < (int)item.size()); idx++)
                            {
                                key = item[idx]["k"].asString();
                                value = item[idx]["v"].asString();
                                pdb->putState("", key, value, false);
                            }
                        }
                    }
                    else
                        std::cout << "Failed parsing json of result" << std::endl;

                    if (!bret)
                    {
                        pblock->val_result[i] = 0;
                        std::cout << "Failed validating tx[" << i << "]" << std::endl;
                    }
                }
                else
                    std::cout << "Failed val_result or parsing tx in block [" << i << "]" << std::endl;

                pos_tx += SZ_TRANSACTION;
            }
            pdb->putState("", "", "", true);

#ifdef __TEST__
            block_end_validate = std::chrono::system_clock::now();
            std::chrono::duration<double> diff = block_end_validate - block_start_validate;
            std::cout << "End of validating one block [ " << diff.count() << "s ]" << std::endl;

            block_start_validate = std::chrono::system_clock::now();
#endif // __TEST __

            block_file = PATH_LEDGER;
            block_file += std::to_string(MEETUP_CH) + "/B" + std::to_string(*block_header.block_num);
            if ((pf = std::fopen(block_file.c_str(), "wb")) != nullptr)
            {
                // write block data in a file
                std::fwrite(&pblock->block[0], sizeof(uint8_t), pblock->szblock, pf);

                // write validation result in the file
                std::fwrite(&pblock->val_result[0], sizeof(uint8_t), pblock->val_result.size(), pf);
                std::fclose(pf);

#ifdef __TEST__
                block_end_validate = std::chrono::system_clock::now();
                std::chrono::duration<double> diff = block_end_validate - block_start_validate;
                std::cout << "End of writting file with one block [ " << diff.count() << "s ]" << std::endl;
#endif // __TEST __

#ifdef __TEST__
                ++idx_10b;
                if ((idx_10b % 10) == 0)
                {
                    block_end_time_10_b = std::chrono::system_clock::now();
                    std::chrono::duration<double> diff = block_end_time_10_b - block_start_time_10_b;
                    std::cout << "End of 10 blocks [ " << idx_10b << " : " << diff.count() << "s ]" << std::endl;
                }
#endif // __TEST __
            }
            else
                std::cout << "Failed writting block file [" << block_file << std::endl;
        }
        else
            std::cout << "Failed parsing block header in thrd_validate" << std::endl;

        _pool_block.releaseBlockBuf(pblock);
    }

}
