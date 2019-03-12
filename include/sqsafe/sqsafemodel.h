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
/*		标准加密解密功能的接口定义												 */
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
	     * 加密算法接口定义
	     */
	public:
		/**
		 * 计算变异的MD5值接口
		 * src			- 需要进行MD5运算的数据
		 * 
		 * 返回值:
		 * string		- 返回加密后的MD5字符串，长度为32的小写字符串，注意此
		 *				  函数为非标准的MD5串，而是内部进行了其它运算后返回的，
		 *				  可以用做内部安全校验使用。
		 */
		virtual string md5_variation(const string& src) = 0;

		/**
		 * 计算变异后的某个文件MD5值接口
		 * filepath		- 需要进行MD5运算的文件路径
		 * 
		 * 返回值:
		 * string		- 返回加密后的MD5字符串，长度为32的小写字符串，注意此
		 *				  函数为非标准的MD5串，而是内部进行了其它运算后返回的，
		 *				  可以用做内部安全校验使用。
		 */
		virtual string md5_file_variation(const string& filepath) = 0;

		/**
		 * 非标准AES加密
		 * input		- 需要进行AES加密的数据，类型为string
		 * public_key	- AES KEY,这个值可以是个固定的值（比如SITE_ID）
		 * private_key	- 返回的私有key,每次调用都会变化
		 * 
		 * 返回值:
		 * string		- 返回进行AES加密后的数据，注意public_key在内部要经过
		 *				- 特定的算法运算后，利用重新生成的key进行AES加密运算，
		 *				- 所以如果拿public_key进行标准的AES解密，是不能成功的。
		 */
		virtual string aes_encryptx(const string& input, 
									const string& public_key,
									string& private_key) = 0;

		/**
		 * 非标准AES解密
		 * input		- 需要进行AES解密的数据，类型为string
		 * public_key	- AES KEY,这个值可以是个固定的值（比如SITE_ID）
		 * private_key	- 由aes_encryptx生成的key
		 * 
		 * 返回值:
		 * string		- 返回进行AES加密后的数据，注意只有经过aes_encryptx函数
		 *				- 加密过的数据，才能用此函数进行解密，否则是不能成功的。
		 */
		virtual string aes_decryptx(const string& input, 
									const string& public_key,
									const string& private_key) = 0;

		/**
		 * 非标准AES加密，不返回私有key（适用于非网络传输数据的加密）
		 * input		- 需要进行AES加密的数据，类型为string
		 * public_key	- AES KEY,这个值可以是个固定的值（比如SITE_ID）
		 * 
		 * 返回值:
		 * string		- 返回进行AES加密后的数据，注意public_key在内部要经过
		 *				- 特定的算法运算后，利用重新生成的key进行AES加密运算，
		 *				- 所以如果拿public_key进行标准的AES解密，是不能成功的。
		 */
		virtual string aes_encryptfx(const string& input, 
									 const string& public_key) = 0;

		/**
		 * 非标准AES加密，不返回私有key（适用于非网络传输数据的加密）
		 * input		- 需要进行AES加密的数据，类型为string
		 * public_key	- AES KEY,这个值可以是个固定的值（比如SITE_ID）
		 * 
		 * 返回值:
		 * string		- 返回进行AES加密后的数据的base64串，注意public_key在内部要经过
		 *				- 特定的算法运算后，利用重新生成的key进行AES加密运算，
		 *				- 所以如果拿public_key进行标准的AES解密，是不能成功的。
		 */
		virtual string aes_encryptfx_b64(const string& input, 
										 const string& public_key) = 0;

		/**
		 * 非标准AES解密，不用私有key即可解密（适用于非网络传输数据的解密）
		 * input		- 需要进行AES解密的数据，类型为string
		 * public_key	- AES KEY,这个值可以是个固定的值（比如SITE_ID）
		 * 
		 * 返回值:
		 * string		- 返回进行AES加密后的数据，注意只有经过aes_encryptx函数
		 *				- 加密过的数据，才能用此函数进行解密，否则是不能成功的。
		 */
		virtual string aes_decryptfx(const string& input, 
									 const string& public_key) = 0;

		/**
		 * 非标准AES解密，不用私有key即可解密（适用于非网络传输数据的解密）
		 * input		- 需要进行AES解密的数据为base64串，类型为string
		 * public_key	- AES KEY,这个值可以是个固定的值（比如SITE_ID）
		 * 
		 * 返回值:
		 * string		- 返回进行AES加密后的数据，注意只有经过aes_encryptx函数
		 *				- 加密过的数据，才能用此函数进行解密，否则是不能成功的。
		 */
		virtual string aes_decryptfx_b64(const string& input, 
									     const string& public_key) = 0;

		/**
		 * 非标准AES加密，返回值为加密后数据的base64串
		 * input		- 需要进行AES加密的数据，类型为string
		 * public_key	- AES KEY,这个值可以是个固定的值（比如SITE_ID）
		 * private_key	- 返回的私有key,每次调用都会变化
		 * 
		 * 返回值:
		 * string		- 返回值为AES加密后的数据的base64串，算法和aes_encryptx
		 *				  相同，不同的是返回值为加密后的数据进行了base64运算， 此
		 *				  函数的目的是为了方便数据利用HTTP协议进行传输。
		 */
		virtual string aes_encryptx_b64(const string& input, 
										const string& public_key,
										string& private_key) = 0;

		/**
		 * 非标准AES解密
		 * input		- 输入参数为要解密数据的base64串
		 * public_key	- AES KEY,这个值可以是个固定的值（比如SITE_ID）
		 * private_key	- 由aes_encryptx_b64生成的key
		 * 
		 * 返回值:
		 * string		- 返回进行AES加密后的数据，算法和aes_decryptx相同，不同
		 *				  的是输入参数（input）为要解密数据的base64串，此函数的目
		 *				  的是为了配合函数aes_encryptx_b64使用的。
		 */
		virtual string aes_decryptx_b64(const string& input, 
										const string& public_key,
										const string& private_key) = 0;	

		/**
		 * 非标准RC4解密
		 * input		- 输入参数为要加密或者解密的数据
		 * public_key	- KEY
		 *
		 * 返回值:
		 * string		- 返回进行RC4加密或者解密后的数据
		 */
		virtual string rc4_algofx(const string& input, 
								  const string& public_key) = 0;

		/**
		 * 简单解密，速度快
		 * input		- 输入参数为要加密或者解密的数据,输出也为此参数，会
		 *				  改变之前的数据内容
		 * key			- KEY
		 *
		 * 返回值:
		 * void			- 无
		 */
		virtual void simple_encrypt_data(string& data, 
										 const string& key) = 0;

		/**
		 * 计算一块数据的hash值
		 * input		- 输入参数为要计算hash的数据
		 * key			- KEY
		 *
		 * 返回值:
		 * string		- 返回的hash值，如果计算失败则为空
		 */
		virtual string get_sha1_variation(const string& input) = 0;
	};

	class ISQSafeHelper
	{
		/**
	     * 辅助加密解密功能接口定义
	     */
	public:
		/**
		 * 对某个文件的内容进行加密
		 * filepath		- 文件路径
		 * key			- 需要加密的key
		 * 
		 * 返回值:
		 * void			- 无
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
		 * 对数据进行轻度加密,适用于大文件加密
		 * input		- 需要加密的数据
		 * key			- 需要加密的key
		 * 
		 * 返回值:
		 * void			- 返回加密后的数据
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
	     * 使用此接口来获取上面的功能接口，此接口不需要释放，使用时，直接调用
		 * 静态成员get_algo_interface()或者get_helper_interface()即可。
	     */
	public:
		ISQSafeModel();
		virtual ~ISQSafeModel();

	public:
		/**
		 * 获取标准算法接口
		 *
		 * 返回值:
		 * ISQSafeAlgo*
		 */
		static ISQSafeAlgo* std_algo();

		/**
		 * 获取辅助加密解密接口
		 *
		 * 返回值:
		 * ISQSafeHelper*
		 */
		static ISQSafeHelper* helper();
	};
	
}}
#endif //SQSAFE_MODEL_H