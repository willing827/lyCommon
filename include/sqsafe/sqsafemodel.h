/*--------------------------------------------------------------------------*/
/*  sasafemodel.h                                                            */
/*                                                                           */
/*  History                                                                  */
/*      12/05/2015                                                           */
/*                                                                           */
/*  Author                                                                   */
/*      Guo Lei																 */
/*                                                                           */
/*  Abstract																 */	
/*		��׼���ܽ��ܹ��ܵĽӿڶ���												 */
/*                                                                           */
/*  Copyright (C) 2015 by SNQU network technology Inc.                       */
/*  All rights reserved                                                      */
/*--------------------------------------------------------------------------*/
#ifndef SQSAFE_MODEL_H
#define SQSAFE_MODEL_H

#include <sqstd/sqinc.h>
namespace snqu { namespace safe {
	#define AES_BLOCK_LEN 16
	class ISQSafeAlgo
	{
		/**
	     * �����㷨�ӿڶ���
	     */
	public:
		/**
		 * ��������MD5ֵ�ӿ�
		 * src			- ��Ҫ����MD5���������
		 * 
		 * ����ֵ:
		 * string		- ���ؼ��ܺ��MD5�ַ���������Ϊ32��Сд�ַ�����ע���
		 *				  ����Ϊ�Ǳ�׼��MD5���������ڲ���������������󷵻صģ�
		 *				  ���������ڲ���ȫУ��ʹ�á�
		 */
		virtual string md5_variation(const string& src) = 0;

		/**
		 * ���������ĳ���ļ�MD5ֵ�ӿ�
		 * filepath		- ��Ҫ����MD5������ļ�·��
		 * 
		 * ����ֵ:
		 * string		- ���ؼ��ܺ��MD5�ַ���������Ϊ32��Сд�ַ�����ע���
		 *				  ����Ϊ�Ǳ�׼��MD5���������ڲ���������������󷵻صģ�
		 *				  ���������ڲ���ȫУ��ʹ�á�
		 */
		virtual string md5_file_variation(const string& filepath) = 0;

		/**
		 * �Ǳ�׼AES����
		 * input		- ��Ҫ����AES���ܵ����ݣ�����Ϊstring
		 * public_key	- AES KEY,���ֵ�����Ǹ��̶���ֵ������SITE_ID��
		 * private_key	- ���ص�˽��key,ÿ�ε��ö���仯
		 * 
		 * ����ֵ:
		 * string		- ���ؽ���AES���ܺ�����ݣ�ע��public_key���ڲ�Ҫ����
		 *				- �ض����㷨����������������ɵ�key����AES�������㣬
		 *				- ���������public_key���б�׼��AES���ܣ��ǲ��ܳɹ��ġ�
		 */
		virtual string aes_encryptx(const string& input, 
									const string& public_key,
									string& private_key) = 0;

		/**
		 * �Ǳ�׼AES����
		 * input		- ��Ҫ����AES���ܵ����ݣ�����Ϊstring
		 * public_key	- AES KEY,���ֵ�����Ǹ��̶���ֵ������SITE_ID��
		 * private_key	- ��aes_encryptx���ɵ�key
		 * 
		 * ����ֵ:
		 * string		- ���ؽ���AES���ܺ�����ݣ�ע��ֻ�о���aes_encryptx����
		 *				- ���ܹ������ݣ������ô˺������н��ܣ������ǲ��ܳɹ��ġ�
		 */
		virtual string aes_decryptx(const string& input, 
									const string& public_key,
									const string& private_key) = 0;

		/**
		 * �Ǳ�׼AES���ܣ�������˽��key�������ڷ����紫�����ݵļ��ܣ�
		 * input		- ��Ҫ����AES���ܵ����ݣ�����Ϊstring
		 * public_key	- AES KEY,���ֵ�����Ǹ��̶���ֵ������SITE_ID��
		 * 
		 * ����ֵ:
		 * string		- ���ؽ���AES���ܺ�����ݣ�ע��public_key���ڲ�Ҫ����
		 *				- �ض����㷨����������������ɵ�key����AES�������㣬
		 *				- ���������public_key���б�׼��AES���ܣ��ǲ��ܳɹ��ġ�
		 */
		virtual string aes_encryptfx(const string& input, 
									 const string& public_key) = 0;

