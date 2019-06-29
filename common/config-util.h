#pragma once

#include "json.h"

namespace config
{
	class json_conf : public Json::Value
	{
	public:
		json_conf() {}
		~json_conf() {}
		bool loadConfig(const std::string strPath);

	private:
	};
}
