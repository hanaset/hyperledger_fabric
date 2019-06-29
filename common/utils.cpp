#include "utils.h"

void hex2bin(const std::string& hex, uint8_t* dest, size_t sz_dest) 
{
	for (unsigned int i = 0; i < hex.length(); i += 2) {
		std::string byteString = hex.substr(i, 2);
		(*dest) = (char)strtol(byteString.c_str(), NULL, 16);
		dest += 1;
		--sz_dest;
	}
}

bool compare_bytes(uint8_t* h1, uint8_t* h2, int sz)
{
	bool bret(true);
	for (int i = 0; i < sz; i++)
	{
		if (h1[i] != h2[i])
		{
			bret = false;
			break;
		}
	}
	return bret;
}