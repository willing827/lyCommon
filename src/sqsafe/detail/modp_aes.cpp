#include <sqsafe/detail/modp_aes.h>
#include "aes/aes.h"
#include <sqsafe/detail/phpAES.h>
#include <vmp/sqvmsdk.h>
#include <sqstd/sqstringhelper.h>


namespace snqu { namespace safe {
void AES128_CBC_encrypt_buffer(const string& plaint, 
							   char *key,
							   char *iv,
							   string &cipher)
{
	_VMProtectBegin(__FUNCTION__);
	cipher.clear();

	aes_encrypt_ctx ctx[1];
	aes_encrypt_key((unsigned char*)key, 128, ctx);

	unsigned char Iv[3*AES_BLOCK_LEN] = {0};
	unsigned long wlen = AES_BLOCK_LEN;

	memcpy(Iv, iv, AES_BLOCK_LEN);

	int nPlaintSize = plaint.length();
	int nRead = 0 ;
	if (nPlaintSize<AES_BLOCK_LEN)
		nRead = nPlaintSize;
	else 
		nRead = AES_BLOCK_LEN;

	memcpy(Iv + AES_BLOCK_LEN, plaint.data(), nRead);
	if (nRead < AES_BLOCK_LEN)
	{
		for(int i = 0 ; i < AES_BLOCK_LEN; i++)
			Iv[i+AES_BLOCK_LEN] ^= Iv[i];

		aes_encrypt((const unsigned char*)(Iv+nRead),(unsigned char*)(Iv+nRead),ctx);
		nRead += AES_BLOCK_LEN;

		// write the IV and the encrypted file bytes
		cipher.append((char*)Iv,nRead);
		return;
	}
	else					
	{
		unsigned char *b1 = Iv, *b2 = b1 + AES_BLOCK_LEN, *b3 = b2 + AES_BLOCK_LEN, *bt;

		// write the IV
		cipher.append((char*)Iv, AES_BLOCK_LEN);
		for (;;)
		{
			int nCanProcess = 0; //当前循环能处理多少个字符
			if (nPlaintSize - nRead >= AES_BLOCK_LEN)
				nCanProcess = AES_BLOCK_LEN;
			else
				nCanProcess  = nPlaintSize - nRead;

			memcpy(b3, plaint.data()+nRead,nCanProcess);
			nRead +=nCanProcess;    //用nread 进行计数，标识已经加密的明文的数目

			for (int i = 0; i < AES_BLOCK_LEN; ++i)
				b1[i] ^= b2[i];
			aes_encrypt(b1, b1, ctx);

			if (nCanProcess> 0 && nCanProcess < AES_BLOCK_LEN)
			{
				wlen = nCanProcess;
				for(int i = 0; i < nCanProcess; ++i)
					b3[i] ^= b1[i];

				for(int i = nCanProcess; i < AES_BLOCK_LEN; ++i)
					b3[i] = b1[i];

				aes_encrypt(b3, b3, ctx);
				cipher.append((char*)b3, AES_BLOCK_LEN);
			}
			cipher.append((char*)b1,wlen);//计算出来的密文append到cipherStore的末尾

			if(nCanProcess != AES_BLOCK_LEN)
				return;

			bt = b3, b3 = b2, b2 = b1, b1 = bt;
		}
	}

	_VMProtectEnd();
}

void AES128_CBC_decrypt_buffer(const string& cipher, 
							   char *key,
							   char *iv,
							   string &plaint)
{
	_VMProtectBegin(__FUNCTION__);
	plaint.clear();

	aes_decrypt_ctx ctx_de[1];
	aes_decrypt_key((unsigned char*)key, 128, ctx_de);

	unsigned char Iv[3 * AES_BLOCK_LEN], buf[AES_BLOCK_LEN];	
	unsigned long wlen = AES_BLOCK_LEN;

	int ncipherSize = cipher.length();

	int len = 0 ;
	if(ncipherSize<2*AES_BLOCK_LEN)
		len = ncipherSize;
	else
		len = 2*AES_BLOCK_LEN;

	memcpy(Iv, cipher.data(), len);

	if (len < 2*AES_BLOCK_LEN)   
	{
		len -= AES_BLOCK_LEN ;
		aes_decrypt((const unsigned char *)(Iv + len), (unsigned char*)(Iv + len), ctx_de);

		for(int i = 0; i < len; ++i)
			Iv[i] ^= Iv[i + AES_BLOCK_LEN];

		plaint.append((char*)Iv, len);
		return;
	}
	else
	{
		int  nProcessCount = len;//总共已经解密了多少密文
		unsigned char *b1 = Iv, *b2 = b1 + AES_BLOCK_LEN, *b3 = b2 + AES_BLOCK_LEN, *bt;
		for (;;)
		{
			int nRead = 0;
			if (ncipherSize - nProcessCount < AES_BLOCK_LEN)
				nRead  = ncipherSize - nProcessCount;
			else
				nRead = AES_BLOCK_LEN;

			memcpy(b3, cipher.data() + nProcessCount, nRead);
			nProcessCount +=nRead;

			aes_decrypt(b2, buf, ctx_de);

			if (nRead==0 || nRead == AES_BLOCK_LEN)
			{
				for(int i = 0; i < AES_BLOCK_LEN; ++i)
					buf[i] ^= b1[i];
			}
			else
			{
				wlen = nRead;

				for(int i = 0; i < nRead; ++i)
					buf[i] ^= b3[i];

				for(int i = nRead; i < AES_BLOCK_LEN; ++i)
					b3[i] = buf[i];

				aes_decrypt(b3, b3, ctx_de);

				for(int i = 0; i < AES_BLOCK_LEN; ++i)
					b3[i] ^= b1[i];

				plaint.append((char*)b3, AES_BLOCK_LEN);

			}

			plaint.append((char*)buf, wlen);

			if(nRead != AES_BLOCK_LEN)
				return;

			bt = b1, b1 = b2, b2 = b3, b3 = bt;
		}
	}

	_VMProtectEnd();
}

int PHP_AES128_CBC_encrypt(const string& plaint, 
							char *key,
							char *iv,
							string &cipher)
{
	_VMProtectBegin(__FUNCTION__);
	Aes aes;
	aes.aesSetIV((uint8*)iv);
	return aes.aesEncrypt((uint8*)plaint.c_str(), (uint8*)key, (uint8*)&cipher[0]);
	_VMProtectEnd();
}

int PHP_AES128_CBC_Stream_encrypt(const string& plaint, 
                           char *key,
                           char *iv,
                           string &cipher)
{
    _VMProtectBegin(__FUNCTION__);
    Aes aes;
    aes.aesSetIV((uint8*)iv);
    return aes.aesEncryptStream((uint8*)plaint.c_str(), plaint.size(), (uint8*)key, (uint8*)&cipher[0]);
    _VMProtectEnd();
}



void PHP_AES128_CBC_decrypt(const string& cipher, 
							char *key,
							char *iv,
							string &plaint)
{
	_VMProtectBegin(__FUNCTION__);
	Aes aes;
	aes.aesSetIV((uint8*)iv);
	aes.aesDecrypt((uint8*)cipher.c_str(), (uint8*)key, (uint8*)&plaint[0], plaint.size());
    str::trim(plaint, "\a");
	_VMProtectEnd();
}

int CPHP_AES128_CBC_decrypt(const char *cipher,
							int cipher_size,
							char *key,
							char *iv,
							char *plaint,
							int *plaint_size)
{
	_VMProtectBegin(__FUNCTION__);
	Aes aes;
	aes.aesSetIV((uint8*)iv);
	*plaint_size = aes.aesDecrypt((uint8*)cipher, (uint8*)key, (uint8*)plaint, cipher_size);
	return *plaint_size;
	_VMProtectEnd();
}
}}