#include <sqwin/win/sqservice.h>
#include <sqwin/win/sqreg.h>
#include <sqwin/win/sqpath.h>
#include <sqstd/sqstringhelper.h>

namespace snqu { namespace service{

    SC_HANDLE _open_service_manager(void)
    {
        return OpenSCManagerW(NULL, NULL/*SERVICES_ACTIVE_DATABASEW*/, SC_MANAGER_ALL_ACCESS);
    }

    bool _change_pe(SC_HANDLE svc_manager, const char* svc_name, const char* pe)
    {
        SC_HANDLE	hme = OpenServiceA(svc_manager, svc_name, SERVICE_CHANGE_CONFIG);
        bool		ret = false;

        if (hme)
        {
            ret = ChangeServiceConfigA(hme, SERVICE_NO_CHANGE, SERVICE_NO_CHANGE, SERVICE_NO_CHANGE,
                pe, NULL, NULL, NULL, NULL, NULL, NULL) == TRUE;
            CloseServiceHandle(hme);
        }

        return ret;
    }
    bool _enable_service(SC_HANDLE svc_manager, const char* svc_name, bool enable)
    {
        SC_HANDLE	hme = OpenServiceA(svc_manager, svc_name, SERVICE_CHANGE_CONFIG);
        bool		ret = false;

        if (hme)
        {
            ret = ChangeServiceConfigA(hme, SERVICE_NO_CHANGE, enable ? SERVICE_AUTO_START : SERVICE_DISABLED, 
                SERVICE_NO_CHANGE, NULL, NULL, NULL, NULL, NULL, NULL, NULL) == TRUE;
            CloseServiceHandle(hme);
        }

        return ret;
    }
    bool _change_description(SC_HANDLE svc, std::string desc)
    {
        SERVICE_DESCRIPTIONA	sd = { &desc[0] };

        return ChangeServiceConfig2A(svc, SERVICE_CONFIG_DESCRIPTION, &sd) == TRUE;
    }
    bool _change_description(SC_HANDLE svc_manager, const char* svc_name, const char* desc)
    {
        SC_HANDLE	hme = OpenServiceA(svc_manager, svc_name, SERVICE_CHANGE_CONFIG);
        bool		ret = false;

        if (hme)
        {
            ret = _change_description(hme, desc);
            CloseServiceHandle(hme);
        }

        return ret;
    }

    bool Exist(const std::string& svc_name)
    {
        SC_HANDLE		hsm = _open_service_manager(), hme = NULL;
        bool			ret = false;
        SERVICE_STATUS	statu = { 0 };

        if (hsm)
        {
            hme = OpenServiceA(hsm, svc_name.c_str(), SERVICE_QUERY_STATUS);
            if (hme)
            {
                ret = true;
            }
            else
            {
                if (ERROR_SERVICE_DOES_NOT_EXIST != GetLastError())
                {
                    ret = true;
                }
            }
            CloseServiceHandle(hsm);
        }

        return ret;
    }

