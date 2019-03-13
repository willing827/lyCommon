#include <sqsafe/detail/WinCrypto.h>
#include <sqwin/sqwin.h>
#include <bcrypt.h>
#include <sqlog/sqlog.h>
#include <sqnet/sqnetapi.h>

#pragma comment(lib, "Bcrypt.lib")
#pragma comment(lib, "crypt32.lib")

namespace snqu {

#define NT_SUCCESS(Status)          (((NTSTATUS)(Status)) >= 0)
#define STATUS_UNSUCCESSFUL         ((NTSTATUS)0xC0000001L)

namespace detail {

std::string WinHash(const std::string& input, const wchar_t* hash_type)
{
	std::string ret_data;
	BCRYPT_ALG_HANDLE       hAlg = NULL;
	BCRYPT_HASH_HANDLE      hHash = NULL;
	NTSTATUS                status = STATUS_UNSUCCESSFUL;
	DWORD                   cbData = 0,
		cbHash = 0,
		cbHashObject = 0;
	PBYTE                   pbHashObject = NULL;
	PBYTE                   pbHash = NULL;

	do
	{
		//open an algorithm handle
		if (!NT_SUCCESS(status = BCryptOpenAlgorithmProvider(&hAlg, hash_type, NULL, 0)))
		{
			SNLOG(kTrace, "**** Error 0x%x returned by BCryptOpenAlgorithmProvider", status);
			break;
		}

		//calculate the size of the buffer to hold the hash object
		if (!NT_SUCCESS(status = BCryptGetProperty(hAlg, BCRYPT_OBJECT_LENGTH, (PBYTE)&cbHashObject, sizeof(DWORD), &cbData, 0)))
		{
			SNLOG(kTrace, "**** Error 0x%x returned by BCryptGetProperty", status);
			break;
		}

		//allocate the hash object on the heap
		pbHashObject = (PBYTE)HeapAlloc(GetProcessHeap(), 0, cbHashObject);
		if (NULL == pbHashObject)
		{
			SNLOG(kTrace, "**** memory allocation failed");
			break;
		}

		//calculate the length of the hash
		if (!NT_SUCCESS(status = BCryptGetProperty(hAlg, BCRYPT_HASH_LENGTH, (PBYTE)&cbHash, sizeof(DWORD), &cbData, 0)))
		{
			SNLOG(kTrace, "**** Error 0x%x returned by BCryptGetProperty", status);
			break;
		}

		//allocate the hash buffer on the heap
		pbHash = (PBYTE)HeapAlloc(GetProcessHeap(), 0, cbHash);
		if (NULL == pbHash)
		{
			SNLOG(kTrace, "**** memory allocation failed");
			break;
		}

		//create a hash
		if (!NT_SUCCESS(status = BCryptCreateHash(hAlg, &hHash, pbHashObject, cbHashObject, NULL, 0, 0)))
		{
			SNLOG(kTrace, "**** Error 0x%x returned by BCryptCreateHash", status);
			break;
		}

		//hash some data
		if (!NT_SUCCESS(status = BCryptHashData(hHash, (PBYTE)input.data(), input.length(), 0)))
		{
			SNLOG(kTrace, "**** Error 0x%x returned by BCryptHashData", status);
			break;
		}

		//close the hash
		if (!NT_SUCCESS(status = BCryptFinishHash(hHash, pbHash, cbHash, 0)))
		{
			SNLOG(kTrace, "**** Error 0x%x returned by BCryptFinishHash", status);
			break;
		}
		ret_data.assign((char*)pbHash, cbHash);
		ret_data = net::BinToHex(ret_data);
	} while (0);

	if (hAlg) BCryptCloseAlgorithmProvider(hAlg, 0);
	if (hHash) BCryptDestroyHash(hHash);
	if (pbHashObject) HeapFree(GetProcessHeap(), 0, pbHashObject);
	if (pbHash) HeapFree(GetProcessHeap(), 0, pbHash);

	return ret_data;
}

int InitCrypt(const std::string& key, const std::string& iv, BCRYPT_ALG_HANDLE& hAlg, BCRYPT_KEY_HANDLE& hKey, DWORD& cbBlockLen, PBYTE& pbIV, PBYTE& pbKeyObject, LPCWSTR ALG_TYPE)
{
	NTSTATUS                status = STATUS_UNSUCCESSFUL;
	DWORD                   cbData(0), cbKeyObject(0);

	do
	{
		// Open an algorithm handle.
		if (!NT_SUCCESS(status = BCryptOpenAlgorithmProvider(&hAlg, ALG_TYPE, MS_PRIMITIVE_PROVIDER, 0)))
		{
			SNLOG(kTrace, "**** Error 0x%x returned by BCryptOpenAlgorithmProvider", status);
			break;
		}

		// Calculate the size of the buffer to hold the KeyObject.
		if (!NT_SUCCESS(status = BCryptGetProperty(hAlg, BCRYPT_OBJECT_LENGTH, (PBYTE)&cbKeyObject, sizeof(DWORD), &cbData, 0)))
		{
			SNLOG(kTrace, "**** Error 0x%x returned by BCryptGetProperty", status);
			break;
		}

		// Allocate the key object on the heap.
		pbKeyObject = (PBYTE)HeapAlloc(GetProcessHeap(), 0, cbKeyObject);
		if (NULL == pbKeyObject)
		{
			SNLOG(kTrace, "**** memory allocation failed");
			break;
		}

		// Calculate the block length for the IV.
		if (!NT_SUCCESS(status = BCryptGetProperty(hAlg, BCRYPT_BLOCK_LENGTH, (PBYTE)&cbBlockLen, sizeof(DWORD), &cbData, 0)))
		{
			SNLOG(kTrace, "**** Error 0x%x returned by BCryptGetProperty", status);
			break;
		}

		if (!iv.empty())
		{
			// Determine whether the cbBlockLen is not longer than the IV length.
			if (cbBlockLen > iv.length())
			{
				SNLOG(kTrace, "**** block length is longer than the provided IV length");
				break;
			}

			// Allocate a buffer for the IV. The buffer is consumed during the encrypt/decrypt process.
			pbIV = (PBYTE)HeapAlloc(GetProcessHeap(), 0, cbBlockLen);
			if (NULL == pbIV)
			{
				SNLOG(kTrace, "**** memory allocation failed");
				break;
			}
			memset(pbIV, 0, cbBlockLen);
			memcpy_s(pbIV, cbBlockLen, iv.data(), min(iv.length(), cbBlockLen));
		}

		if (wcscmp(BCRYPT_RC4_ALGORITHM, ALG_TYPE) != 0)
		{
			if (!NT_SUCCESS(status = BCryptSetProperty(hAlg, BCRYPT_CHAINING_MODE, (PBYTE)BCRYPT_CHAIN_MODE_CBC, sizeof(BCRYPT_CHAIN_MODE_CBC), 0)))
			{
				//STATUS_INVALID_HANDLE
				SNLOG(kTrace, "**** Error 0x%x returned by BCryptSetProperty", status);
				break;
			}
		}

		// Generate the key from supplied input key bytes.
		if (!NT_SUCCESS(status = BCryptGenerateSymmetricKey(hAlg, &hKey, pbKeyObject, cbKeyObject, (PBYTE)key.data(), key.length(), 0)))
		{
			SNLOG(kTrace, "**** Error 0x%x returned by BCryptGenerateSymmetricKey", status);
			break;
		}

	} while (0);

	return status;
}

bool WinEncrypt(const std::string& input_data, const std::string& key, const std::string& iv, std::string& out_data, LPCWSTR ALG_TYPE)
{
	BCRYPT_ALG_HANDLE       hAlg = NULL;
	BCRYPT_KEY_HANDLE       hKey = NULL;
	NTSTATUS                status = STATUS_UNSUCCESSFUL;
	DWORD					cbBlockLen(0), iRetlen(0);
	PBYTE					pbIV(NULL), pbKey(NULL);
	PBYTE					pRetBuff(NULL);
	do
	{
		if (!NT_SUCCESS(status = InitCrypt(key, iv, hAlg, hKey, cbBlockLen, pbIV, pbKey, ALG_TYPE)))
		{
			SNLOG(kTrace, "**** Error 0x%x returned by InitCrypt", status);
			break;
		}

		// Get the output buffer size.
		if (!NT_SUCCESS(status = BCryptEncrypt(hKey, (PBYTE)input_data.data(), input_data.length(), NULL, pbIV, pbIV ? cbBlockLen : 0, NULL, 0, &iRetlen, pbIV ? BCRYPT_BLOCK_PADDING : 0)))
		{
			SNLOG(kTrace, "**** Error 0x%x returned by BCryptEncrypt", status);
			break;
		}

		// alloc output buffer to write
		pRetBuff = (PBYTE)HeapAlloc(GetProcessHeap(), 0, iRetlen);
		if (NULL == pRetBuff)
		{
			SNLOG(kTrace, "**** memory allocation failed");
			break;
		}

		// Use the key to encrypt the plaintext buffer.
		// For block sized messages, block padding will add an extra block.
		if (!NT_SUCCESS(status = BCryptEncrypt(hKey, (PBYTE)input_data.data(), input_data.length(), NULL, pbIV, pbIV ? cbBlockLen : 0, pRetBuff, iRetlen, &iRetlen, pbIV ? BCRYPT_BLOCK_PADDING : 0)))
		{
			SNLOG(kTrace, "**** Error 0x%x returned by BCryptEncrypt", status);
			break;
		}

		out_data.assign((char*)pRetBuff, iRetlen);

	} while (0);

	if (hAlg) BCryptCloseAlgorithmProvider(hAlg, 0);
	if (hKey) BCryptDestroyKey(hKey);
	if (pRetBuff) HeapFree(GetProcessHeap(), 0, pRetBuff);
	if (pbIV) HeapFree(GetProcessHeap(), 0, pbIV);
	if (pbKey) HeapFree(GetProcessHeap(), 0, pbKey);

	return NT_SUCCESS(status);
}

bool WinDecrypt(const std::string& input_data, const std::string& key, const std::string& iv, std::string& out_data, LPCWSTR ALG_TYPE)
{
	BCRYPT_ALG_HANDLE       hAlg = NULL;
	BCRYPT_KEY_HANDLE       hKey = NULL;
	NTSTATUS                status = STATUS_UNSUCCESSFUL;
	DWORD					cbBlockLen(0), iRetlen(0);
	PBYTE					pbIV(NULL), pbKey(NULL);
	PBYTE					pRetBuff(NULL);

	do
	{
		if (!NT_SUCCESS(status = InitCrypt(key, iv, hAlg, hKey, cbBlockLen, pbIV, pbKey, ALG_TYPE)))
		{
			SNLOG(kTrace, "**** Error 0x%x returned by InitCrypt", status);
			break;
		}

		// Get the output buffer size.
		if (!NT_SUCCESS(status = BCryptDecrypt(hKey, (PBYTE)input_data.data(), input_data.length(), NULL, pbIV, cbBlockLen, NULL, 0, &iRetlen, BCRYPT_BLOCK_PADDING)))
		{
			SNLOG(kTrace, "**** Error 0x%x returned by BCryptDecrypt", status);
			break;
		}

		pRetBuff = (PBYTE)HeapAlloc(GetProcessHeap(), 0, iRetlen);
		if (NULL == pRetBuff)
		{
			SNLOG(kTrace, "**** memory allocation failed");
			break;
		}

		if (!NT_SUCCESS(status = BCryptDecrypt(hKey, (PBYTE)input_data.data(), input_data.length(), NULL, pbIV, cbBlockLen, pRetBuff, iRetlen, &iRetlen, BCRYPT_BLOCK_PADDING)))
		{
			SNLOG(kTrace, "**** Error 0x%x returned by BCryptDecrypt", status);
			break;
		}

		out_data.assign((char*)pRetBuff, iRetlen);

	} while (0);

	if (hAlg) BCryptCloseAlgorithmProvider(hAlg, 0);
	if (hKey) BCryptDestroyKey(hKey);
	if (pRetBuff) HeapFree(GetProcessHeap(), 0, pRetBuff);
	if (pbIV) HeapFree(GetProcessHeap(), 0, pbIV);
	if (pbKey) HeapFree(GetProcessHeap(), 0, pbKey);

	return NT_SUCCESS(status);
}


}

bool WinAESEncrypt(const std::string& input_data, const std::string& key, const std::string& iv, std::string& out_data)
{
	return detail::WinEncrypt(input_data, key, iv, out_data, BCRYPT_AES_ALGORITHM);
}

bool WinAESDecrypt(const std::string& input_data, const std::string& key, const std::string& iv, std::string& out_data)
{
	return detail::WinDecrypt(input_data, key, iv, out_data, BCRYPT_AES_ALGORITHM);
}

bool WinDESEncrypt(const std::string& input_data, const std::string& key, const std::string& iv, std::string& out_data)
{
	return detail::WinEncrypt(input_data, key, iv, out_data, BCRYPT_DES_ALGORITHM);
}

bool WinDESDecrypt(const std::string& input_data, const std::string& key, const std::string& iv, std::string& out_data)
{
	return detail::WinDecrypt(input_data, key, iv, out_data, BCRYPT_DES_ALGORITHM);
}

bool WinRC4Encrypt(const std::string& input_data, const std::string& key, std::string& out_data)
{
	return detail::WinEncrypt(input_data, key, "", out_data, BCRYPT_RC4_ALGORITHM);
}

bool WinRC4Decrypt(const std::string& input_data, const std::string& key, std::string& out_data)
{
	return detail::WinDecrypt(input_data, key, "", out_data, BCRYPT_RC4_ALGORITHM);
}


std::string WinMD5(const std::string& input)
{
	return detail::WinHash(input, BCRYPT_MD5_ALGORITHM);
}

std::string WinSHA1(const std::string& input)
{
	return detail::WinHash(input, BCRYPT_SHA1_ALGORITHM);
}

std::string WinSHA256(const std::string& input)
{
	return detail::WinHash(input, BCRYPT_SHA256_ALGORITHM);
}

std::string WinBase64(const std::string& input)
{
	std::string ret_data;
	DWORD base64Length = 0;

	DWORD keyMaterialLength;
	LPBYTE keyMaterial;

	keyMaterialLength = input.length();
	keyMaterial = (LPBYTE)input.data();

	if (!CryptBinaryToStringA(keyMaterial, keyMaterialLength, CRYPT_STRING_BASE64, NULL, &base64Length))
	{
		SNLOG(kTrace, "Error %x during GetExportedKey!", GetLastError());
		return ret_data;
	}
	ret_data.resize(base64Length, 0);
	if (!CryptBinaryToStringA(keyMaterial, keyMaterialLength, CRYPT_STRING_BASE64, (LPSTR)ret_data.data(), &base64Length))
	{
		SNLOG(kTrace, "Error %x during GetExportedKey!", GetLastError());
	}

	return ret_data;
}

}