		/**
		 * �Ǳ�׼AES���ܣ�������˽��key�������ڷ����紫�����ݵļ��ܣ�
		 * input		- ��Ҫ����AES���ܵ����ݣ�����Ϊstring
		 * public_key	- AES KEY,���ֵ�����Ǹ��̶���ֵ������SITE_ID��
		 * 
		 * ����ֵ:
		 * string		- ���ؽ���AES���ܺ�����ݵ�base64����ע��public_key���ڲ�Ҫ����
		 *				- �ض����㷨����������������ɵ�key����AES�������㣬
		 *				- ���������public_key���б�׼��AES���ܣ��ǲ��ܳɹ��ġ�
		 */
		virtual string aes_encryptfx_b64(const string& input, 
										 const string& public_key) = 0;

		/**
		 * �Ǳ�׼AES���ܣ�����˽��key���ɽ��ܣ������ڷ����紫�����ݵĽ��ܣ�
		 * input		- ��Ҫ����AES���ܵ����ݣ�����Ϊstring
		 * public_key	- AES KEY,���ֵ�����Ǹ��̶���ֵ������SITE_ID��
		 * 
		 * ����ֵ:
		 * string		- ���ؽ���AES���ܺ�����ݣ�ע��ֻ�о���aes_encryptx����
		 *				- ���ܹ������ݣ������ô˺������н��ܣ������ǲ��ܳɹ��ġ�
		 */
		virtual string aes_decryptfx(const string& input, 
									 const string& public_key) = 0;

		/**
		 * �Ǳ�׼AES���ܣ�����˽��key���ɽ��ܣ������ڷ����紫�����ݵĽ��ܣ�
		 * input		- ��Ҫ����AES���ܵ�����Ϊbase64��������Ϊstring
		 * public_key	- AES KEY,���ֵ�����Ǹ��̶���ֵ������SITE_ID��
		 * 
		 * ����ֵ:
		 * string		- ���ؽ���AES���ܺ�����ݣ�ע��ֻ�о���aes_encryptx����
		 *				- ���ܹ������ݣ������ô˺������н��ܣ������ǲ��ܳɹ��ġ�
		 */
		virtual string aes_decryptfx_b64(const string& input, 
									     const string& public_key) = 0;

		/**
		 * �Ǳ�׼AES���ܣ�����ֵΪ���ܺ����ݵ�base64��
		 * input		- ��Ҫ����AES���ܵ����ݣ�����Ϊstring
		 * public_key	- AES KEY,���ֵ�����Ǹ��̶���ֵ������SITE_ID��
		 * private_key	- ���ص�˽��key,ÿ�ε��ö���仯
		 * 
		 * ����ֵ:
		 * string		- ����ֵΪAES���ܺ�����ݵ�base64�����㷨��aes_encryptx
		 *				  ��ͬ����ͬ���Ƿ���ֵΪ���ܺ�����ݽ�����base64���㣬 ��
		 *				  ������Ŀ����Ϊ�˷�����������HTTPЭ����д��䡣
		 */
		virtual string aes_encryptx_b64(const string& input, 
										const string& public_key,
										string& private_key) = 0;

		/**
		 * �Ǳ�׼AES����
		 * input		- �������ΪҪ�������ݵ�base64��
		 * public_key	- AES KEY,���ֵ�����Ǹ��̶���ֵ������SITE_ID��
		 * private_key	- ��aes_encryptx_b64���ɵ�key
		 * 
		 * ����ֵ:
		 * string		- ���ؽ���AES���ܺ�����ݣ��㷨��aes_decryptx��ͬ����ͬ
		 *				  �������������input��ΪҪ�������ݵ�base64�����˺�����Ŀ
		 *				  ����Ϊ����Ϻ���aes_encryptx_b64ʹ�õġ�
		 */
		virtual string aes_decryptx_b64(const string& input, 
										const string& public_key,
										const string& private_key) = 0;	

