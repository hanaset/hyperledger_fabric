#include <iostream>
#include <string>
#include "channel/channel.h"

#define DEFAULT_TXCOUNT 5000000
#define CHANNEL_ID      10

int main(int argc, const char** argv) {
	std::cout << "database initialization start" << std::endl;

	if (!channel_manager::GetInstance()->create_channel_db(CHANNEL_ID, "wsdb_" + std::to_string(CHANNEL_ID))) {
		std::cerr << "error: database creation failure" << std::endl;
		return 1;
	}

	int to, from;
	char cto[9], cfrom[9];
	int nTx = DEFAULT_TXCOUNT;
	idatabase* db = channel_manager::GetInstance()->get_channel_db(CHANNEL_ID);
	for (int j = 0; j < 4; j++) {
		for (int i = 0; i < nTx; i++) {

			to =	(j + 1) * 1000000 + 1;
			from =	(j + 1) * 10000000 + (j + 1) * 1000000 + 1;

			sprintf(cto, "%08X", i + to);
			sprintf(cfrom, "%08X", i + from);
			db->putState("", cto, "500");
			db->putState("", cfrom, "500");
		}
	}

	db->putState("", "total-medium-token", std::to_string(500 * 2 * nTx));

	std::cout << "database initialization end" << std::endl;
	return 0;
}