    bool Install(const std::string& svc_file, const std::string& svc_name, const std::string& svc_desc, bool is_reg, bool is_driver_svc)
    {
        bool		ret = false;
        SC_HANDLE	hsm = _open_service_manager(),
            hme = NULL;

        if (hsm)
        {
            auto ext = path::get_externtion(svc_file);
            if (str::nequal(ext, ".dll") == 0 && is_reg)
            {
                // DLL service by svchost.exe
                std::string	svchost("%systemroot%\\system32\\svchost.exe -k ");
                DWORD			err = 0;

                svchost += svc_name;
                ret = reg::CreateKey(HKEY_LOCAL_MACHINE, GetRegistryPath(svc_name, true), NULL) == 0;
                if (ret)
                {
                    std::string temp_val;
                    ret = reg::SetStr(HKEY_LOCAL_MACHINE, GetRegistryPath(NULL, true), svc_name, temp_val, reg::REGSTR_MULTI) == 0;
                    if (!ret)	reg::RemoveVal(HKEY_LOCAL_MACHINE, GetRegistryPath(svc_name, true).c_str(), NULL);
                }

                if (ret)
                {
                    ret = false;
                    DWORD svc_type = SERVICE_WIN32_SHARE_PROCESS;
                    DWORD svc_start = SERVICE_AUTO_START;

                    if (is_driver_svc)
                    {//驱动类型的服要特殊处理
                        svc_type = SERVICE_DRIVER;
                        svc_start = SERVICE_DEMAND_START;
                    }
                    hme = CreateServiceA(hsm, svc_name.c_str(), svc_name.c_str(), SC_MANAGER_ALL_ACCESS, svc_type
                        , svc_start, SERVICE_ERROR_NORMAL, svchost.c_str()
                        , NULL, NULL, NULL, NULL, NULL);
                    err = GetLastError();
                    if (hme == NULL && err == ERROR_SERVICE_EXISTS)
                    {
                        hme = OpenServiceA(hsm, svc_name.c_str(), SC_MANAGER_ALL_ACCESS);
                        if (hme)
                        {
                            DeleteService(hme);
                            CloseServiceHandle(hme);
                            hme = CreateServiceA(hsm, svc_name.c_str(), svc_name.c_str(), SC_MANAGER_ALL_ACCESS, SERVICE_WIN32_SHARE_PROCESS
                                , SERVICE_AUTO_START, SERVICE_ERROR_NORMAL, svchost.c_str()
                                , NULL, NULL, NULL, NULL, NULL);
                        }
                    }
                    if (hme && is_reg)
                    {
                        ret = reg::SetStr(HKEY_LOCAL_MACHINE, (GetRegistryPath(svc_name, false) + "\\Parameters"), "ServiceDll", svc_file, reg::REGSTR_EXPAND) == 0;
                        if (ret)
                        {
                            ret = reg::SetStr(HKEY_LOCAL_MACHINE, GetRegistryPath(svc_name, false).c_str(), "Description", svc_desc) == 0;
                        }
                    }
                    if (!ret)
                    {
                        if (hme)	DeleteService(hme);
                        reg::RemoveVal(HKEY_LOCAL_MACHINE, GetRegistryPath(svc_name, true).c_str(), NULL);
                        reg::RemoveVal(HKEY_LOCAL_MACHINE, GetRegistryPath(svc_name, false).c_str(), NULL);
                    }
                    if (hme)	CloseServiceHandle(hme);
                }
            }
            else
            {
                // exe independent process service...
                std::string	pe(svc_file);
                hme = CreateServiceA(hsm, svc_name.c_str(), svc_name.c_str(), SC_MANAGER_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS
                    , SERVICE_AUTO_START, SERVICE_ERROR_IGNORE, pe.c_str()
                    , NULL, NULL, NULL, NULL, NULL);
                if (!hme)
                {
                    if (GetLastError() == ERROR_SERVICE_EXISTS)
                    {
                        ret = _change_pe(hsm, svc_name.c_str(), svc_file.c_str());
                        _change_description(hsm, svc_name.c_str(), svc_desc.c_str());
                    }
                    else if (GetLastError() == ERROR_SERVICE_MARKED_FOR_DELETE)
                    {
                        ;
                    }
                }
                else
                {
                    if (!svc_desc.empty()) _change_description(hme, svc_desc);
                    ret = true;
                    CloseServiceHandle(hme);
                }
            }

            CloseServiceHandle(hsm);
        }

        return ret;
    }

    bool Delete(const std::string& svc_name, bool is_reg)
    {
        // for avoid facing error ERROR_SERVICE_MARKED_FOR_DELETE in CreateService, strong recommend you NOT call this !!!
        SC_HANDLE	hsm = _open_service_manager(), hme = NULL;
        bool		ret = false;

        if (hsm)
        {
            hme = OpenServiceA(hsm, svc_name.c_str(), SC_MANAGER_ALL_ACCESS);
            if (hme)
            {
                ret = DeleteService(hme) == TRUE;
                if (!ret && GetLastError() == ERROR_SERVICE_MARKED_FOR_DELETE) ret = true;
                CloseServiceHandle(hme);
            }
            CloseServiceHandle(hsm);
        }
        if (ret && is_reg)
        {
            reg::RemoveVal(HKEY_LOCAL_MACHINE, (std::string("Software\\Microsoft\\Windows NT\\CurrentVersion\\Svchost\\") + svc_name), NULL);
            reg::RemoveVal(HKEY_LOCAL_MACHINE, (std::string("System\\CurrentControlSet\\Services\\") + svc_name), NULL);
        }

        return ret;
    }

    bool Start(const std::string& svc_name, DWORD wait_running_timeout)
    {
        SC_HANDLE	hsm = _open_service_manager(), hme = NULL;
        bool		ret = false;

        if (hsm)
        {
            hme = OpenServiceA(hsm, svc_name.c_str(), SERVICE_QUERY_STATUS | SERVICE_START);
            if (hme)
            {
                BOOL	started = StartServiceA(hme, 0, NULL);
                if (!started && GetLastError() == ERROR_SERVICE_DISABLED)
                {
                    if (_enable_service(hsm, svc_name.c_str(), true))	started = StartServiceA(hme, 0, NULL);
                }
                if (started)
                {
                    if (wait_running_timeout == 0) ret = true;
                    else
                    {
                        SERVICE_STATUS	statu = { 0 };
                        DWORD			begin = GetTickCount();
                        while (QueryServiceStatus(hme, &statu))
                        {
                            if (statu.dwCurrentState == SERVICE_RUNNING)
                            {
                                ret = true;
                                break;
                            }

                            DWORD	cur = GetTickCount();
                            if ((cur >= begin && cur - begin < wait_running_timeout) ||
                                (((DWORD)-1) - begin + cur < wait_running_timeout))
                            {
                                Sleep(50);
                            }
                            else
                            {
                                break;
                            }
                        }
                    }
                }
                CloseServiceHandle(hme);
            }
            CloseServiceHandle(hsm);
        }
        return ret;
    }

