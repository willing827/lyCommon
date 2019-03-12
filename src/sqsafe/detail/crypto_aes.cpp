#include <sqsafe/detail/crypto_aes.h>
#include <vmp/sqvmsdk.h>
#include "aes.h"
#include "modes.h"
#include "Hex.h"

namespace snqu{ namespace safe{

std::string crypto_AES128_CBC_encrypt(const std::string& plaint, const std::string& sKey, const std::string& sIV)
{
    _VMProtectBegin(__FUNCTION__);
    using namespace CryptoPP;
    std::string outstr;

    //野key  
    SecByteBlock key(AES::DEFAULT_KEYLENGTH);
    memset(key, 0x00, key.size());
    sKey.size() <= AES::DEFAULT_KEYLENGTH ? memcpy(key, sKey.c_str(), sKey.size()) : memcpy(key, sKey.c_str(), AES::DEFAULT_KEYLENGTH);

    //野iv  
    byte iv[AES::BLOCKSIZE];
    memset(iv, 0x00, AES::BLOCKSIZE);
    sIV.size() <= AES::BLOCKSIZE ? memcpy(iv, sIV.c_str(), sIV.size()) : memcpy(iv, sIV.c_str(), AES::BLOCKSIZE);

    AES::Encryption aesEncryption((byte *)key, AES::DEFAULT_KEYLENGTH);

    CBC_Mode_ExternalCipher::Encryption cbcEncryption(aesEncryption, iv);

    try
    {
        StreamTransformationFilter cbcEncryptor(cbcEncryption, new StringSink(outstr), BlockPaddingSchemeDef::BlockPaddingScheme::PKCS_PADDING);
        cbcEncryptor.Put((byte *)plaint.c_str(), plaint.length());
        cbcEncryptor.MessageEnd();
    }
    catch(...)
    {

    }
    _VMProtectEnd();
    return std::move(outstr);
}

std::string crypto_AES128_CBC_decrypt(const std::string& plaint, const std::string& sKey, const std::string& sIV)
{
    _VMProtectBegin(__FUNCTION__);
    using namespace CryptoPP;
    std::string outstr;

    //野key  
    SecByteBlock key(AES::DEFAULT_KEYLENGTH);
    memset(key, 0x00, key.size());
    sKey.size() <= AES::DEFAULT_KEYLENGTH ? memcpy(key, sKey.c_str(), sKey.size()) : memcpy(key, sKey.c_str(), AES::DEFAULT_KEYLENGTH);

    //野iv  
    byte iv[AES::BLOCKSIZE];
    memset(iv, 0x00, AES::BLOCKSIZE);
    sIV.size() <= AES::BLOCKSIZE ? memcpy(iv, sIV.c_str(), sIV.size()) : memcpy(iv, sIV.c_str(), AES::BLOCKSIZE);

    AES::Decryption aesDecryption((byte *)key, AES::DEFAULT_KEYLENGTH);

    CBC_Mode_ExternalCipher::Decryption cbcDecryption(aesDecryption, iv);
    try
    {
        StreamTransformationFilter cbcEncryptor(cbcDecryption, new StringSink(outstr), BlockPaddingSchemeDef::BlockPaddingScheme::PKCS_PADDING);
        cbcEncryptor.Put((byte *)plaint.c_str(), plaint.length());
        cbcEncryptor.MessageEnd();
    }
    catch (...)
    {
        
    }
    _VMProtectEnd();
    return std::move(outstr);
}

}}