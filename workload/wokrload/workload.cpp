#include "workload.h"
#include "../../common/utils.h"
#include "../../common/escc.h"

#include <iostream>
#include <fstream>
#include <openssl/sha.h>
#include <stdlib.h>
#include <string>

workload_vt::workload_vt(uint8_t* skey, uint8_t* pkey, int id)
    : _id(id)
{
    memcpy(_skey, skey, 32);
    memcpy(_pkey, pkey, 64);
}


workload_vt::~workload_vt()
{
}


void workload_vt::make_test_data(uint8_t ch, uint8_t cc, std::vector<SPTR_TRANSACTION>& vect_tx)
{
    Json::FastWriter j_writer;
    Json::Value root, trans;

    std::string to("aaaaaaaa"), from("bbbbbbbb"), arg;
    vect_tx.resize(400*1000);
    SPTR_TRANSACTION pTx;
    uint16_t sz;
    escc _escc;
    for (int i=0; i< (400*1000); i++)
	{
        pTx = std::make_shared<TRANSACTION>();
        memset(pTx->byte_buf, 0x00, SZ_TRANSACTION);
        pTx->sz_data = SZ_TRANSACTION;

		memset(pTx->creator_sign, 0x00, 64);
		memset(pTx->tx_id, 0x00, 32);
		(*pTx->creator) = 0;

		(*pTx->ch_id) = ch;
		(*pTx->cc_id) = cc;

        trans.append(to);
        trans.append(from);
        trans.append("10");
        root["trans"] = trans;
        arg = j_writer.write(root);
        trans.clear();
        root.clear();

        std::cout << arg << std::endl;
		memcpy(pTx->cc_arg, arg.data(), arg.size());

        SHA256((const uint8_t*)pTx->creator, 4+1+1+arg.size(), pTx->tx_id);
        if (!_escc.sign(pTx->tx_id, _skey, pTx->creator_sign))
        {
            std::cout << "Failed Sign ..." << std::endl;
        }

        sz = 64 + 32 + 4 + 1 + 1 + arg.size();
        memcpy((void*)pTx->sz_proposal, (void*)&sz, 2);
        vect_tx[i] = pTx;
        pTx.reset();

        to.swap(from);
    }
}

#define CCFUNC_TYPE_TOTAL 0
#define CCFUNC_TYPE_TRANS 1
#define CCFUNC_TYPE_BALAN 2
#define CCFUNC_TYPE_ROUND 3

Json::Value workload_vt::get_ccarg_mrc20(int index, int ftype) {
    static int _ftype = CCFUNC_TYPE_TOTAL;
    if (ftype == CCFUNC_TYPE_ROUND) {
        ftype = _ftype;
        _ftype = (_ftype + 1) % 3;
    }

    Json::Value ccarg, farg;
    std::string fname;

    switch (ftype) {
    case CCFUNC_TYPE_TOTAL:
        fname = "total";
        break;

    case CCFUNC_TYPE_TRANS:
        fname = "trans";
        farg.append("kwon" + std::to_string(index));
        farg.append("choi" + std::to_string(index));
        farg.append("10");
        break;

    case CCFUNC_TYPE_BALAN:
        fname = "balan";
        farg.append("choi" + std::to_string(index - 1));
        break;

    default:
        std::cerr << "invalid chaincode function type: " << ftype << std::endl;
        return ccarg;
    }

    ccarg[fname] = farg;
    return ccarg;
}

void workload_vt::make_test_data_mrc20(uint8_t ch, uint8_t cc, std::vector<SPTR_TRANSACTION>& vect_tx, uint32_t n_tx) {
    Json::FastWriter writer;
    vect_tx.resize(n_tx);
    SPTR_TRANSACTION tx;
    escc escc;

    for (int i = 0; i < n_tx; i++) {
        std::string ccargJson;
        tx = std::make_shared<TRANSACTION>();
        
        *tx->creator = 1;
        *tx->ch_id = ch;
        *tx->cc_id = cc;
        ccargJson = writer.write(get_ccarg_mrc20(i + 1, CCFUNC_TYPE_ROUND));
        memcpy(tx->cc_arg, ccargJson.data(), ccargJson.size());
        //std::cout << "cc_arg: " << tx->cc_arg << std::endl;

        SHA256((const uint8_t*)tx->creator, 4 + 1 + 1 + ccargJson.size(), tx->tx_id);
        if (!escc.sign(tx->tx_id, _skey, tx->creator_sign)) {
            std::cerr << "Failed Sign ..." << std::endl;
        }

        *tx->sz_proposal = 64 + 32 + 4 + 1 + 1 + ccargJson.size();

        vect_tx[i] = tx;
        tx.reset();
    }
}