    bool Pause(const std::string& svc_name)
    {
        SC_HANDLE	hsm = _open_service_manager(), hme = NULL;
        bool		ret = false;

        if (hsm)
        {
            hme = OpenServiceA(hsm, svc_name.c_str(), SERVICE_PAUSE_CONTINUE);
            if (hme)
            {
                SERVICE_STATUS	statu = { 0 };
                ret = ControlService(hme, SERVICE_CONTROL_PAUSE, &statu) == TRUE;
                CloseServiceHandle(hme);
            }
            CloseServiceHandle(hsm);
        }
        return ret;
    }

    bool Resume(const std::string& svc_name)
    {
        SC_HANDLE	hsm = _open_service_manager(), hme = NULL;
        bool		ret = false;

        if (hsm)
        {
            hme = OpenServiceA(hsm, svc_name.c_str(), SERVICE_PAUSE_CONTINUE);
            if (hme)
            {
                SERVICE_STATUS	statu = { 0 };
                ret = ControlService(hme, SERVICE_CONTROL_CONTINUE, &statu) == TRUE;
                CloseServiceHandle(hme);
            }
            CloseServiceHandle(hsm);
        }
        return ret;
    }

    bool Enable(const std::string& svc_name, bool enable)
    {
        SC_HANDLE	hsm = _open_service_manager();
        bool		ret = false;

        if (hsm)
        {
            ret = _enable_service(hsm, svc_name.c_str(), enable);
            CloseServiceHandle(hsm);
        }
        return ret;
    }

    bool Stop(const std::string& svc_name, DWORD wait_stopped_timeout)
    {
        SC_HANDLE	hsm = _open_service_manager(), hme = NULL;
        bool		ret = false;

        if (hsm)
        {
            hme = OpenServiceA(hsm, svc_name.c_str(), SERVICE_QUERY_STATUS | SERVICE_STOP);
            if (hme)
            {
                SERVICE_STATUS	statu = { 0 };
                if (ControlService(hme, SERVICE_CONTROL_STOP, &statu))
                {
                    if (wait_stopped_timeout == 0) ret = true;
                    else
                    {
                        DWORD			begin = GetTickCount();
                        while (QueryServiceStatus(hme, &statu))
                        {
                            if (statu.dwCurrentState == SERVICE_STOPPED)
                            {
                                ret = true;
                                break;
                            }

                            DWORD	cur = GetTickCount();
                            if ((cur >= begin && cur - begin < wait_stopped_timeout) ||
                                (((DWORD)-1) - begin + cur < wait_stopped_timeout))
                            {
                                Sleep(50);
                            }
                            else
                            {
                                break;
                            }
                        }
                    }
                }
                CloseServiceHandle(hme);
            }
            CloseServiceHandle(hsm);
        }
        return ret;
    }

    bool ChangePEFile(const std::string& svc_name, const std::string& to_pe_file, bool is_reg)
    {
        bool		ret = false;

        if (is_reg)
        {
            std::string ext = path::get_externtion(to_pe_file);
            std::string	path(GetRegistryPath(svc_name, false));
            if (str::nequal(ext, ".dll") == 0)
            {
                std::string	value("%systemroot%\\system32\\svchost.exe -k "), old("");

                value += svc_name;
                reg::GetStr(HKEY_LOCAL_MACHINE, path.c_str(), "ImagePath", old);
                if (reg::SetStr(HKEY_LOCAL_MACHINE, path.c_str(), "ImagePath", value.c_str(), reg::REGSTR_EXPAND) == 0)
                {
                    ret = reg::SetStr(HKEY_LOCAL_MACHINE, (path + "\\Parameters"), "ServiceDll", to_pe_file, reg::REGSTR_EXPAND) == 0;
                    if (!ret)	reg::SetStr(HKEY_LOCAL_MACHINE, path, "ImagePath", old.c_str());
                }
            }
            else
            {
                // as exe service
                ret = reg::SetStr(HKEY_LOCAL_MACHINE, path.c_str(), "ImagePath", to_pe_file, reg::REGSTR_EXPAND) == 0;
            }
        }
        else
        {
            SC_HANDLE	hsm = _open_service_manager();

            if (hsm)
            {
                ret = _change_pe(hsm, svc_name.c_str(), to_pe_file.c_str());
                CloseServiceHandle(hsm);
            }
        }

        return ret;
    }

