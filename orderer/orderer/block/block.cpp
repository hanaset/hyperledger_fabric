/*
Copyright Medium Corp. 2019 All Rights Reserved.


creator : HAMA
*/


#include <iostream>
#include "stdlib.h"
#include "block.h"

Block::Block(uint32_t blocklen)
{
    _block = (uint8_t*)malloc(blocklen); 
}


Block::~Block()
{
    if (_block != nullptr) {
        free(_block);
    }
   
}
