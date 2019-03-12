#include <sqsafe/detail/WinCrypto.h>
#include <sqwin/sqwin.h>
#include <bcrypt.h>
#include <sqlog/sqlog.h>


#pragma comment(lib, "Bcrypt.lib")

namespace snqu {

#define NT_SUCCESS(Status)          (((NTSTATUS)(Status)) >= 0)
#define STATUS_UNSUCCESSFUL         ((NTSTATUS)0xC0000001L)

int InitCrypt(const std::string& key, const std::string& iv, BCRYPT_ALG_HANDLE& hAesAlg, BCRYPT_KEY_HANDLE& hKey, DWORD& cbBlockLen, PBYTE& pbIV)
{
	NTSTATUS                status = STATUS_UNSUCCESSFUL;
	DWORD                   cbData(0), cbKeyObject(0),
		cbBlob(0);
	PBYTE					pbKeyObject = NULL,
		pbBlob = NULL;

	do
	{
		// Open an algorithm handle.
		if (!NT_SUCCESS(status = BCryptOpenAlgorithmProvider(&hAesAlg, BCRYPT_AES_ALGORITHM, NULL, 0)))
		{
			SNLOG(kTrace, "**** Error 0x%x returned by BCryptOpenAlgorithmProvider\n", status);
			break;
		}

		// Calculate the size of the buffer to hold the KeyObject.
		if (!NT_SUCCESS(status = BCryptGetProperty(hAesAlg, BCRYPT_OBJECT_LENGTH, (PBYTE)&cbKeyObject, sizeof(DWORD), &cbData, 0)))
		{
			SNLOG(kTrace, "**** Error 0x%x returned by BCryptGetProperty\n", status);
			break;
		}

		// Allocate the key object on the heap.
		pbKeyObject = (PBYTE)HeapAlloc(GetProcessHeap(), 0, cbKeyObject);
		if (NULL == pbKeyObject)
		{
			SNLOG(kTrace, "**** memory allocation failed\n");
			break;
		}

		// Calculate the block length for the IV.
		if (!NT_SUCCESS(status = BCryptGetProperty(hAesAlg, BCRYPT_BLOCK_LENGTH, (PBYTE)&cbBlockLen, sizeof(DWORD), &cbData, 0)))
		{
			SNLOG(kTrace, "**** Error 0x%x returned by BCryptGetProperty\n", status);
			break;
		}

		// Determine whether the cbBlockLen is not longer than the IV length.
		if (cbBlockLen > iv.length())
		{
			SNLOG(kTrace, "**** block length is longer than the provided IV length\n");
			break;
		}

		// Allocate a buffer for the IV. The buffer is consumed during the encrypt/decrypt process.
		pbIV = (PBYTE)HeapAlloc(GetProcessHeap(), 0, cbBlockLen);
		if (NULL == pbIV)
		{
			SNLOG(kTrace, "**** memory allocation failed\n");
			break;
		}
		memset(pbIV, 0, cbBlockLen);
		memcpy_s(pbIV, cbBlockLen, iv.data(), iv.length());
		if (!NT_SUCCESS(status = BCryptSetProperty(hAesAlg, BCRYPT_CHAINING_MODE, (PBYTE)BCRYPT_CHAIN_MODE_CBC, sizeof(BCRYPT_CHAIN_MODE_CBC), 0)))
		{
			SNLOG(kTrace, "**** Error 0x%x returned by BCryptSetProperty\n", status);
			break;
		}

		// Generate the key from supplied input key bytes.
		if (!NT_SUCCESS(status = BCryptGenerateSymmetricKey(hAesAlg, &hKey, pbKeyObject, cbKeyObject, (PBYTE)key.data(), key.length(), 0)))
		{
			SNLOG(kTrace, "**** Error 0x%x returned by BCryptGenerateSymmetricKey\n", status);
			break;
		}

		// Save another copy of the key for later.
		if (!NT_SUCCESS(status = BCryptExportKey(hKey, NULL, BCRYPT_OPAQUE_KEY_BLOB, NULL, 0, &cbBlob, 0)))
		{
			SNLOG(kTrace, "**** Error 0x%x returned by BCryptExportKey\n", status);
			break;
		}

		// Allocate the buffer to hold the BLOB.
		pbBlob = (PBYTE)HeapAlloc(GetProcessHeap(), 0, cbBlob);
		if (NULL == pbBlob)
		{
			SNLOG(kTrace, "**** memory allocation failed\n");
			break;
		}
		if (!NT_SUCCESS(status = BCryptExportKey(hKey, NULL, BCRYPT_OPAQUE_KEY_BLOB, pbBlob, cbBlob, &cbBlob, 0)))
		{
			SNLOG(kTrace, "**** Error 0x%x returned by BCryptExportKey\n", status);
			break;
		}

	} while (0);

	if (pbKeyObject) HeapFree(GetProcessHeap(), 0, pbKeyObject);
	if (pbBlob) HeapFree(GetProcessHeap(), 0, pbBlob);

	return status;
}


bool WinEncrypt(const std::string& input_data, const std::string& key, const std::string& iv, std::string& out_data)
{
	BCRYPT_ALG_HANDLE       hAesAlg = NULL;
	BCRYPT_KEY_HANDLE       hKey = NULL;
	NTSTATUS                status = STATUS_UNSUCCESSFUL;
	DWORD					cbBlockLen(0), iRetlen(0);
	PBYTE					pbIV(NULL);
	PBYTE					pRetBuff(NULL);
	do
	{
		if (!NT_SUCCESS(status = InitCrypt(key, iv, hAesAlg, hKey, cbBlockLen, pbIV)))
		{
			SNLOG(kTrace, "**** Error 0x%x returned by InitCrypt\n", status);
			break;
		}

		// Get the output buffer size.
		if (!NT_SUCCESS(status = BCryptEncrypt(hKey, (PBYTE)input_data.data(), input_data.length(), NULL, pbIV, cbBlockLen, NULL, 0, &iRetlen, BCRYPT_BLOCK_PADDING)))
		{
			SNLOG(kTrace, "**** Error 0x%x returned by BCryptEncrypt\n", status);
			break;
		}

		// alloc output buffer to write
		pRetBuff = (PBYTE)HeapAlloc(GetProcessHeap(), 0, iRetlen);
		if (NULL == pRetBuff)
		{
			SNLOG(kTrace, "**** memory allocation failed\n");
			break;
		}

		// Use the key to encrypt the plaintext buffer.
		// For block sized messages, block padding will add an extra block.
		if (!NT_SUCCESS(status = BCryptEncrypt(hKey, (PBYTE)input_data.data(), input_data.length(), NULL, pbIV, cbBlockLen, pRetBuff, iRetlen, &iRetlen, BCRYPT_BLOCK_PADDING)))
		{
			SNLOG(kTrace, "**** Error 0x%x returned by BCryptEncrypt\n", status);
			break;
		}
		out_data.assign((char*)pRetBuff, iRetlen);

	} while (0);

	if (hAesAlg) BCryptCloseAlgorithmProvider(hAesAlg, 0);
	if (hKey) BCryptDestroyKey(hKey);
	if (pRetBuff) HeapFree(GetProcessHeap(), 0, pRetBuff);
	if (pbIV) HeapFree(GetProcessHeap(), 0, pbIV);

	return NT_SUCCESS(status);
}

bool WinDecrypt(const std::string& input_data, const std::string& key, const std::string& iv, std::string& out_data)
{
	BCRYPT_ALG_HANDLE       hAesAlg = NULL;
	BCRYPT_KEY_HANDLE       hKey = NULL;
	NTSTATUS                status = STATUS_UNSUCCESSFUL;
	DWORD					cbBlockLen(0), iRetlen(0);
	PBYTE					pbIV(NULL);
	PBYTE					pRetBuff(NULL);

	do
	{
		if (!NT_SUCCESS(status = InitCrypt(key, iv, hAesAlg, hKey, cbBlockLen, pbIV)))
		{
			SNLOG(kTrace, "**** Error 0x%x returned by InitCrypt\n", status);
			break;
		}

		// Reinitialize the IV because encryption would have modified it.
		memset(pbIV, 0, cbBlockLen);
		memcpy_s(pbIV, cbBlockLen, iv.data(), iv.length());

		// Get the output buffer size.
		if (!NT_SUCCESS(status = BCryptDecrypt(hKey, (PBYTE)input_data.data(), input_data.length(), NULL, pbIV, cbBlockLen, NULL, 0, &iRetlen, BCRYPT_BLOCK_PADDING)))
		{
			SNLOG(kTrace, "**** Error 0x%x returned by BCryptDecrypt\n", status);
			break;
		}

		pRetBuff = (PBYTE)HeapAlloc(GetProcessHeap(), 0, iRetlen);
		if (NULL == pRetBuff)
		{
			SNLOG(kTrace, "**** memory allocation failed\n");
			break;
		}

		if (!NT_SUCCESS(status = BCryptDecrypt(hKey, (PBYTE)input_data.data(), input_data.length(), NULL, pbIV, cbBlockLen, pRetBuff, iRetlen, &iRetlen, BCRYPT_BLOCK_PADDING)))
		{
			SNLOG(kTrace, "**** Error 0x%x returned by BCryptDecrypt\n", status);
			break;
		}

		out_data.assign((char*)pRetBuff, iRetlen);

	} while (0);

	if (hAesAlg) BCryptCloseAlgorithmProvider(hAesAlg, 0);
	if (hKey) BCryptDestroyKey(hKey);
	if (pRetBuff) HeapFree(GetProcessHeap(), 0, pRetBuff);
	if (pbIV) HeapFree(GetProcessHeap(), 0, pbIV);

	return NT_SUCCESS(status);
}

}