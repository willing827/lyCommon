#include <sqwin/win/WSSLCert.h>
#include <sqwin/sqwin.h>
#include <cryptuiapi.h>
#include <sqlog/sqlog.h>

#pragma comment(lib, "Crypt32.lib")
#pragma comment(lib, "Cryptui.lib")

namespace snqu{ namespace cert {

bool ImportPFXData(const std::string& cert_data, const std::wstring& cert_pwd, const std::wstring& common_name)
{
	DWORD   dwErr = 0;
	CRYPT_DATA_BLOB pPFX;
	pPFX.cbData = cert_data.length();
	pPFX.pbData = (BYTE*)&cert_data[0];
	if (!PFXIsPFXBlob(&pPFX))
	{
		dwErr = GetLastError();
		SNLOG(kError, "Failed PFXIsPFXBlob error:0x%x\n", dwErr);
		return false;
	}

	HCERTSTORE hPfxCertStore = NULL;
	PCCERT_CONTEXT pCertContext = NULL;
	HCERTSTORE hSystemStore = NULL;

	do
	{
		hPfxCertStore = PFXImportCertStore(&pPFX, cert_pwd.c_str(), CRYPT_EXPORTABLE | CRYPT_MACHINE_KEYSET | CRYPT_DELETEKEYSET);
		if (hPfxCertStore == NULL)
		{
			dwErr = GetLastError();
			SNLOG(kError, "Failed PFXImportCertStore error:0x%x\n", dwErr);
			break;
		}

		// 查找临时证书库中导入的证书
		const WCHAR* pszCommonName = common_name.c_str();
		CERT_RDN_ATTR certRDNAttr[1];
		certRDNAttr[0].pszObjId = (LPSTR)szOID_COMMON_NAME;
		certRDNAttr[0].dwValueType = CERT_RDN_ANY_TYPE;
		certRDNAttr[0].Value.pbData = (PBYTE)pszCommonName;
		certRDNAttr[0].Value.cbData = wcslen(pszCommonName) * 2;
		CERT_RDN certRDN = { 1, certRDNAttr };
		pCertContext = CertFindCertificateInStore(hPfxCertStore,
			X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
			CERT_UNICODE_IS_RDN_ATTRS_FLAG,
			CERT_FIND_SUBJECT_ATTR, &certRDN, pCertContext);

		if (pCertContext == NULL)
		{
			dwErr = GetLastError();
			SNLOG(kError, "Failed CertFindCertificateInStore error:0x%x\n", dwErr);
			break;
		}

		CRYPTUI_WIZ_IMPORT_SRC_INFO importSrc;
		memset(&importSrc, 0, sizeof(CRYPTUI_WIZ_IMPORT_SRC_INFO));
		importSrc.dwSize = sizeof(CRYPTUI_WIZ_IMPORT_SRC_INFO);
		importSrc.dwSubjectChoice = CRYPTUI_WIZ_IMPORT_SUBJECT_CERT_CONTEXT;
		importSrc.pCertContext = pCertContext;
		hSystemStore = CertOpenStore(CERT_STORE_PROV_SYSTEM,	// System store will be a virtual store
			0,                    // Encoding type not needed  with this PROV
			NULL,                 // Accept the default HCRYPTPROV
			CERT_SYSTEM_STORE_LOCAL_MACHINE, // Set the system store location in the registry
			L"MY");
		if (hSystemStore == NULL)
		{
			dwErr = GetLastError();
			SNLOG(kError, "Failed CertOpenStore error:0x%x\n", dwErr);
			break;
		}

		if (CryptUIWizImport(CRYPTUI_WIZ_NO_UI | CRYPTUI_WIZ_IMPORT_TO_LOCALMACHINE, NULL, NULL, &importSrc, hSystemStore) == 0)
		{
			dwErr = GetLastError();
			SNLOG(kError, "Failed CryptUIWizImport error:0x%x\n", dwErr);
			break;
		}

	} while (0);

	if(pCertContext) CertFreeCertificateContext(pCertContext);
	if (hPfxCertStore) CertCloseStore(hPfxCertStore, 0);
	if (hSystemStore) CertCloseStore(hSystemStore, 0);
	return dwErr == 0;
}


bool ImportCertFile(const std::wstring& cert_path, const std::wstring& cert_pwd)
{
	CRYPTUI_WIZ_IMPORT_SRC_INFO importSrc;
	memset(&importSrc, 0, sizeof(CRYPTUI_WIZ_IMPORT_SRC_INFO));
	importSrc.dwSize = sizeof(CRYPTUI_WIZ_IMPORT_SRC_INFO);
	importSrc.dwSubjectChoice = CRYPTUI_WIZ_IMPORT_SUBJECT_FILE;
	importSrc.pwszFileName = cert_path.c_str();
	importSrc.pwszPassword = cert_pwd.c_str();
	importSrc.dwFlags = CRYPT_EXPORTABLE;// | CRYPT_USER_PROTECTED;

	if (CryptUIWizImport(CRYPTUI_WIZ_NO_UI | CRYPTUI_WIZ_IMPORT_TO_LOCALMACHINE,
						 NULL,NULL,&importSrc,NULL) == 0)
	{
		auto dwErr = GetLastError();
		SNLOG(kError, "Failed CryptUIWizImport error:0x%x\n", dwErr);
		return false;
	}

	return true;
}

bool DeleteCert(const std::wstring& common_name)
{
	PCCERT_CONTEXT pCertContext = NULL;
	HCERTSTORE hSystemStore = NULL;
	DWORD   dwErr = 0;

	hSystemStore = CertOpenStore(CERT_STORE_PROV_SYSTEM,	// System store will be a virtual store
								0,                    // Encoding type not needed  with this PROV
								NULL,                 // Accept the default HCRYPTPROV
								CERT_SYSTEM_STORE_LOCAL_MACHINE, // Set the system store location in the registry
								L"MY");

	if (hSystemStore == NULL)
	{
		dwErr = GetLastError();
		SNLOG(kError, "Failed CertOpenSystemStore error:0x%x\n", dwErr);
		return false;
	}

	do 
	{
		// 查找系统个人证书库中导入的证书
		const WCHAR* pszCommonName = common_name.c_str();
		CERT_RDN_ATTR certRDNAttr[1];
		certRDNAttr[0].pszObjId = (LPSTR)szOID_COMMON_NAME;
		certRDNAttr[0].dwValueType = CERT_RDN_ANY_TYPE;
		certRDNAttr[0].Value.pbData = (PBYTE)pszCommonName;
		certRDNAttr[0].Value.cbData = wcslen(pszCommonName) * 2;
		CERT_RDN certRDN = { 1, certRDNAttr };
		pCertContext = CertFindCertificateInStore(hSystemStore,
			X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
			CERT_UNICODE_IS_RDN_ATTRS_FLAG,
			CERT_FIND_SUBJECT_ATTR, &certRDN, pCertContext);

		if (pCertContext == NULL)
		{
			dwErr = GetLastError();
			SNLOG(kError, "Failed CertFindCertificateInStore error:0x%x\n", dwErr);
			break;
		}

		if (!CertDeleteCertificateFromStore(pCertContext))
		{
			dwErr = GetLastError();
			SNLOG(kError, "Failed CertDeleteCertificateFromStore error:0x%x\n", dwErr);
			break;
		}

	} while (0);
	
	if (pCertContext) CertFreeCertificateContext(pCertContext);
	if (hSystemStore) CertCloseStore(hSystemStore, 0);
	return dwErr == 0;
}

}}