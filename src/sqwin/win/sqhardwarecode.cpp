#include <sqwin/win/sqhardwarecode.h>
#include <sqwin/win/sqwindows.h>
#include <comutil.h>  
#include <Wbemidl.h>  
#include <tchar.h>  
#include <strsafe.h>  
#include <algorithm>  
#include <atlconv.h>  
#include <ntddndis.h>  
#include <iostream>
#include <codec/sqcodec.h>
#include <sqwin/win/sqos.h>
#include <sqwin/win/sqwinnet.h>

#pragma comment (lib, "comsuppw.lib")  
#pragma comment (lib, "wbemuuid.lib")  

#define  PROPERTY_MAX_LEN 128

namespace snqu{
    namespace detail {

        enum QueryType
        {
            QueryNetDeviceID = 0,   //网卡原生MAC地址 
            QueryDiskSeriral,       //硬盘序列号 
            QueryBaseBoardSeriral,  //主板序列号 
            QueryProcessorId,       //CPU ID 
            QueryBiosSeriral,       //BIOS序列号 
            QueryBaseBoardProduct,  //主板型号 
            QueryMacAddress,        //网卡当前MAC地址 
        };

        typedef struct _T_WQL_QUERY  
        {  
            CHAR*   szSelect;       // SELECT语句  
            WCHAR*  szProperty;     // 属性字段  
        } T_WQL_QUERY;  

        const T_WQL_QUERY szWQLQuery[] = {  
            // 网卡原生MAC地址  
            "SELECT * FROM Win32_NetworkAdapter WHERE (MACAddress IS NOT NULL) AND (NOT (PNPDeviceID LIKE 'ROOT%'))",  
            L"PNPDeviceID",  

            // 硬盘序列号  
            "SELECT * FROM Win32_DiskDrive WHERE (SerialNumber IS NOT NULL) AND (MediaType LIKE 'Fixed hard disk%')",  
            L"SerialNumber",  

            // 主板序列号  
            "SELECT * FROM Win32_BaseBoard WHERE (SerialNumber IS NOT NULL)",  
            L"SerialNumber",      

            // 处理器ID  
            "SELECT * FROM Win32_Processor WHERE (ProcessorId IS NOT NULL)",  
            L"ProcessorId",  

            // BIOS序列号  
            "SELECT * FROM Win32_BIOS WHERE (SerialNumber IS NOT NULL)",  
            L"SerialNumber",  

            // 主板型号  
            "SELECT * FROM Win32_BaseBoard WHERE (Product IS NOT NULL)",  
            L"Product",  

            // 网卡当前MAC地址  
            "SELECT * FROM Win32_NetworkAdapter WHERE (MACAddress IS NOT NULL) AND (NOT (PNPDeviceID LIKE 'ROOT%'))",  
            L"MACAddress",  
        };  

        static std::string g_hardware_code;

