#include "escc.h"
#include "string.h"

escc::escc()
{
    _sign_context = secp256k1_context_create(SECP256K1_CONTEXT_SIGN | SECP256K1_CONTEXT_VERIFY);
}

escc::~escc()
{
    secp256k1_context_destroy(_sign_context);
}

bool escc::sign(unsigned char* msg, const unsigned char* skey, unsigned char* sign)
{
	if (secp256k1_ecdsa_sign(_sign_context, &_sig, msg, skey, NULL, NULL) == 1)
	{
		memcpy(sign, _sig.data, 64);
		return true;
	}

    return false;
}

bool escc::verify(unsigned char* sign, unsigned char* pkey, unsigned char* msg)
{
	memcpy(_sig.data, sign, 64);
    memcpy(_publicKey.data, pkey, 64);
    if (secp256k1_ecdsa_verify(_sign_context, &_sig, msg, &_publicKey) == 1)
        return true;
    else
        return false;
}