		/**
		 * �Ǳ�׼RC4����
		 * input		- �������ΪҪ���ܻ��߽��ܵ�����
		 * public_key	- KEY
		 *
		 * ����ֵ:
		 * string		- ���ؽ���RC4���ܻ��߽��ܺ������
		 */
		virtual string rc4_algofx(const string& input, 
								  const string& public_key) = 0;

		/**
		 * �򵥽��ܣ��ٶȿ�
		 * input		- �������ΪҪ���ܻ��߽��ܵ�����,���ҲΪ�˲�������
		 *				  �ı�֮ǰ����������
		 * key			- KEY
		 *
		 * ����ֵ:
		 * void			- ��
		 */
		virtual void simple_encrypt_data(string& data, 
										 const string& key) = 0;

		/**
		 * ����һ�����ݵ�hashֵ
		 * input		- �������ΪҪ����hash������
		 * key			- KEY
		 *
		 * ����ֵ:
		 * string		- ���ص�hashֵ���������ʧ����Ϊ��
		 */
		virtual string get_sha1_variation(const string& input) = 0;
	};

	class ISQSafeHelper
	{
		/**
	     * �������ܽ��ܹ��ܽӿڶ���
	     */
	public:
		/**
		 * ��ĳ���ļ������ݽ��м���
		 * filepath		- �ļ�·��
		 * key			- ��Ҫ���ܵ�key
		 * 
		 * ����ֵ:
		 * void			- ��
		 */
		virtual void encrypt_file(const string& filepath, const string& key) = 0;


		virtual string php_decryptx_b64(const string& input, 
										const string& public_key,
										const string& private_key) = 0;

		virtual string php_encryptx_b64(const string& input, 
										const string& public_key,
										string& private_key) = 0;

		virtual string php_encryptx(const string& input, 
									const string& public_key,
									string& private_key) = 0;

        virtual string php_encryptx_stream(const string& input, 
                                    const string& public_key,
                                    string& private_key) = 0;

		virtual string php_decryptx(const string& input, 
			const string& public_key,
			const string& private_key) = 0;

		virtual string url_encode(const string &text) = 0;
		virtual string url_decode(const string &text) = 0;

		/**
		 * �����ݽ�����ȼ���,�����ڴ��ļ�����
		 * input		- ��Ҫ���ܵ�����
		 * key			- ��Ҫ���ܵ�key
		 * 
		 * ����ֵ:
		 * void			- ���ؼ��ܺ������
		 */
		virtual string mild_encrypt_data(const string& input, 
									     const string& public_key) = 0;

	public:
		virtual int caes_decryptx(const char *input, 
			int input_size,
			const char *public_key,
			int pubkey_size,
			const char *private_key,
			char **output,
			int *output_size) = 0;

		virtual int cphp_decryptx_b64(const char *input,
			int input_size,
			const char *public_key,
			int pubkey_size,
			const char *private_key,
			char **output,
			int *output_size) = 0;
	};

	//////////////////////////////////////////////////////////////////////////
	// class ISQSafeModel
	class ISQSafeModel
	{
		/**
	     * ʹ�ô˽ӿ�����ȡ����Ĺ��ܽӿڣ��˽ӿڲ���Ҫ�ͷţ�ʹ��ʱ��ֱ�ӵ���
		 * ��̬��Աget_algo_interface()����get_helper_interface()���ɡ�
	     */
	public:
		ISQSafeModel();
		virtual ~ISQSafeModel();

	public:
		/**
		 * ��ȡ��׼�㷨�ӿ�
		 *
		 * ����ֵ:
		 * ISQSafeAlgo*
		 */
		static ISQSafeAlgo* std_algo();

		/**
		 * ��ȡ�������ܽ��ܽӿ�
		 *
		 * ����ֵ:
		 * ISQSafeHelper*
		 */
		static ISQSafeHelper* helper();
	};
	
}}
#endif //SQSAFE_MODEL_H