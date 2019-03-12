#pragma once
#include <stdio.h>
#include <string>

using namespace std;
#include <tchar.h>

#include <wintrust.h>
#include <SoftPub.h>
#include <Mscat.h>
#include "ChecksumVerify.h"

#pragma comment(lib, "crypt32.lib")
#pragma comment(lib, "Wintrust.lib")

#include <Shlwapi.h>
#pragma comment(lib,"shlwapi.lib")


class SIGNATUREINFO 
{
public:
	SIGNATUREINFO()
	{
		SignerSerialNumber=NULL;
		szSignerIssuerName=NULL;
		szSignerSubjectName=NULL;

	}
	~SIGNATUREINFO()
	{
		if(SignerSerialNumber)
			free(SignerSerialNumber);
		if(szSignerIssuerName)
			free(szSignerIssuerName);
		if(szSignerSubjectName)
			free(szSignerSubjectName);
	}
public:
	wstring szProgramName;
	wstring szPublisherLink;
	wstring szMoreInfoLink;

	//签名人证书(Signer certificate)
	PBYTE SignerSerialNumber;
	WCHAR* szSignerIssuerName;
	WCHAR* szSignerSubjectName;

};

class CQueryCert
{
private:

	BOOL InitCertificateInfo(HCERTSTORE hStore,PCMSG_SIGNER_INFO pSignerInfo,
		PBYTE *ppSerialNumber,
		LPWSTR* ppIssuerName,
		LPWSTR* ppSubjectName)
	{
		if (NULL == hStore || NULL == pSignerInfo)
			return FALSE;

		BOOL bOK=FALSE;
		LPWSTR IssuerName=NULL;
		LPWSTR SubjectName=NULL;
		PBYTE SerialNumber=NULL;

		CERT_INFO CertInfo;
		CertInfo.Issuer = pSignerInfo->Issuer;
		CertInfo.SerialNumber = pSignerInfo->SerialNumber;
		//根据颁发人和序列号在证书池中找到相应证书
		PCCERT_CONTEXT pCertContext = CertFindCertificateInStore(hStore,
			X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
			0,
			CERT_FIND_SUBJECT_CERT,
			(PVOID)&CertInfo,
			NULL);
		if (!pCertContext)
		{
			return FALSE;
		}

		//获取序列号
		DWORD dwDataSize = pCertContext->pCertInfo->SerialNumber.cbData;
		SerialNumber=(PBYTE)malloc(dwDataSize);
		if(!SerialNumber)
		{
			goto Clean;
		}
		for (DWORD i = 0; i < dwDataSize; i++)
		{
			SerialNumber[i] = pCertContext->pCertInfo->SerialNumber.pbData[dwDataSize - (i + 1)]; 
		}

		DWORD dwIssuerNameSize;
		//获取颁发人字符长度
		dwIssuerNameSize = CertGetNameStringW(pCertContext,CERT_NAME_SIMPLE_DISPLAY_TYPE,CERT_NAME_ISSUER_FLAG,NULL,NULL,0);
		if (!dwIssuerNameSize)
		{
			goto Clean;
		}

		IssuerName=(WCHAR*)malloc(dwIssuerNameSize*2);
		//returns the required size of the destination string (including the terminating NULL character). 
		if(!IssuerName)
		{
			goto Clean;
		}

		//获取颁发人字符
		dwIssuerNameSize = CertGetNameStringW(pCertContext,CERT_NAME_SIMPLE_DISPLAY_TYPE,CERT_NAME_ISSUER_FLAG,NULL,IssuerName,dwIssuerNameSize);
		if (!dwIssuerNameSize)
		{
			goto Clean;
		}

		DWORD dwSubjectNameSize;
		//获取签名人字符长度
		dwSubjectNameSize = CertGetNameStringW(pCertContext,CERT_NAME_SIMPLE_DISPLAY_TYPE,0,NULL,NULL,0);
		if (!dwSubjectNameSize)
		{
			goto Clean;
		}
		SubjectName=(WCHAR*)malloc(dwSubjectNameSize*2);
		//returns the required size of the destination string (including the terminating NULL character). 
		if(!SubjectName)
		{
			goto Clean;
		}

		//获取签名人字符
		dwSubjectNameSize = CertGetNameStringW(pCertContext,CERT_NAME_SIMPLE_DISPLAY_TYPE,0,NULL,SubjectName,dwSubjectNameSize);
		if (!dwSubjectNameSize)
		{
			goto Clean;
		}

		bOK=TRUE;

Clean:
		if(!bOK)
		{
			if(SubjectName)
				free(SubjectName);
			if(IssuerName)
				free(IssuerName);
			if(SerialNumber)
				free(SerialNumber);
		}
		else
		{
			*ppSubjectName=SubjectName;
			*ppIssuerName=IssuerName;
			*ppSerialNumber=SerialNumber;
		}
		return bOK;
	}