void workload_vt::create_workload_detail(int _to, int _from, int count, FILE* fp, int *amount)
{
    Json::FastWriter j_writer;
    Json::Value root, trans, result;
    Json::Value filecount;
    
    char tx_id[65], sign[129];

    filecount["count"] = count;
    fprintf(fp, "%s", j_writer.write(filecount).c_str());

    std::string arg;
    char to[9], from[9];
    SPTR_TRANSACTION pTx;
    uint16_t sz;
    escc _escc;

    for (int i = 0; i < count; i++)
    {
        pTx = std::make_shared<TRANSACTION>();
        memset(pTx->byte_buf, 0x00, SZ_TRANSACTION);
        pTx->sz_data = SZ_TRANSACTION;

        //to = std::to_string(i + _from);
        //from = std::to_string(i + _to);

        sprintf(to, "%08X", i + _to);
        sprintf(from, "%08X", i + _from);

        memset(pTx->creator_sign, 0x00, 64);
        memset(pTx->tx_id, 0x00, 32);
        (*pTx->creator) = _id;

        (*pTx->ch_id) = 10;
        (*pTx->cc_id) = 9;

        trans.append(to);
        trans.append(from);
        trans.append(std::to_string(amount[i]));
        root["trans"] = trans;
        arg = j_writer.write(root);

        result["ch-id"] = *(pTx->ch_id);
        result["cc-id"] = *(pTx->cc_id);
        result["cc-arg"] = arg.c_str();
        result["creator"] = *(pTx->creator);

        memcpy(pTx->cc_arg, arg.data(), arg.size());

        SHA256((const uint8_t*)pTx->creator, 4 + 1 + 1 + arg.size(), pTx->tx_id);

        if (!_escc.sign(pTx->tx_id, _skey, pTx->creator_sign))
        {
            std::cout << "Failed Sign ..." << std::endl;
            return;
        }

        if (!_escc.verify(pTx->creator_sign, _pkey, pTx->tx_id))
        {
            std::cout << "Failed Verity ..." << std::endl;
            return;
        }

        for (int j = 0, k = 0; j <= KEY_LENGTH; j++, k += 2)
            sprintf(&tx_id[k], "%02X", pTx->tx_id[j]);


        for (int j = 0, k = 0; j <= SIGN_LENGTH; j++, k += 2)
            sprintf(&sign[k], "%02X", pTx->creator_sign[j]);

        result["tx-id"] = tx_id;
        result["sign"] = sign;
        memset(tx_id, 0, 65);
        memset(sign, 0, 129);
        sz = 64 + 32 + 4 + 1 + 1 + arg.size();
        memcpy((void*)pTx->sz_proposal, (void*)&sz, 2);

        fprintf(fp, "%s", j_writer.write(result).c_str());
        pTx.reset();
        trans.clear();
        root.clear();

    }

}

void workload_vt::create_workload(int _to, int _from, int count, std::string PATH) {

    
    FILE *fp;
    int *amount = (int*)malloc(sizeof(int)*count);

    for (int i = 0; i < count; i++) {
        amount[i] = rand() % 10 + 1;
    }

    std::string path1(PATH + "_A.dat");
    std::string path2(PATH + "_B.dat");


    fp = fopen(path1.c_str(), "w");
    create_workload_detail(_to, _from, count, fp, amount);
    fclose(fp);

    fp = fopen(path2.c_str(), "w");
    create_workload_detail(_from, _to, count, fp, amount);
    fclose(fp);
   
    
}

