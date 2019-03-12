#include <sqnet/sqcrypthelper.h>
#include <sqwin/win/sqtools.h>
#include <vmp/sqvmsdk.h>
#include <sqsafe/sqsafemodel.h>

namespace snqu{ namespace net2{ 
	static unsigned char __the_simple_key[] = {
		0x14, 0x39, 0x28, 0x3e, 0xab, 0x4f, 0x3c, 0x3d, 
		0x16, 0xf1, 0x2e, 0x3f, 0xcc, 0xcc, 0x21, 0x08
	};

SQEncryptHelper::SQEncryptHelper() {}
SQEncryptHelper::~SQEncryptHelper() {}

static
VOID 
NetMapEncryKey(
	LPBYTE lpDeCodeBuffer, 
	LPBYTE lpKeyBuffer, 
	INT nSize
	)
{
	_VMProtectBegin(__FUNCTION__);
	BYTE KeyMap[0x100] = {0}; 
	INT  i = 0;
	BYTE cbTmp = 0;
	BYTE cbIdx = 0;

	for (i = 0;i < 0x100; i++)
	{  
		lpDeCodeBuffer[i] = i;
		KeyMap[i] = lpKeyBuffer[i % nSize];            
	}


	for (i = 0; i < 0x100; i++)
	{
		cbIdx = lpDeCodeBuffer[i] + cbIdx + KeyMap[i];
		cbTmp = lpDeCodeBuffer[i];
		lpDeCodeBuffer[i] = lpDeCodeBuffer[cbIdx];
		lpDeCodeBuffer[cbIdx] = cbTmp; 
	}

	_VMProtectEnd();
}

static
VOID 
WINAPI 
NetDeCodeBuffer(
	LPBYTE lpDeCodeBuffer,
	LPBYTE lpKeyBuffer,
	INT nSize,
	LPBYTE lpNewBuffer
	)
{
	_VMProtectBegin(__FUNCTION__);
	BYTE    cnIdx = 0; 
	BYTE    n = 0; 
	BYTE    m = 0; 
	BYTE    nTmp = 0;
	INT     i  = 0;


	for (i = 0 ; i< nSize; i++)
	{
		cnIdx = i + 1;   
		n = lpDeCodeBuffer[cnIdx];
		m += n;

		lpDeCodeBuffer[cnIdx] = lpDeCodeBuffer[m];
		lpDeCodeBuffer[m] = n;
		nTmp = n + lpDeCodeBuffer[cnIdx];
		lpNewBuffer[i] = lpDeCodeBuffer[nTmp] ^ lpKeyBuffer[i];
	}

	RtlZeroMemory(lpDeCodeBuffer,0x100);
	_VMProtectEnd();
}

static
VOID 
NetSimpleCryptData(
	__in LPBYTE lpHashKey,
	__in INT nKeySize,
	__inout LPBYTE lpDataBuffer,
	__in INT nDataSize
	)
{
	_VMProtectBegin(__FUNCTION__);
	BYTE   KeyBuffer[0x100] = {0};
	LPBYTE lpTmpBuffer = NULL;

	lpTmpBuffer = (LPBYTE)LocalAlloc(LPTR,nDataSize);
	if (NULL != lpTmpBuffer)
	{
		if (nKeySize > 0x100) nKeySize = 0x100;
		NetMapEncryKey(KeyBuffer,lpHashKey, nKeySize);
		NetDeCodeBuffer(KeyBuffer, lpDataBuffer,nDataSize,lpTmpBuffer);
		memcpy(lpDataBuffer,lpTmpBuffer,nDataSize);

		LocalFree((HLOCAL)lpTmpBuffer);	
	}

	_VMProtectEnd();
}

void SQEncryptHelper::generate_random_key(string& keyout, uint32 keysize)
{
	_VMProtectBegin(__FUNCTION__);

	srand((uint32)GetTickCount());
	for (uint32 i = 0; i < keysize; i++)
	{
		keyout[i] = (uint8)(rand() % 0xfe);
	}
	_VMProtectEnd();
}


void SQEncryptHelper::get_simple_encrypt_key(string& keyout)
{
	_VMProtectBegin(__FUNCTION__);
	keyout.resize(sizeof(__the_simple_key));
	for (size_t i = 0; i < sizeof(__the_simple_key); i++)
	{
		keyout[i] = __the_simple_key[i];
	}
	_VMProtectEnd();
}

void SQEncryptHelper::simple_encrypt_key(string& data, uint32 size)
{
	_VMProtectBegin(__FUNCTION__);
	auto asize = size > 0 ? size : data.size();
	NetSimpleCryptData(__the_simple_key, sizeof(__the_simple_key), 
                        reinterpret_cast<uint8*>(&data[0]), asize);
	_VMProtectEnd();
}


void SQEncryptHelper::simple_encrypt(string& data, const string& key, uint32 keysize)
{
	_VMProtectBegin(__FUNCTION__);
	NetSimpleCryptData((uint8*)&key[0], keysize, (uint8*)&data[0], data.size());
	_VMProtectEnd();
}

void SQEncryptHelper::simple_encrypt(char *data, uint32 size, const string& key, uint32 keysize)
{
	_VMProtectBegin(__FUNCTION__);
	NetSimpleCryptData((uint8*)&key[0], keysize, (uint8*)data, size);
	_VMProtectEnd();
}

uint16_t SQEncryptHelper::checksum(const char *buf, unsigned int size)
{
	unsigned long long sum = 0;
	const unsigned long long *b = (unsigned long long *) buf;

	unsigned t1, t2;
	unsigned short t3, t4;

	/* Main loop - 8 bytes at a time */
	while (size >= sizeof(unsigned long long))
	{
		unsigned long long s = *b++;
		sum += s;
		if (sum < s) sum++;
		size -= 8;
	}

	/* Handle tail less than 8-bytes long */
	buf = (const char *) b;
	if (size & 4)
	{
		unsigned s = *(unsigned *)buf;
		sum += s;
		if (sum < s) sum++;
		buf += 4;
	}

	if (size & 2)
	{
		unsigned short s = *(unsigned short *) buf;
		sum += s;
		if (sum < s) sum++;
		buf += 2;
	}

	if (size)
	{
		unsigned char s = *(unsigned char *) buf;
		sum += s;
		if (sum < s) sum++;
	}

	/* Fold down to 16 bits */
	t1 = (unsigned int)sum;
	t2 = sum >> 32;
	t1 += t2;
	if (t1 < t2) t1++;
	t3 = t1;
	t4 = t1 >> 16;
	t3 += t4;
	if (t3 < t4) t3++;

	return ~t3;
}
}}