	BOOL InitProgramNameAndPublisherInfo(HCERTSTORE hStore,PCMSG_SIGNER_INFO pSignerInfo,SIGNATUREINFO &SigInfo)
	{
		//解析pSignerInfo结构并输出信息
		//循环查找SPC_SP_OPUS_INFO_OBJID  OID
		PSPC_SP_OPUS_INFO pOpusInfo = NULL;
		for (DWORD i = 0; i < pSignerInfo->AuthAttrs.cAttr; i++)
		{
			if (0 == lstrcmpA(SPC_SP_OPUS_INFO_OBJID,pSignerInfo->AuthAttrs.rgAttr[i].pszObjId))
			{
				DWORD dwDataSize=0;
				//获取SPC_SP_OPUS_INFO结构大小
				BOOL bRet = CryptDecodeObject(X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
					SPC_SP_OPUS_INFO_OBJID,
					pSignerInfo->AuthAttrs.rgAttr[i].rgValue[0].pbData,
					pSignerInfo->AuthAttrs.rgAttr[i].rgValue[0].cbData,
					0,
					NULL,
					&dwDataSize);
				if (!bRet)
				{
					//printf("CryptDecodeObject faild with %x\n",GetLastError());
					return FALSE;
				}
				//申请空间
				pOpusInfo = (PSPC_SP_OPUS_INFO)malloc(dwDataSize);
				if (!pOpusInfo)
				{
					//printf("malloc faild with %x\n",GetLastError());
					return FALSE;
				}
				//解码
				bRet = CryptDecodeObject(X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
					SPC_SP_OPUS_INFO_OBJID,
					pSignerInfo->AuthAttrs.rgAttr[i].rgValue[0].pbData,
					pSignerInfo->AuthAttrs.rgAttr[i].rgValue[0].cbData,
					0,
					pOpusInfo,
					&dwDataSize);
				if (!bRet)
				{
					//printf("CryptDecodeObject faild with %x\n",GetLastError());
					return FALSE;
				}

				//以下填充信息

				if(pOpusInfo->pwszProgramName)
					SigInfo.szProgramName=pOpusInfo->pwszProgramName;

				if (pOpusInfo->pPublisherInfo)
				{
					switch(pOpusInfo->pPublisherInfo->dwLinkChoice)
					{
					case SPC_URL_LINK_CHOICE:
						if(pOpusInfo->pPublisherInfo->pwszUrl)
							SigInfo.szPublisherLink=pOpusInfo->pPublisherInfo->pwszUrl;
						break;
					case SPC_FILE_LINK_CHOICE:
						if(pOpusInfo->pPublisherInfo->pwszFile)
							SigInfo.szPublisherLink=pOpusInfo->pPublisherInfo->pwszFile;
						break;
					}
				}

				if (pOpusInfo->pMoreInfo)
				{
					switch(pOpusInfo->pMoreInfo->dwLinkChoice)
					{
					case SPC_URL_LINK_CHOICE:
						if(pOpusInfo->pMoreInfo->pwszUrl)
							SigInfo.szMoreInfoLink=pOpusInfo->pMoreInfo->pwszUrl;
						break;
					case SPC_FILE_LINK_CHOICE:
						if(pOpusInfo->pMoreInfo->pwszFile)
							SigInfo.szMoreInfoLink=pOpusInfo->pMoreInfo->pwszFile;
						break;
					}
				}

				if (pOpusInfo != NULL)
					free(pOpusInfo);
				return TRUE;
			}
		}


		return FALSE;
	}

 
public:
	BOOL GetDigitalSignatureInfo(LPCWSTR pFilePath,SIGNATUREINFO &SigInfo)
	{
		DWORD dwEncoding,dwContentType,dwFormatType;
		HCERTSTORE hStore = NULL;
		HCRYPTMSG hMsg = NULL;
		BOOL bRet = FALSE;
		//获取消息句柄hMsg与签名文件的贮存句柄hStore,传进去的szFilePath必须为Unicode string
		bRet = CryptQueryObject(CERT_QUERY_OBJECT_FILE,
			pFilePath,
			CERT_QUERY_CONTENT_FLAG_PKCS7_SIGNED_EMBED,
			CERT_QUERY_FORMAT_FLAG_BINARY,
			0,
			&dwEncoding,
			&dwContentType,
			&dwFormatType,
			&hStore,
			&hMsg,
			NULL);
		if (!bRet)
		{
			return FALSE;
		}

		DWORD dwSignerInfoSize=0;
		//获取签名信息大小，以便申请足够的内存
		bRet = CryptMsgGetParam(hMsg,CMSG_SIGNER_INFO_PARAM,0,NULL,&dwSignerInfoSize);
		if (!bRet)
		{
			return FALSE;
		}
		//申请空间
		PCMSG_SIGNER_INFO pSignerInfo = (PCMSG_SIGNER_INFO)malloc(dwSignerInfoSize);
		if (!pSignerInfo)
		{
			return FALSE;
		}
		//获取签名信息
		bRet = CryptMsgGetParam(hMsg,CMSG_SIGNER_INFO_PARAM,0,(PVOID)pSignerInfo,&dwSignerInfoSize);
		if (!bRet)
		{
			free(pSignerInfo);
			return FALSE;
		}

		//获取发行人、网址等信息
		if (!InitProgramNameAndPublisherInfo(hStore,pSignerInfo,SigInfo))
		{
			free(pSignerInfo);
			return FALSE;
		}
		//以下获取签名人的证书信息
		if (!InitCertificateInfo(hStore,pSignerInfo,
			&SigInfo.SignerSerialNumber,
			&SigInfo.szSignerIssuerName,
			&SigInfo.szSignerSubjectName))
		{
			free(pSignerInfo);
			return FALSE;
		}
 
		free(pSignerInfo);

		return TRUE;
	}
};

