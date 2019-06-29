#pragma once

#include <string>

void hex2bin(const std::string& hex, uint8_t* dest, size_t sz_dest);
bool compare_bytes(uint8_t* h1, uint8_t* h2, int sz);
