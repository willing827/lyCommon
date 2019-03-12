#pragma once
#include <sqwin/sqwin.h>
#include <string>


namespace snqu { namespace service{

    bool Exist(const std::string& svc_name);
    bool Install(const std::string& svc_file, const std::string& svc_name, const std::string& svc_desc, 
                 bool is_reg = false, bool is_driver_svc = false);
    bool Delete(const std::string& svc_name, bool is_reg = false);
    bool Start(const std::string& svc_name, DWORD wait_running_timeout = 0/*milliseconds*/);
    bool Pause(const std::string& svc_name);
    bool Resume(const std::string& svc_name);
    bool Enable(const std::string& svc_name, bool enable);
    bool Stop(const std::string& svc_name, DWORD wait_stopped_timeout = 0/*milliseconds*/);
    bool ChangePEFile(const std::string& svc_name, const std::string& to_pe_file, bool is_reg = false);
    bool ChangeDescription(const std::string& svc_name, const std::string& description);
    bool IsRunning(const std::string& svc_name);
    std::string GetPEPath(const std::string& svc_name, bool is_reg = false);	// NOTE: DLL service will return "%system%\svchost.exe -k name" !!!
    std::string GetRegistryPath(const std::string& svc_name, bool at_svchost, void** root_key = NULL);

}}