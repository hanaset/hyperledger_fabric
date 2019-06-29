#pragma once

#include <string>
#include <vector>
#include "../../data_define/data_def.h"
#include "../../common/json.h"

class workload_vt
{
public:
	workload_vt(uint8_t*  skey, uint8_t*  pkey, int id);
	virtual ~workload_vt();

    uint8_t  _skey[32];
    uint8_t  _pkey[64];
    int _id;

    int first_load_workload(std::string path, std::vector<SPTR_TRANSACTION>& vect_tx);
    int second_load_workload(std::string path, std::vector<SPTR_TRANSACTION>& vect_tx, int before_idx);
	void make_test_data(uint8_t ch, uint8_t cc, std::vector<SPTR_TRANSACTION>& vect_tx);
    void make_test_data_mrc20(uint8_t ch, uint8_t cc, std::vector<SPTR_TRANSACTION>& vect_tx, uint32_t n_tx);

    /*WorkLoad File Create*/

    void create_workload(int _to, int _from, int count, std::string PATH);
    void create_workload_detail(int _to, int _from, int count, FILE* fp, int* amount);
    void create_workload_mrc20(int _to, int _from, int count, std::string PATH);
    void create_workload_mrc20_detail(int _to, int _from, int count, FILE* fp, int* amount);

private:
    Json::Value get_ccarg_mrc20(int index, int ftype);
};

