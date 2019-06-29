#pragma once

#include <vector>
#include <cstdint>

class Block
{
public:
	Block(uint32_t blocklen);
	~Block();

    uint8_t* _block;
    uint32_t _size;

    //std::vector<uint8_t> _vecBlock;

};