__inline BOOL CheckFileTrust( LPCWSTR lpFileName )
{
	BOOL bRet = FALSE;
	WINTRUST_DATA wd = { 0 };
	WINTRUST_FILE_INFO wfi = { 0 };
	WINTRUST_CATALOG_INFO wci = { 0 };
	CATALOG_INFO ci = { 0 };

	HCATADMIN hCatAdmin = NULL;
	if ( !CryptCATAdminAcquireContext( &hCatAdmin, NULL, 0 ) )
	{
		return FALSE;
	}

	HANDLE hFile = CreateFileW( lpFileName, GENERIC_READ, FILE_SHARE_READ,
		NULL, OPEN_EXISTING, 0, NULL );
	if ( INVALID_HANDLE_VALUE == hFile )
	{
		CryptCATAdminReleaseContext( hCatAdmin, 0 );
		return FALSE;
	}

	DWORD dwCnt = 100;
	BYTE byHash[100];
	CryptCATAdminCalcHashFromFileHandle( hFile, &dwCnt, byHash, 0 );
	CloseHandle( hFile );

	LPWSTR pszMemberTag = new WCHAR[dwCnt * 2 + 1];
	for ( DWORD dw = 0; dw < dwCnt; ++dw )
	{
		wsprintfW( &pszMemberTag[dw * 2], L"%02X", byHash[dw] );
	}

	HCATINFO hCatInfo = CryptCATAdminEnumCatalogFromHash( hCatAdmin,
		byHash, dwCnt, 0, NULL );
	if ( NULL == hCatInfo )
	{
		wfi.cbStruct       = sizeof( WINTRUST_FILE_INFO );
		wfi.pcwszFilePath  = lpFileName;
		wfi.hFile          = NULL;
		wfi.pgKnownSubject = NULL;

		wd.cbStruct            = sizeof( WINTRUST_DATA );
		wd.dwUnionChoice       = WTD_CHOICE_FILE;
		wd.pFile               = &wfi;
		wd.dwUIChoice          = WTD_UI_NONE;
		wd.fdwRevocationChecks = WTD_REVOKE_NONE;
		wd.dwStateAction       = WTD_STATEACTION_IGNORE;
		wd.dwProvFlags         = WTD_SAFER_FLAG;
		wd.hWVTStateData       = NULL;
		wd.pwszURLReference    = NULL;
	}
	else
	{
		CryptCATCatalogInfoFromContext( hCatInfo, &ci, 0 );
		wci.cbStruct             = sizeof( WINTRUST_CATALOG_INFO );
		wci.pcwszCatalogFilePath = ci.wszCatalogFile;
		wci.pcwszMemberFilePath  = lpFileName;
		wci.pcwszMemberTag       = pszMemberTag;

		wd.cbStruct            = sizeof( WINTRUST_DATA );
		wd.dwUnionChoice       = WTD_CHOICE_CATALOG;
		wd.pCatalog            = &wci;
		wd.dwUIChoice          = WTD_UI_NONE;
		wd.fdwRevocationChecks = WTD_STATEACTION_VERIFY;
		wd.dwProvFlags         = 0;
		wd.hWVTStateData       = NULL;
		wd.pwszURLReference    = NULL;
	}
	GUID action = WINTRUST_ACTION_GENERIC_VERIFY_V2;
	HRESULT hr  = WinVerifyTrust( NULL, &action, &wd );
	bRet        = SUCCEEDED( hr );

	if ( NULL != hCatInfo )
	{
		CryptCATAdminReleaseCatalogContext( hCatAdmin, hCatInfo, 0 );
	}

	CryptCATAdminReleaseContext( hCatAdmin, 0 );

	delete[] pszMemberTag;
	return bRet;
} 


