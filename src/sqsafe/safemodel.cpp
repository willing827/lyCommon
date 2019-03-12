#include "stdafx.h"
#include "safemodel.h"
#include <codec/detail/modp_b64.h>
#include <codec/detail/modp_md5.h>
#include <sqsafe/detail/modp_aes.h>
#include "minialgo.h"
#include "../src/sqsafe/detail/aes/rdtsc.h"
#include <vmp/sqvmsdk.h>
#include <codec/detail/sha1.h>
#include <codec/sqcodec.h>
#include <sqsafe/detail/crypto_aes.h>


namespace snqu { namespace safe {


SQSafeAlgo::SQSafeAlgo()
{
}

SQSafeAlgo::~SQSafeAlgo()
{
}

string SQSafeAlgo::md5_variation(const string& src)
{
	_VMProtectBegin(__FUNCTION__);
	string x("");
	if (!src.empty()) {
		MD5 md5(src.c_str(), (size_t)src.size());
		x = md5.toString();

		string tx(x.size(), '\0');
		simple_swap_byte_encrypt((uint8*)&(tx[0]), (uint8*)x.c_str(), x.size());

		MD5 newmd5(tx.c_str(), (size_t)tx.size());
		x = newmd5.toString();
	}
	_VMProtectEnd();
	return x;
}

string SQSafeAlgo::md5_file_variation(const string& filepath)
{
	_VMProtectBegin(__FUNCTION__);
	string file_data = load_file(filepath);
	return md5_variation(file_data);
	_VMProtectEnd();
}

string SQSafeAlgo::aes_encrypt(const string& input, char *key, char *iv)
{
	_VMProtectBegin(__FUNCTION__);
	string x("");
	AES128_CBC_encrypt_buffer(input, key, iv, x);
	return x;
	_VMProtectEnd();
}

string SQSafeAlgo::aes_decrypt(const string& input, char *key, char *iv)
{
	_VMProtectBegin(__FUNCTION__);
	string x("");
	AES128_CBC_decrypt_buffer(input, key, iv, x);
	return x;
	_VMProtectEnd();
}

string SQSafeAlgo::aes_encryptfx(const string& input, 
								 const string& public_key)
{
	_VMProtectBegin(__FUNCTION__);
	if (input.size() <= 0)
		return "";

	string aeskey("");
	string iv("");
	get_aes_private_key(public_key, aeskey, iv);
	return aes_encrypt(input, const_cast<char*>(aeskey.c_str()), 
					   const_cast<char*>(iv.c_str()));
	_VMProtectEnd();
}

string SQSafeAlgo::aes_encryptfx_b64(const string& input, 
								     const string& public_key)
{
	_VMProtectBegin(__FUNCTION__);
	if (input.size() <= 0)
		return "";

	// 返回参数为base64串
	return codec::B64Encode(aes_encryptfx(input, public_key));
	_VMProtectEnd();
}

string SQSafeAlgo::aes_decryptfx(const string& input, 
								 const string& public_key)
{
	_VMProtectBegin(__FUNCTION__);
	if (input.size() <= 0)
		return "";

	string aeskey("");
	string iv("");
	get_aes_private_key(public_key, aeskey, iv);
	return aes_decrypt(input, const_cast<char*>(aeskey.c_str()), 
					   const_cast<char*>(iv.c_str()));
	_VMProtectEnd();
}

string SQSafeAlgo::aes_decryptfx_b64(const string& input, 
									 const string& public_key)
{
	if (input.size() <= 0)
		return "";

	// 输入参数为base64串
	return aes_decryptfx(codec::B64Decode(input), public_key);
}

int32 SQSafeAlgo::estimate_aes_size(int32 size)
{
	int32 length = size;
	if (length % AES_BLOCK_LEN != 0) {
		length += (AES_BLOCK_LEN - (length % AES_BLOCK_LEN));
	}

	return length;
}

string SQSafeAlgo::aes_b64_encrypt(const string& input, char *key, char *iv)
{
	// 返回值为base64串
	string x = aes_encrypt(input, key, iv);
	return codec::B64Encode(x);
}

string SQSafeAlgo::aes_b64_decrypt(const string& input, char *key, char *iv)
{
	// 输入参数为base64串
	string x = codec::B64Decode(input);
	return aes_decrypt(x, key, iv);
}

string SQSafeAlgo::aes_encryptx(const string& input, 
								const string& public_key,
								string& private_key)
{
	string aeskey("");
	string aesiv("");
	string x("");

	_VMProtectBegin(__FUNCTION__);

	// 根据public_key计算私钥
	string fixed_key = public_key;
	private_key = fmt::Format("{0}", read_tsc());
	fixed_key.append(private_key);
	get_aes_private_key(fixed_key, aeskey, aesiv);
	x = aes_encrypt(input, 
					const_cast<char *>(aeskey.c_str()), 
					const_cast<char *>(aesiv.c_str()));

	_VMProtectEnd();
	return x;
}

string SQSafeAlgo::aes_decryptx(const string& input, 
								const string& public_key,
								const string& private_key)
{
	string aeskey("");
	string aesiv("");
	string x("");

	_VMProtectBegin(__FUNCTION__);

	// 根据public_key计算私钥
	string fixed_key = public_key;
	fixed_key.append(private_key);
	get_aes_private_key(fixed_key, aeskey, aesiv);
	x = aes_decrypt(input, 
					const_cast<char *>(aeskey.c_str()), 
					const_cast<char *>(aesiv.c_str()));

	_VMProtectEnd();
	return x;
}

string SQSafeAlgo::aes_encryptx_b64(const string& input, 
									const string& public_key,
									string& private_key)
{
	_VMProtectBegin(__FUNCTION__);
	// 输出参数为base64串, base64_encode(aesx)
	string x = aes_encryptx(input, public_key, private_key);
	return codec::B64Encode(x);
	_VMProtectEnd();
}

string SQSafeAlgo::aes_decryptx_b64(const string& input, 
									const string& public_key,
									const string& private_key)
{
	_VMProtectBegin(__FUNCTION__);
	// 输入参数为base64串, aesx(base64_decode)
	string x = codec::B64Decode(input);
	return aes_decryptx(x, public_key, private_key);
	_VMProtectEnd();
}

void SQSafeAlgo::get_aes_private_key(const string& public_key, string& key, string& iv)
{
	_VMProtectBegin(__FUNCTION__);
	// 根据public_key计算私钥
	string varmd5 = md5_variation(public_key);
	key = varmd5.substr(0, 16);

	for (int i = varmd5.size() - 4, j = 0; i > 0; i--, j++) {
		iv.append(&varmd5[i], 1);
		if (j >= 15)
			break;
	}
	_VMProtectEnd();
}

uint64_t SQSafeAlgo::local_time64()
{
	_VMProtectBegin(__FUNCTION__);
	SYSTEMTIME systm;
	GetLocalTime(&systm);
	return (INT64)(time(NULL) * 1000 + systm.wMilliseconds);
	_VMProtectEnd();
}

string SQSafeAlgo::rc4_algofx(const string& input, 
							  const string& public_key)
{
	_VMProtectBegin(__FUNCTION__);
	string private_key = md5_variation(public_key);
	return codec::RC4Encode(input, private_key);
	_VMProtectEnd();
}

void SQSafeAlgo::simple_encrypt_data(string& data, 
									 const string& key)
{
	_VMProtectBegin(__FUNCTION__);
	internal_simple_encrypt_data(data, key);
	_VMProtectEnd();
}

string SQSafeAlgo::get_sha1_variation(const string& input)
{
	_VMProtectBegin(__FUNCTION__);
	return SHA1Algo_Variation(input);
	_VMProtectEnd();
}


//////////////////////////////////////////////////////////////////////////
SQSafeHelper::SQSafeHelper()
{
}

SQSafeHelper::~SQSafeHelper()
{
}

void SQSafeHelper::encrypt_file(const string& filepath, const string& key)
{
	_VMProtectBegin(__FUNCTION__);
	string data = load_file(filepath);
	if (data.size() > 0) {
		internal_simple_encrypt_data(data, key);
		save_file(filepath, data);
	}

	_VMProtectEnd();
}

string SQSafeHelper::php_decryptx_b64(const string& input, 
						const string& public_key,
						const string& private_key)
{
	_VMProtectBegin(__FUNCTION__);
	string x = input;
	modp_url_b64_decode(x);
	return aes_decryptx(x, public_key, private_key, x.size());
	_VMProtectEnd();
}

int SQSafeHelper::cphp_decryptx_b64(const char *input,
									int input_size,
									const char *public_key,
									int pubkey_size,
									const char *private_key,
									char **output,
									int *output_size)
{
	_VMProtectBegin(__FUNCTION__);
	char *xb64_data;
	char *dcrypt_data;
	int xb64_size = 0;
	int db64_size = 0;
	int result_size = 0;

	if (output_size != NULL)
		*output_size = 0;

	db64_size = cmodp_url_b64_decode(input, input_size, &xb64_data, &xb64_size);
	if (db64_size > 0) {
		result_size = caes_decryptx(xb64_data, db64_size, public_key, pubkey_size, private_key, &dcrypt_data, NULL);
		if (result_size > 0) {
			*output = dcrypt_data;
			if (output_size != NULL) 
				*output_size = result_size;
		}

		free(xb64_data);
	}

	return result_size;
	_VMProtectEnd();
}

string SQSafeHelper::aes_decryptx(const string& input, 
								const string& public_key,
								const string& private_key,
								int out_size)
{
	_VMProtectBegin(__FUNCTION__);
	string aeskey("");
	string aesiv("");
	string x(out_size, '\0');
	// 根据public_key计算私钥
	string fixed_key = public_key;
    fixed_key.append(private_key);

	SQSafeAlgo algo;
	algo.get_aes_private_key(fixed_key, aeskey, aesiv);
    x = crypto_AES128_CBC_decrypt(input, aeskey.c_str(), aesiv.c_str());
	return x;
	_VMProtectEnd();
}

int SQSafeHelper::caes_decryptx(const char *input, 
								int input_size,
								const char *public_key,
								int pubkey_size,
								const char *private_key,
								char **output,
								int *output_size)
{
	_VMProtectBegin(__FUNCTION__);
	string aeskey("");
	string aesiv("");
	char *decrypt_data = (char *)malloc(input_size);
	int decrypt_size = 0;
	int result_size = 0;

	// 根据public_key计算私钥
	string fixed_key = public_key;
	fixed_key.append(private_key);

	SQSafeAlgo algo;
	algo.get_aes_private_key(fixed_key, aeskey, aesiv);
	result_size = CPHP_AES128_CBC_decrypt(input, input_size, (char *)aeskey.c_str(), 
		(char *)aesiv.c_str(), decrypt_data, &decrypt_size);
	if (result_size > 0) {
		*output = decrypt_data;
		if (output_size != NULL)
			*output_size = result_size;
	}
	else {
		free(decrypt_data);
	}

	return result_size;
	_VMProtectEnd();
}

string SQSafeHelper::php_encryptx(const string& input, 
								  const string& public_key,
								  string& private_key)
{
	_VMProtectBegin(__FUNCTION__);
	return aes_encryptx(input, public_key, private_key);
	_VMProtectEnd();
}

string SQSafeHelper::php_encryptx_stream(const string& input, 
                                  const string& public_key,
                                  string& private_key)
{
    _VMProtectBegin(__FUNCTION__);
    return aes_encryptx_stream(input, public_key, private_key);
    _VMProtectEnd();
}

string SQSafeHelper::php_decryptx(const string& input, 
								  const string& public_key,
								  const string& private_key)
{
	_VMProtectBegin(__FUNCTION__);
	return aes_decryptx(input, public_key, private_key, input.size());
	_VMProtectEnd();
}

string SQSafeHelper::php_encryptx_b64(const string& input, 
									  const string& public_key,
									  string& private_key)
{
	_VMProtectBegin(__FUNCTION__);
	return modp_url_b64_encode(aes_encryptx_stream(input, public_key, private_key));
	_VMProtectEnd();
}

string SQSafeHelper::aes_encryptx(const string& input, 
								  const string& public_key,
								  string& private_key)
{
	_VMProtectBegin(__FUNCTION__);
	string aeskey("");
	string aesiv("");

	SQSafeAlgo algo;
	int padlen = algo.estimate_aes_size(input.size());
	string x(padlen, '\0');

	// 根据public_key计算私钥
	string fixed_key = public_key;
	private_key = fmt::Format("{0}", read_tsc());
	fixed_key.append(private_key);
	
	algo.get_aes_private_key(fixed_key, aeskey, aesiv);
	int aeslen = PHP_AES128_CBC_encrypt(input, (char *)aeskey.c_str(), (char *)aesiv.c_str(), x);
	x.resize(aeslen);
	return x;
	_VMProtectEnd();
}

string SQSafeHelper::aes_encryptx_stream(const string& input, 
                                        const string& public_key,
                                        string& private_key)
{
    _VMProtectBegin(__FUNCTION__);
    string aeskey("");
    string aesiv("");

    SQSafeAlgo algo;
    int padlen = algo.estimate_aes_size(input.size());
    string x(padlen, '\0');

    // 根据public_key计算私钥
    string fixed_key = public_key;
    private_key = fmt::Format("{0}", read_tsc());
    fixed_key.append(private_key);

    algo.get_aes_private_key(fixed_key, aeskey, aesiv);
    //printf("aeskey:%s, aesiv:%s\r\n", aeskey.c_str(), aesiv.c_str());
    x = crypto_AES128_CBC_encrypt(input, aeskey, aesiv);
    return x;
    _VMProtectEnd();
}

string SQSafeHelper::mild_encrypt_data(const string& input, 
									   const string& public_key)
{
	_VMProtectBegin(__FUNCTION__);
	SQSafeAlgo algo;
	string private_key = codec::MD5(public_key);
	string pkey = private_key.substr(0, 16);
	return codec::RC4Encode(input, pkey);
	_VMProtectEnd();
}

typedef unsigned char BYTE;  
inline BYTE toHex(const BYTE &x)  
{  
	return x > 9 ? x -10 + 'A': x + '0';  
}  

inline BYTE fromHex(const BYTE &x)  
{  
	return isdigit(x) ? x-'0' : x-'A'+10;  
}  

string SQSafeHelper::url_encode(const string &text)
{  
	string sOut;
	int text_size = strlen(text.c_str());
	if(!text.empty())
	{
		for (size_t ix = 0; ix < (size_t)text_size; ix++ )
		{        
			BYTE buf[4];
			memset( buf, 0, 4 );  
			if( isalnum( (BYTE)text[ix] ) )
			{        
				buf[0] = text[ix];  
			}
			else  
			{  
				buf[0] = '%';  
				buf[1] = toHex( (BYTE)text[ix] >> 4 );  
				buf[2] = toHex( (BYTE)text[ix] % 16);  
			}  
			sOut += (char *)buf;  
		} 
	}
	return sOut;  
};  

string SQSafeHelper::url_decode(const string &text)
{  
	string sOut;
	int text_size = strlen(text.c_str());
	if(!text.empty())
	{
		for (size_t ix = 0; ix < (size_t)text_size; ix++)
		{  
			BYTE ch = 0;  
			if(text[ix]=='%')  
			{  
				ch = (fromHex(text[ix+1])<<4);  
				ch |= fromHex(text[ix+2]);  
				ix += 2;  
			}  
			else if(text[ix] == '+')  
			{  
				ch = ' ';  
			}  
			else  
			{  
				ch = text[ix];  
			}  
			sOut += (char)ch;  
		}  
	}
	return sOut;  
}

//////////////////////////////////////////////////////////////////////////
ISQSafeModel::ISQSafeModel()
{
}

ISQSafeModel::~ISQSafeModel()
{
}

ISQSafeAlgo* ISQSafeModel::std_algo()
{
	static SQSafeAlgo algo;
	return reinterpret_cast<ISQSafeAlgo *>(&algo);
}

ISQSafeHelper* ISQSafeModel::helper()
{
	static SQSafeHelper helper;
	return reinterpret_cast<ISQSafeHelper *>(&helper);
}
}}