#ifndef SQCRYPTHELPER_H
#define SQCRYPTHELPER_H

#include <sqstd/sqinc.h>
#include <memory>


namespace snqu{ namespace net2{ 
class SQEncryptHelper
{
public:
	SQEncryptHelper();
	virtual ~SQEncryptHelper();

	static void generate_random_key(string& keyout, uint32 keysize);
	static void get_simple_encrypt_key(string& keyout);
	static void simple_encrypt_key(string& data, uint32 size);
	static void simple_encrypt(string& data, const string& key, uint32 keysize);
	static void simple_encrypt(char *data, uint32 size, const string& key, uint32 keysize);

	static uint16_t checksum(const char *buf, unsigned int  size);
};
}}
#endif // SQCRYPTHELPER_H