__inline BOOL CheckMySelf(LPCWSTR lpAppName,BOOL valid_digital = TRUE)
{
	if(valid_digital)
	{
		SIGNATUREINFO Info;
		CQueryCert QueryCert;
		if(QueryCert.GetDigitalSignatureInfo(lpAppName,Info))
		{
			if(wcscmp(Info.szSignerIssuerName,L"VeriSign Class 3 Code Signing 2010 CA")
				&& wcscmp(Info.szSignerIssuerName,L"WoSign Class 3 Code Signing CA"))
			{
				return FALSE;
			}

			if(wcscmp(Info.szSignerSubjectName,L"四川盛趣时代网络科技有限公司"))
			{
				return FALSE;
			}
		}
	}

	DWORD HeaderCheckSum = 0;
	DWORD CheckSum = 0;
	
	if(CHECKSUM_SUCCESS==xxxMapFileAndCheckSumW(lpAppName,&HeaderCheckSum,&CheckSum))
	{
		if (CheckSum != HeaderCheckSum)
		{
			return FALSE;
		}  
	}

	// 	if(!CheckFileTrust(lpAppName))
	// 	{
	// 		return FALSE;
	// 	}

	return TRUE;
}


__inline CHAR
	WINAPI
	HexChar2Char(
	__in CHAR ch
	)
{
	if ((ch>='0')&&(ch<='9'))
		return   ch-0x30;  
	else   if((ch>='A')&&(ch<='F'))  
		return   ch-'A'+10;  
	else   if((ch>='a')&&(ch<='f'))  
		return   ch-'a'+10;
	else   return   (-1);
}


// hexastext default is FALSE
__inline VOID
	WINAPI
	DecryptString(
	__in char *src, 
	__in int len, 
	__out char *out, 
	__in BOOL hexastext
	)
{
	int   i = 0, j = 0;
	BYTE  byascii_src = 0;
	BYTE  hbyte = 0, lbyte = 0;
	char* byte_src = NULL;
	char  cch = 0;
	BYTE  xorkey = 0x01;

	if (hexastext) {
		byte_src = (char *)malloc(len);
		for (i = 0, j = 0; i < len; i++) {
			cch = HexChar2Char(src[i]);
			if ((i % 2) == 0)
				hbyte = (BYTE)cch << 4;
			else {
				lbyte = (BYTE)cch;
				byte_src[j++] = (char )(hbyte | lbyte);
			}
		}
		src = byte_src;
		len = j;
	}

	for (i = 0; i < len; i++)  {
		byascii_src = (BYTE)src[i];
		// 11110000
		hbyte = (byascii_src & 0xf0) >> 4;
		// 00001111
		lbyte = (byascii_src & 0x0f) << 4;
		byascii_src = lbyte | hbyte;

		if (0x00 == xorkey) xorkey++;
		out[i] = (char)(byascii_src ^ (xorkey++));
	}

	if (hexastext && byte_src != NULL)
		free(byte_src);
}