void workload_vt::create_workload_mrc20_detail(int _to, int _from, int count, FILE* fp, int *amount, int mode) {

    Json::FastWriter j_writer;
    Json::Value root, trans, result;
    Json::Value filecount;

    char tx_id[65], sign[129];

    filecount["count"] = count;
    fprintf(fp, "%s", j_writer.write(filecount).c_str());

    std::string arg;
    char to[9], from[9];
    SPTR_TRANSACTION pTx;
    uint16_t sz;
    escc _escc;

    for (int i = 0; i < count; i++)
    {
        pTx = std::make_shared<TRANSACTION>();
        memset(pTx->byte_buf, 0x00, SZ_TRANSACTION);
        pTx->sz_data = SZ_TRANSACTION;

        //to = std::to_string(i + _from);
        //from = std::to_string(i + _to);

        sprintf(to, "%08X", i + _to);
        sprintf(from, "%08X", i + _from);

        memset(pTx->creator_sign, 0x00, 64);
        memset(pTx->tx_id, 0x00, 32);
        (*pTx->creator) = _id;

        (*pTx->ch_id) = 10;
        (*pTx->cc_id) = 11;

        switch (mode) {
        case 0:
            trans.append(to);
            trans.append(from);
            trans.append(std::to_string(amount[i]));
            root["trans"] = trans;
            break;
        case 1:
            trans.append(to);
            root["balan"] = trans;
            break;
        //case 2:
        //    root["total"] = Json::nullValue;
        //    break;
        }

        arg = j_writer.write(root);

        result["ch-id"] = *(pTx->ch_id);
        result["cc-id"] = *(pTx->cc_id);
        result["cc-arg"] = arg;
        result["creator"] = *(pTx->creator);

        memcpy(pTx->cc_arg, arg.data(), arg.size());

        SHA256((const uint8_t*)pTx->creator, 4 + 1 + 1 + arg.size(), pTx->tx_id);
        if (!_escc.sign(pTx->tx_id, _skey, pTx->creator_sign))
        {
            std::cout << "Failed Sign ..." << std::endl;
            return;
        }

        //if (!_escc.verify(pTx->creator_sign, _pkey, pTx->tx_id))
        //{
        //    std::cout << "Failed Verity ..." << std::endl;
        //    return;
        //}

        for (int j = 0, k = 0; j < KEY_LENGTH; j++, k += 2)
            sprintf(&tx_id[k], "%02X", pTx->tx_id[j]);


        for (int j = 0, k = 0; j < SIGN_LENGTH; j++, k += 2)
            sprintf(&sign[k], "%02X", pTx->creator_sign[j]);

        result["tx-id"] = tx_id;
        result["sign"] = sign;
        memset(tx_id, 0, 65);
        memset(sign, 0, 129);
        sz = 64 + 32 + 4 + 1 + 1 + arg.size();
        memcpy((void*)pTx->sz_proposal, (void*)&sz, 2);

        fprintf(fp, "%s", j_writer.write(result).c_str());
        pTx.reset();
        trans.clear();
        root.clear();

    }
}

void workload_vt::create_workload_mrc20(int _to, int _from, int count, std::string PATH, int mode) {

    FILE *fp;
    int *amount = (int*)malloc(sizeof(int)*count);

    for (int i = 0; i < count; i++) {
        amount[i] = rand() % 10 + 1;
    }
    std::string path1(PATH + "_A.dat");
    std::string path2(PATH + "_B.dat");

    fp = fopen(path1.c_str(), "w");
    create_workload_mrc20_detail(_to, _from, count, fp, amount, mode);
    fclose(fp);

    fp = fopen(path2.c_str(), "w");
    create_workload_mrc20_detail(_from, _to, count, fp, amount, mode);
    fclose(fp);
}


