#include <sqsafe/detail/openssl_aes.h>
#include <openssl/aes.h> 
#include <openssl/evp.h>

namespace snqu{ namespace safe{

int padding_len(int size)
{
    int length = size;
    if (length % AES_BLOCK_SIZE != 0) {
        length += (AES_BLOCK_SIZE - (length % AES_BLOCK_SIZE));
    }
    else
    {
        length += AES_BLOCK_SIZE;
    }

    return length;
}

//加密
std::string openssl_cbc_128_encrypt(const std::string& in_data, const std::string& key, const std::string& iv)
{
    std::string out_data;
    if (in_data.empty()) return out_data;

    EVP_CIPHER_CTX *ctx = NULL;
    int len = 0;
    int isSuccess = 0;
    int in_length = in_data.length();
    out_data.resize(padding_len(in_length));
    unsigned char* in_buf = (unsigned char*)in_data.c_str();
    unsigned char* out_buf = (unsigned char*)&out_data[0];

    do
    {
        ctx = EVP_CIPHER_CTX_new();
        if (NULL == ctx)
        {
            isSuccess = -1;
            break;
        }
        //初始化ctx，加密算法初始化  
        isSuccess = EVP_EncryptInit_ex(ctx, EVP_aes_128_cbc(), NULL, (unsigned char*)key.c_str(), (unsigned char*)iv.c_str());
        if (!isSuccess) break;

        //加密数据  
        isSuccess = EVP_EncryptUpdate(ctx, out_buf, &len, in_buf, in_length);
        if (!isSuccess) break;
        in_length = len;

        EVP_EncryptFinal_ex(ctx, out_buf + len, &len);
        if (!isSuccess) break;
        in_length += len;
        out_data.resize(in_length);

    } while (0);

    if (NULL != ctx)
    {
        EVP_CIPHER_CTX_free(ctx);
    }
    return out_data;
}

//解密
std::string openssl_cbc_128_decrypt(const std::string& in_data, const std::string& key, const std::string& iv)
{
    std::string out_data;
    if (in_data.empty()) return out_data;

    EVP_CIPHER_CTX *ctx = NULL;
    int len = 0;
    int out_len = 0;
    int isSuccess;
    int in_length = in_data.length();
    out_data.resize(in_length);
    unsigned char* in_buf = (unsigned char*)in_data.c_str();
    unsigned char* out_buf = (unsigned char*)&out_data[0];

    do
    {
        ctx = EVP_CIPHER_CTX_new();
        if (NULL == ctx)
        {
            isSuccess = -1;
            break;
        }

        //初始化ctx，加密算法初始化  
        isSuccess = EVP_DecryptInit_ex(ctx, EVP_aes_128_cbc(), NULL, (unsigned char*)key.c_str(), (unsigned char*)iv.c_str());
        if (!isSuccess) break;

        //解密数据  
        isSuccess = EVP_DecryptUpdate(ctx, out_buf, &len, in_buf, in_length);
        if (!isSuccess) break;
        out_len = len;

        isSuccess = EVP_DecryptFinal_ex(ctx, out_buf + out_len, &len);
        if (!isSuccess) break;
        out_len += len;
        /*解密数据块不为16整数倍时执行*/
        out_data.resize(out_len);

    } while (0);

    if (NULL != ctx)
    {
        EVP_CIPHER_CTX_free(ctx);
    }
    return out_data;
}

}}

