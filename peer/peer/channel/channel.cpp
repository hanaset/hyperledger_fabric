#include "channel.h"
#include "../../../data_define/path_def.h"

channel_manager::channel_manager()
{
}

channel_manager::~channel_manager()
{
    _map_ch_db.clear();
}

bool channel_manager::create_channel_db(uint8_t ch, std::string dbname)
{
    std::string path(PATH_CH);
    path += std::to_string(ch) + "/" + dbname;

    LevelDB* db = new LevelDB;
    if (db->connect("", path))
    {
        _map_ch_db.insert(std::make_pair(ch, db));
        return true;
    }

    if (db != nullptr)      delete db;
    return false;
}

idatabase* channel_manager::get_channel_db(uint8_t ch)
{
    auto itr = _map_ch_db.find(ch);

    if (itr != _map_ch_db.end())
    {
        return itr->second;
    }

    return nullptr;
}
