#ifndef MODP_AES_H
#define MODP_AES_H

#include <sqstd/sqinc.h>
#include <sqsafe/sqsafemodel.h>

namespace snqu { namespace safe {
	void AES128_CBC_encrypt_buffer(
		const string& plaint, 
		char *key,
		char *iv,
		string &cipher);

	void AES128_CBC_decrypt_buffer(
		const string& cipher, 
		char *key,
		char *iv,
		string &plaint);

	int PHP_AES128_CBC_encrypt(const string& plaint, 
		char *key,
		char *iv,
		string &cipher);

    int PHP_AES128_CBC_Stream_encrypt(const string& plaint, 
        char *key,
        char *iv,
        string &cipher);

	void PHP_AES128_CBC_decrypt(const string& cipher, 
		char *key,
		char *iv,
		string &plaint);

	int CPHP_AES128_CBC_decrypt(const char *cipher,
		int cipher_size,
		char *key,
		char *iv,
		char *plaint,
		int *plaint_size);
}}
#endif // MODP_AES_H