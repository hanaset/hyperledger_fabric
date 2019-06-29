#include "config-util.h"

#include <iostream>
#include <fstream>

namespace config
{
	bool json_conf::loadConfig(const std::string strPath)
	{
		bool bret(false);
		std::ifstream config_file(strPath.data(), std::ifstream::binary);

		if ((bret = config_file.is_open()))
		{
			config_file >> *this;
		}
		return bret;
	}
}
