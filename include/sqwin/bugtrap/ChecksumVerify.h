
#define CHECKSUM_SUCCESS            0
#define CHECKSUM_OPEN_FAILURE       1
#define CHECKSUM_MAP_FAILURE        2
#define CHECKSUM_MAPVIEW_FAILURE    3
#define CHECKSUM_UNICODE_FAILURE    4


/***********************************************************************
*		CheckSum (internal)
*/
__inline WORD CalcCheckSum(
	DWORD StartValue, LPVOID BaseAddress, DWORD WordCount)
{
	LPWORD Ptr;
	DWORD Sum;
	DWORD i;

	Sum = StartValue;
	Ptr = (LPWORD)BaseAddress;
	for (i = 0; i < WordCount; i++)
	{
		Sum += *Ptr;
		if (HIWORD(Sum) != 0)
		{
			Sum = LOWORD(Sum) + HIWORD(Sum);
		}
		Ptr++;
	}

	return (WORD)(LOWORD(Sum) + HIWORD(Sum));
}


/***********************************************************************
*		xxxCheckSumMappedFile (IMAGEHLP.@)
*/
__inline PIMAGE_NT_HEADERS WINAPI xxxCheckSumMappedFile(
	LPVOID BaseAddress, DWORD FileLength,
	LPDWORD HeaderSum, LPDWORD CheckSum)
{
	IMAGE_DOS_HEADER *dos = (IMAGE_DOS_HEADER *) BaseAddress;
	PIMAGE_NT_HEADERS32 Header32;
	PIMAGE_NT_HEADERS64 Header64;
	DWORD *ChecksumFile;
	DWORD CalcSum;
	DWORD HdrSum;

	CalcSum = (DWORD)CalcCheckSum(0,
		BaseAddress,
		(FileLength + 1) / sizeof(WORD));

	if (dos->e_magic != IMAGE_DOS_SIGNATURE)
		return NULL;

	Header32 = (IMAGE_NT_HEADERS32 *)((char *)dos + dos->e_lfanew);

	if (Header32->Signature != IMAGE_NT_SIGNATURE)
		return NULL;

	if (Header32->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC)
		ChecksumFile = &Header32->OptionalHeader.CheckSum;
	else if (Header32->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC)
	{
		Header64 = (IMAGE_NT_HEADERS64 *)Header32;
		ChecksumFile = &Header64->OptionalHeader.CheckSum;
	}
	else
		return NULL;

	HdrSum = *ChecksumFile;

	/* Subtract image checksum from calculated checksum. */
	/* fix low word of checksum */
	if (LOWORD(CalcSum) >= LOWORD(HdrSum))
	{
		CalcSum -= LOWORD(HdrSum);
	}
	else
	{
		CalcSum = ((LOWORD(CalcSum) - LOWORD(HdrSum)) & 0xFFFF) - 1;
	}

	/* fix high word of checksum */
	if (LOWORD(CalcSum) >= HIWORD(HdrSum))
	{
		CalcSum -= HIWORD(HdrSum);
	}
	else
	{
		CalcSum = ((LOWORD(CalcSum) - HIWORD(HdrSum)) & 0xFFFF) - 1;
	}

	/* add file length */
	CalcSum += FileLength;

	*CheckSum = CalcSum;
	*HeaderSum = *ChecksumFile;

	return (PIMAGE_NT_HEADERS) Header32;
}

/***********************************************************************
*		MapFileAndCheckSumW (IMAGEHLP.@)
*/
__inline DWORD WINAPI xxxMapFileAndCheckSumW(
	PCWSTR Filename, PDWORD HeaderSum, PDWORD CheckSum)
{
	HANDLE hFile;
	HANDLE hMapping;
	LPVOID BaseAddress;
	DWORD FileLength;

	hFile = CreateFileW(Filename,
		GENERIC_READ,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		0);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		return CHECKSUM_OPEN_FAILURE;
	}

	hMapping = CreateFileMappingW(hFile,
		NULL,
		PAGE_READONLY,
		0,
		0,
		NULL);
	if (hMapping == NULL)
	{
		CloseHandle(hFile);
		return CHECKSUM_MAP_FAILURE;
	}

	BaseAddress = MapViewOfFile(hMapping,
		FILE_MAP_READ,
		0,
		0,
		0);
	if (BaseAddress == 0)
	{
		CloseHandle(hMapping);
		CloseHandle(hFile);
		return CHECKSUM_MAPVIEW_FAILURE;
	}

	FileLength = GetFileSize(hFile,
		NULL);

	xxxCheckSumMappedFile(BaseAddress,
		FileLength,
		HeaderSum,
		CheckSum);

	UnmapViewOfFile(BaseAddress);
	CloseHandle(hMapping);
	CloseHandle(hFile);

	return CHECKSUM_SUCCESS;
}

