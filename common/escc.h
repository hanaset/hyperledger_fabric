#pragma once

#include <secp256k1.h>

#define KEY_LENGTH      64
#define SIGN_LENGTH     64

class escc
{
public:
	escc();
	virtual ~escc();

    bool sign(unsigned char* msg, const unsigned char* skey, unsigned char* sign);
    bool verify(unsigned char* sign, unsigned char* pkey, unsigned char* msg);

private:
    secp256k1_context *_sign_context;

    secp256k1_ecdsa_signature _sig;
    unsigned char _secretKey[KEY_LENGTH];
    secp256k1_pubkey _publicKey;
};