int workload_vt::first_load_workload(std::string path, std::vector<SPTR_TRANSACTION>& vect_tx)
{
    std::fstream fin;
    fin.open(path, std::fstream::in);

    if (!fin.is_open()) {
        return false;
    }

    escc _escc;

    Json::Value root;
    Json::Reader jreader;
    std::string tmp;
    SPTR_TRANSACTION pTx;
    uint16_t sz_proposal;

    size_t cnt_tx;
    // read count
    if (std::getline(fin, tmp))
    {
        if (tmp.empty())
        {
            return false;
        }

        jreader.parse(tmp, root);
        cnt_tx = root["count"].asInt64();
        root.clear();
    }

    if (cnt_tx == 0)    return false;
    vect_tx.resize(cnt_tx);
    size_t idx = 0;
    while (std::getline(fin, tmp))
    {
        if (tmp.empty())
        {
            break;
        }

        jreader.parse(tmp, root);
        pTx = std::make_shared<TRANSACTION>();
        memset(pTx->byte_buf, 0x00, pTx->sz_data);

        // sign => binary, memcpy
        hex2bin(root["sign"].asString(), pTx->creator_sign, 64);
        hex2bin(root["tx-id"].asString(), pTx->tx_id, 32);

        (*pTx->creator) = root["creator"].asInt();
        (*pTx->ch_id) = (uint8_t)root["ch-id"].asInt();
        (*pTx->cc_id) = (uint8_t)root["cc-id"].asInt();


        tmp = root["cc-arg"].asString();
        memcpy((void*)pTx->cc_arg, (void*)tmp.data(), tmp.size());

        sz_proposal = 64 + 32 + 4 + 1 + 1 + tmp.size();
        memcpy((void*)pTx->sz_proposal, (void*)&sz_proposal, 2);


        if (!_escc.verify(pTx->creator_sign, _pkey, pTx->tx_id))
        {
            std::cout << "Failed Verity ..." << std::endl;
            return false;
        }

        Json::Value temp;

        if (!jreader.parse(tmp, temp))
            printf("err");

        root.clear();
        vect_tx[idx] = pTx;
        ++idx;
        if (idx >= vect_tx.size())  break;
    }

    return idx;
}

int workload_vt::second_load_workload(std::string path, std::vector<SPTR_TRANSACTION>& vect_tx, int before_idx)
{
    std::fstream fin;
    fin.open(path, std::fstream::in);

    if (!fin.is_open()) {
        return false;
    }

    escc _escc;

    Json::Value root;
    Json::Reader jreader;
    std::string tmp;
    SPTR_TRANSACTION pTx;
    uint16_t sz_proposal;

    size_t cnt_tx;
    // read count
    if (std::getline(fin, tmp))
    {
        if (tmp.empty())
        {
            return false;
        }

        jreader.parse(tmp, root);
        cnt_tx = root["count"].asInt64();
        root.clear();
    }

    if (cnt_tx == 0)    return false;
    vect_tx.resize(cnt_tx + before_idx);
    size_t idx = before_idx;
    while (std::getline(fin, tmp))
    {
        if (tmp.empty())
        {
            break;
        }

        jreader.parse(tmp, root);
        pTx = std::make_shared<TRANSACTION>();
        memset(pTx->byte_buf, 0x00, pTx->sz_data);

        // sign => binary, memcpy
        hex2bin(root["sign"].asString(), pTx->creator_sign, 64);
        hex2bin(root["tx-id"].asString(), pTx->tx_id, 32);

        (*pTx->creator) = root["creator"].asInt();
        (*pTx->ch_id) = (uint8_t)root["ch-id"].asInt();
        (*pTx->cc_id) = (uint8_t)root["cc-id"].asInt();


        tmp = root["cc-arg"].asString();
        memcpy((void*)pTx->cc_arg, (void*)tmp.data(), tmp.size());

        sz_proposal = 64 + 32 + 4 + 1 + 1 + tmp.size();
        memcpy((void*)pTx->sz_proposal, (void*)&sz_proposal, 2);


        if (!_escc.verify(pTx->creator_sign, _pkey, pTx->tx_id))
        {
            std::cout << "Failed Verity ..." << std::endl;
            return false;
        }

        Json::Value temp;

        if (!jreader.parse(tmp, temp))
            printf("err");

        root.clear();
        vect_tx[idx] = pTx;
        ++idx;
        if (idx >= vect_tx.size())  break;
    }

    return idx;
}
