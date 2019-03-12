#include <sqwin/win/sqfilesignature.h>
#include <sqwin/sqwin.h>
#include <wincrypt.h>
#include <WinTrust.h>
#include <SoftPub.h>
#include <mscat.h>
#include <codec/sqcodec.h>

namespace snqu{ namespace os{

    bool self_check(PCERT_SIMPLE_CHAIN pcheck_data, const std::vector<std::string>& cert_chain_check)
    {
        int eidx = pcheck_data->cElement-1;
        char tmp[256] = {0};
        for (unsigned int eidx = 0; eidx < cert_chain_check.size(); eidx++)
        {
            CertGetNameString(
                pcheck_data->rgpElement[eidx]->pCertContext,
                CERT_NAME_SIMPLE_DISPLAY_TYPE, // 简单名字
                0, NULL, tmp, 256);

            if (0 != strcmp(cert_chain_check[eidx].c_str(), tmp))
            {
                return false;
            }
        }

        return true;
    }

    bool check_file_signature(const std::string& full_file_path, const std::vector<std::string>& cert_chain_check)
    {
        auto lpFileName = codec::S2W(full_file_path);
        bool bRet = false;
        WINTRUST_DATA wd = { 0 };
        WINTRUST_FILE_INFO wfi = { 0 };
        WINTRUST_CATALOG_INFO wci = { 0 };
        CATALOG_INFO ci = { 0 };

        HCATADMIN hCatAdmin = NULL;
        if (!CryptCATAdminAcquireContext(&hCatAdmin, NULL, 0))
        {
            return false;
        }

        HANDLE hFile = CreateFileW(lpFileName.c_str(), GENERIC_READ, FILE_SHARE_READ,
            NULL, OPEN_EXISTING, 0, NULL);
        if (INVALID_HANDLE_VALUE == hFile)
        {
            CryptCATAdminReleaseContext(hCatAdmin, 0);
            return false;
        }

        DWORD dwCnt = 100;
        BYTE byHash[100];
        CryptCATAdminCalcHashFromFileHandle(hFile, &dwCnt, byHash, 0);
        CloseHandle(hFile);

        wchar_t* pszMemberTag = new wchar_t[dwCnt * 2 + 1];
        for (DWORD dw = 0; dw < dwCnt; ++dw)
        {
            wsprintfW(&pszMemberTag[dw * 2], L"%02X", byHash[dw]);
        }

        HCATINFO hCatInfo = CryptCATAdminEnumCatalogFromHash(hCatAdmin,
            byHash, dwCnt, 0, NULL);
        if (NULL == hCatInfo)
        {
            wfi.cbStruct = sizeof(WINTRUST_FILE_INFO);
            wfi.pcwszFilePath = lpFileName.c_str();
            wfi.hFile = NULL;
            wfi.pgKnownSubject = NULL;

            wd.cbStruct = sizeof(WINTRUST_DATA);
            wd.dwUnionChoice = WTD_CHOICE_FILE;
            wd.pFile = &wfi;
            wd.dwUIChoice = WTD_UI_NONE;
            wd.fdwRevocationChecks = WTD_REVOKE_NONE;
            wd.dwStateAction = WTD_STATEACTION_VERIFY;
            wd.dwProvFlags = WTD_SAFER_FLAG;
            wd.hWVTStateData = NULL;
            wd.pwszURLReference = NULL;
        }
        else
        {
            CryptCATCatalogInfoFromContext( hCatInfo, &ci, 0 );
            wci.cbStruct = sizeof(WINTRUST_CATALOG_INFO);
            wci.pcwszCatalogFilePath = ci.wszCatalogFile;
            wci.pcwszMemberFilePath = lpFileName.c_str();
            wci.pcwszMemberTag = pszMemberTag;

            wd.cbStruct = sizeof(WINTRUST_DATA);
            wd.dwUnionChoice = WTD_CHOICE_CATALOG;
            wd.pCatalog = &wci;
            wd.dwUIChoice = WTD_UI_NONE;
            wd.fdwRevocationChecks = WTD_STATEACTION_VERIFY;
            wd.dwProvFlags = 0;
            wd.hWVTStateData = NULL;
            wd.pwszURLReference = NULL;
        }
        GUID action = WINTRUST_ACTION_GENERIC_VERIFY_V2;
        HRESULT hr = WinVerifyTrust(NULL, &action, &wd);
        bRet = SUCCEEDED(hr);

        if ( NULL != hCatInfo )
        {
            CryptCATAdminReleaseCatalogContext(hCatAdmin, hCatInfo, 0);
        }
        CryptCATAdminReleaseContext(hCatAdmin, 0); 

        // 签名数据信息
        auto provData = WTHelperProvDataFromStateData(wd.hWVTStateData);

        if ((provData != NULL) && (provData->pasSigners != NULL))
        {
            if (!cert_chain_check.empty() && 
                !self_check(provData->pasSigners->pChainContext->rgpChain[0], cert_chain_check))
                return false;
        }

        // 释放
        wd.dwStateAction = WTD_STATEACTION_CLOSE;
        WinVerifyTrust(NULL, &action, &wd);

        delete[] pszMemberTag;
        return bRet;
    }
}
}