    bool ChangeDescription(const std::string& svc_name, const std::string& description)
    {
        bool		ret = false;
        SC_HANDLE	hsm = _open_service_manager(), hme = NULL;

        if (hsm)
        {
            hme = OpenServiceA(hsm, svc_name.c_str(), SERVICE_CHANGE_CONFIG);
            if (hme)
            {
                ret = _change_description(hme, description);
                CloseServiceHandle(hme);
            }
            CloseServiceHandle(hsm);
        }

        return ret;
    }

    bool IsRunning(const std::string& svc_name)
    {
        SC_HANDLE		hsm = _open_service_manager(), hme = NULL;
        bool			ret = false;
        SERVICE_STATUS	statu = { 0 };

        if (hsm)
        {
            hme = OpenServiceA(hsm, svc_name.c_str(), SERVICE_QUERY_STATUS);
            if (hme)
            {
                if (QueryServiceStatus(hme, &statu))
                {
                    ret = statu.dwCurrentState == SERVICE_RUNNING || statu.dwCurrentState == SERVICE_START_PENDING;
                }
                CloseServiceHandle(hme);
            }
            CloseServiceHandle(hsm);
        }

        return ret;
    }
    
    std::string GetPEPath(const std::string& svc_name, bool is_reg)
    {
        std::string	pe("");
        if (is_reg)
        {
            std::string	path(GetRegistryPath(svc_name, false));
            reg::GetStr(HKEY_LOCAL_MACHINE, path, "ImagePath", pe);
            if (!pe.empty())
            {
                pe = str::to_lower(pe);
                if (pe.find("\\svchost.exe -k") != std::string::npos)
                {
                    if (reg::GetStr(HKEY_LOCAL_MACHINE, (path + "\\Parameters"), "ServiceDll", pe))
                    {
                        pe = "";
                    }
                }
            }
        }
        else
        {
            SC_HANDLE		hsm = _open_service_manager(), hme = NULL;
            DWORD			err = GetLastError();

            if (hsm)
            {
                hme = OpenServiceA(hsm, svc_name.c_str(), SERVICE_QUERY_CONFIG);
                if (hme)
                {
                    LPQUERY_SERVICE_CONFIGA	buf = NULL;
                    DWORD	size = 0;

                    QueryServiceConfigA(hme, NULL, 0, &size);
                    err = GetLastError();
                    if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
                    {
                        buf = (LPQUERY_SERVICE_CONFIGA)LocalAlloc(LMEM_FIXED, size + 16);
                        if (QueryServiceConfigA(hme, buf, size, &size))
                        {
                            pe = buf->lpBinaryPathName;
                            if (!pe.empty())
                            {
                                if (pe[0] == '\"')
                                {
                                    pe.erase(0, 1);
                                    size = pe.rfind('\"');
                                    if (size != std::string::npos)	pe.erase(size);
                                }
                            }
                        }
                        LocalFree(buf);
                    }
                    CloseServiceHandle(hme);
                }
                err = GetLastError();
                CloseServiceHandle(hsm);
            }
        }

        return pe;
    }

    std::string GetRegistryPath(const std::string& svc_name, bool at_svchost, void** root_key)
    {
        std::string	path(at_svchost ? "Software\\Microsoft\\Windows NT\\CurrentVersion\\Svchost" : "System\\CurrentControlSet\\Services");
        path += "\\";
        path += svc_name;
        if (root_key)	*root_key = (void*)HKEY_LOCAL_MACHINE;

        return path;
    }

    bool HandleCommand(const std::vector<std::string> param_list, const std::string& svc_name, const std::string& svc_desc, bool is_reg)
    {
        int			num = 0;
        bool		handled = false;

        // parameter parse:
        for (auto& cmd : param_list)
        {
            if (str::nequal(cmd, "--install") == 0)
            {
                auto me = path::get_module_filename();
                Install(me, svc_name, svc_desc, is_reg);
                handled = true;
                break;
            }
            else if (str::nequal(cmd, "--uninstall") == 0)
            {
                Stop(svc_name, 3000);
                Delete(svc_name);
                handled = true;
                break;
            }
            else if (str::nequal(cmd, "--restart") == 0)
            {
                Stop(svc_name, 3000);
                Start(svc_name);
                handled = true;
            }
            else if (str::nequal(cmd, "--start") == 0)
            {
                Start(svc_name);
                handled = true;
                break;
            }
            else if (str::nequal(cmd, "--stop") == 0)
            {
                Stop(svc_name);
                handled = true;
                break;
            }
        }
        return handled;
    }

}}