        int get_system_info(QueryType query_type, std::string& ret_data)
        {
            using namespace std;
            // Step 1: --------------------------------------------------  
            // Initialize COM. ------------------------------------------  

            auto m_hres =  CoInitializeEx(0, COINIT_MULTITHREADED);   
            if (FAILED(m_hres))  
            {  
                cout << "Failed to initialize COM library. Error code = 0x"   
                    << hex << m_hres << endl;  
                return 1;                  // Program has failed.  
            }  

            // Step 2: --------------------------------------------------  
            // Set general COM security levels --------------------------  

            m_hres =  CoInitializeSecurity(  
                NULL,   
                -1,                          // COM authentication  
                NULL,                        // Authentication services  
                NULL,                        // Reserved  
                RPC_C_AUTHN_LEVEL_DEFAULT,   // Default authentication   
                RPC_C_IMP_LEVEL_IMPERSONATE, // Default Impersonation    
                NULL,                        // Authentication info  
                EOAC_NONE,                   // Additional capabilities   
                NULL                         // Reserved  
                );  


            if (FAILED(m_hres))  
            {  
                cout << "Failed to initialize security. Error code = 0x"   
                    << hex << m_hres << endl;  
                CoUninitialize();  
                return 1;                    // Program has failed.  
            }  

            // Step 3: ---------------------------------------------------  
            // Obtain the initial locator to WMI -------------------------  

            IWbemLocator *pLoc = NULL;  

            m_hres = CoCreateInstance(  
                CLSID_WbemLocator,               
                0,   
                CLSCTX_INPROC_SERVER,   
                IID_IWbemLocator, (LPVOID *) &pLoc);  

            if (FAILED(m_hres))  
            {  
                cout << "Failed to create IWbemLocator object."  
                    << " Err code = 0x"  
                    << hex << m_hres << endl;  
                CoUninitialize();  
                return 1;                 // Program has failed.  
            }  

            // Step 4: -----------------------------------------------------  
            // Connect to WMI through the IWbemLocator::ConnectServer method  

            IWbemServices *pSvc = NULL;  

            // Connect to the root\cimv2 namespace with  
            // the current user and obtain pointer pSvc  
            // to make IWbemServices calls.  
            m_hres = pLoc->ConnectServer(  
                _bstr_t(L"ROOT\\CIMV2"), // Object path of WMI namespace  
                NULL,                    // User name. NULL = current user  
                NULL,                    // User password. NULL = current  
                0,                       // Locale. NULL indicates current  
                NULL,                    // Security flags.  
                0,                       // Authority (for example, Kerberos)  
                0,                       // Context object   
                &pSvc                    // pointer to IWbemServices proxy  
                );  

            if (FAILED(m_hres))  
            {  
                cout << "Could not connect. Error code = 0x"   
                    << hex << m_hres << endl;  
                pLoc->Release();       
                CoUninitialize();  
                return 1;                // Program has failed.  
            }  

            // Step 5: --------------------------------------------------  
            // Set security levels on the proxy -------------------------  

            m_hres = CoSetProxyBlanket(  
                pSvc,                        // Indicates the proxy to set  
                RPC_C_AUTHN_WINNT,           // RPC_C_AUTHN_xxx  
                RPC_C_AUTHZ_NONE,            // RPC_C_AUTHZ_xxx  
                NULL,                        // Server principal name   
                RPC_C_AUTHN_LEVEL_CALL,      // RPC_C_AUTHN_LEVEL_xxx   
                RPC_C_IMP_LEVEL_IMPERSONATE, // RPC_C_IMP_LEVEL_xxx  
                NULL,                        // client identity  
                EOAC_NONE                    // proxy capabilities   
                );  

            if (FAILED(m_hres))  
            {  
                cout << "Could not set proxy blanket. Error code = 0x"   
                    << hex << m_hres << endl;  
                pSvc->Release();  
                pLoc->Release();       
                CoUninitialize();  
                return 1;               // Program has failed.  
            }  

            // Step 6: --------------------------------------------------  
            // Use the IWbemServices pointer to make requests of WMI ----  

            // For example, get the name of the operating system  
            IEnumWbemClassObject* pEnumerator = NULL;  
            m_hres = pSvc->ExecQuery(  
                bstr_t("WQL"),   
                bstr_t(szWQLQuery[query_type].szSelect),  
                WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,   
                NULL,  
                &pEnumerator);  

            if (FAILED(m_hres))  
            {  
                cout << "Query for operating system name failed."  
                    << " Error code = 0x"   
                    << hex << m_hres << endl;  
                pSvc->Release();  
                pLoc->Release();  
                CoUninitialize();  
                return 1;               // Program has failed.  
            }  

            // Step 7: -------------------------------------------------  
            // Get the data from the query in step 6 -------------------  

            IWbemClassObject *pclsObj = NULL;  
            ULONG uReturn = 0;  

            while (pEnumerator)  
            {  
                IWbemClassObject *pclsObj = NULL;  
                ULONG uReturn = 0;  

                pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);  

                if (uReturn == 0)  
                {  
                    break;  
                }  
                VARIANT vtProperty;  
                VariantInit(&vtProperty);   
                pclsObj->Get(szWQLQuery[query_type].szProperty, 0, &vtProperty, NULL, NULL );  
                ret_data = (_bstr_t)vtProperty.bstrVal;
                VariantClear(&vtProperty);  
                pclsObj->Release();  
            }  

            // Cleanup  
            // ========  
            pEnumerator->Release();  
            pSvc->Release();  
            pLoc->Release();  

            //需自己调用
            CoUninitialize();  

            return 0;   // Program successfully completed.  
        }
    }
    
    std::string get_disk_code()
    {
        std::string disk_code;
        detail::get_system_info(detail::QueryDiskSeriral, disk_code);
        return disk_code;
    }

    std::string get_cpu_code()
    {
        std::string cpu_code;
        detail::get_system_info(detail::QueryProcessorId, cpu_code);
        return cpu_code;
    }

    std::string get_baseboard_code()
    {
        std::string baseboard_code;
        detail::get_system_info(detail::QueryBaseBoardSeriral, baseboard_code);
        return baseboard_code;
    }

    std::string get_machine_key()
    {
        if (detail::g_hardware_code.empty())
        {
            std::string temp;
            temp.append(get_disk_code()).append(get_cpu_code()).append(get_baseboard_code());
            detail::g_hardware_code = codec::MD5(temp);
        }
        
        return detail::g_hardware_code;
    }

    std::string local_machine_hash()
    {
        auto ret = net::GetNetCardInfo();

        if (ret.empty())
            return "";

        auto item = ret.begin();
       
        if (item->m_mac.empty())
        {
            item->m_mac = snqu::os::get_host_name();
        }

        return std::to_string(codec::Adler32(item->m_mac));
    }
}