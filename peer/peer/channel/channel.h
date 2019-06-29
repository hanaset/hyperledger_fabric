#pragma once

#include "database.h"
#include <map>

#define MEETUP_CH   10

class channel_manager
{
public:
    static channel_manager* GetInstance()
    {
        static channel_manager* _pInstance = new channel_manager;
        return _pInstance;
    }
    virtual ~channel_manager();

    bool create_channel_db(uint8_t ch, std::string dbname);
    idatabase* get_channel_db(uint8_t ch);

private:
    channel_manager();
    std::map<uint8_t, idatabase*> _map_ch_db;
};
