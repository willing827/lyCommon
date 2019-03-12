/*--------------------------------------------------------------------------*/
/*  safemodel.h                                                            */
/*                                                                           */
/*  History                                                                  */
/*      12/05/2015                                                           */
/*                                                                           */
/*  Author                                                                   */
/*      Guo Lei																 */
/*                                                                           */
/*  Copyright (C) 2015 by SNQU network technology Inc.                       */
/*  All rights reserved                                                      */
/*--------------------------------------------------------------------------*/
#ifndef SAFEMODEL_H
#define SAFEMODEL_H

#include <sqsafe/sqsafemodel.h>
#include <sqstd/sqsysfun.h>

//need cryptlib_mt.lib
//need libcrypto.lib crypt32.lib

namespace snqu { namespace safe {
	class SQSafeAlgo : public ISQSafeAlgo
	{
	public:
		SQSafeAlgo();
		virtual ~SQSafeAlgo();

	public:
		virtual string md5_variation(const string& src) override;
		virtual string md5_file_variation(const string& filepath) override;

		virtual string aes_encryptx(const string& input, 
									const string& public_key,
									string& private_key) override;

		virtual string aes_decryptx(const string& input, 
									const string& public_key,
									const string& private_key) override;

		virtual string aes_encryptfx(const string& input, 
									 const string& public_key) override;

		virtual string aes_decryptfx(const string& input, 
									 const string& public_key) override;

		virtual string aes_encryptfx_b64(const string& input, 
										 const string& public_key) override;

		virtual string aes_decryptfx_b64(const string& input, 
										 const string& public_key) override;

		virtual string aes_encryptx_b64(const string& input, 
										const string& public_key,
										string& private_key) override;

		virtual string aes_decryptx_b64(const string& input, 
										const string& public_key,
										const string& private_key) override;

		virtual string rc4_algofx(const string& input, 
								  const string& public_key) override;

		virtual void simple_encrypt_data(string& data, 
										 const string& key) override;

		virtual string get_sha1_variation(const string& input) override;

	protected:
		string aes_encrypt(const string& input, char *key, char *iv);
		string aes_decrypt(const string& input, char *key, char *iv);
		string aes_b64_encrypt(const string& input, char *key, char *iv);
		string aes_b64_decrypt(const string& input, char *key, char *iv);
		
	public:
		static int32 estimate_aes_size(int32 size);
		void get_aes_private_key(const string& public_key, string& key, string& iv);
		uint64_t local_time64(); 
	};

	//////////////////////////////////////////////////////////////////////////
	// class SQSafeHelper
	class SQSafeHelper : public ISQSafeHelper
	{
	public:
		SQSafeHelper();
		virtual ~SQSafeHelper();

	public:
		virtual void encrypt_file(const string& filepath, const string& key) override;
		
		virtual string php_decryptx_b64(const string& input, 
										const string& public_key,
										const string& private_key) override;


		virtual string php_encryptx_b64(const string& input, 
										const string& public_key,
										string& private_key) override;

		virtual string php_encryptx(const string& input, 
									const string& public_key,
									string& private_key) override;

        virtual string php_encryptx_stream(const string& input, 
                                    const string& public_key,
                                    string& private_key) override;

		virtual string php_decryptx(const string& input, 
									const string& public_key,
									const string& private_key) override;

		virtual string url_encode(const string &text) override;
		virtual string url_decode(const string &text) override;

		virtual string mild_encrypt_data(const string& input, 
									     const string& public_key) override;

	public:
		virtual int caes_decryptx(const char *input, 
			int input_size,
			const char *public_key,
			int pubkey_size,
			const char *private_key,
			char **output,
			int *output_size);

		virtual int cphp_decryptx_b64(const char *input,
			int input_size,
			const char *public_key,
			int pubkey_size,
			const char *private_key,
			char **output,
			int *output_size);

	protected:
		string aes_decryptx(const string& input, 
							const string& public_key,
							const string& private_key,
							int out_size);

		string aes_encryptx(const string& input, 
							const string& public_key,
							string& private_key);

        string aes_encryptx_stream(const string& input, 
            const string& public_key,
            string& private_key);
	};
}}
#endif // SAFEMODEL_H