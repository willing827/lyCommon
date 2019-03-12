/////////////////////////////////////////////////////////////////////////////// 
// FileName:    Firewall_util.cpp
// Created:     2016/11/03
// Author:      luhaijun 
//----------------------------------------------------------------------------- 
/////////////////////////////////////////////////////////////////////////////// 

#include <sqwin/win/sqfirewall.h>
#include <sqwin/sqwin.h>
#include <crtdbg.h>
#include <netfw.h>
#include <codec/sqcodec.h>

#pragma comment( lib, "ole32.lib" )
#pragma comment( lib, "oleaut32.lib" )

class OpFirewall
{
public:
    OpFirewall();
    ~OpFirewall();

    HRESULT		Initialize();

    void		WindowsFirewallCleanup();
    HRESULT		WindowsFirewallIsOn(OUT BOOL* fwOn);
    HRESULT		WindowsFirewallTurnOn();
    HRESULT		WindowsFirewallTurnOff();
    HRESULT		WindowsFirewallAppIsEnabled(IN const wchar_t* fwProcessImageFileName, OUT BOOL* fwAppEnabled);
    HRESULT		WindowsFirewallAddApp(IN const wchar_t* fwProcessImageFileName, IN const wchar_t* fwName);
    HRESULT		WindowsFirewallRemoveApp(IN const wchar_t* fwProcessImageFileName);
    HRESULT		WindowsFirewallPortIsEnabled(IN LONG portNumber, IN NET_FW_IP_PROTOCOL ipProtocol, OUT BOOL* fwPortEnabled);
    HRESULT		WindowsFirewallPortAdd(IN LONG portNumber, IN NET_FW_IP_PROTOCOL ipProtocol, IN const wchar_t* name);

private:
    HRESULT		WindowsFirewallInitialize(OUT INetFwProfile** fwProfile);

private:
    HRESULT		m_comInit;
    INetFwProfile* m_fwProfile;

};

OpFirewall::OpFirewall()
{	
	m_comInit = CoInitializeEx(0, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
	m_fwProfile = NULL;
}


OpFirewall::~OpFirewall()
{
	if (SUCCEEDED(m_comInit))
		CoUninitialize();

	if (m_fwProfile)
		WindowsFirewallCleanup();
}

HRESULT OpFirewall::Initialize()
{
	HRESULT hr = S_OK;
	m_comInit = CoInitializeEx(0, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
	
	if (m_comInit != RPC_E_CHANGED_MODE){	
		if (FAILED(m_comInit))
			return m_comInit;
	}
	
	hr = WindowsFirewallInitialize(&m_fwProfile);
	return hr;
}

HRESULT OpFirewall::WindowsFirewallInitialize(OUT INetFwProfile** m_fwProfile)
{
	HRESULT hr = S_OK;
	INetFwMgr* fwMgr = NULL;
	INetFwPolicy* fwPolicy = NULL;

	_ASSERT(m_fwProfile != NULL);
	*m_fwProfile = NULL;
	hr = CoCreateInstance(__uuidof(NetFwMgr), NULL, CLSCTX_INPROC_SERVER, __uuidof(INetFwMgr), (void**)&fwMgr);
	if (FAILED(hr))	
		goto error;

	// Retrieve the local firewall policy.
	hr = fwMgr->get_LocalPolicy(&fwPolicy);
	if (FAILED(hr))
		goto error;

	// Retrieve the firewall profile currently in effect.
	hr = fwPolicy->get_CurrentProfile(m_fwProfile);
	if (FAILED(hr))
		goto error;
error:

	if (fwPolicy != NULL)
		fwPolicy->Release();

	if (fwMgr != NULL)
		fwMgr->Release();

	return hr;
}


void OpFirewall::WindowsFirewallCleanup()
{
	// Release the firewall profile.
	if (m_fwProfile != NULL)
		m_fwProfile->Release();
}


HRESULT OpFirewall::WindowsFirewallIsOn(OUT BOOL* fwOn)
{
	HRESULT hr = S_OK;
	VARIANT_BOOL fwEnabled;

	_ASSERT(m_fwProfile != NULL);
	_ASSERT(fwOn != NULL);

	*fwOn = FALSE;

	// Get the current state of the firewall.
	hr = m_fwProfile->get_FirewallEnabled(&fwEnabled);
	if (FAILED(hr))
		goto error;

	// Check to see if the firewall is on.
	if (fwEnabled != VARIANT_FALSE)
		*fwOn = TRUE;

error:
	return hr;
}


HRESULT OpFirewall::WindowsFirewallTurnOn()
{
	HRESULT hr = S_OK;
	BOOL fwOn;

	_ASSERT(m_fwProfile != NULL);

	// Check to see if the firewall is off.
	hr = WindowsFirewallIsOn(&fwOn);
	if (FAILED(hr))		
		goto error;

	// If it is, turn it on.
	if (!fwOn)
	{
		// Turn the firewall on.
		hr = m_fwProfile->put_FirewallEnabled(VARIANT_TRUE);
		if (FAILED(hr))
			goto error;
	}

error:

	return hr;
}


HRESULT OpFirewall::WindowsFirewallTurnOff()
{
	HRESULT hr = S_OK;
	BOOL fwOn;

	_ASSERT(m_fwProfile != NULL);

	// Check to see if the firewall is on.
	hr = WindowsFirewallIsOn(&fwOn);
	if (FAILED(hr))
		goto error;

	// If it is, turn it off.
	if (fwOn)
	{
		// Turn the firewall off.
		hr = m_fwProfile->put_FirewallEnabled(VARIANT_FALSE);
		if (FAILED(hr))
			goto error;
	}

error:

	return hr;
}


HRESULT OpFirewall::WindowsFirewallAppIsEnabled(IN const wchar_t* fwProcessImageFileName, OUT BOOL* fwAppEnabled)
{
	HRESULT hr = S_OK;
	BSTR fwBstrProcessImageFileName = NULL;
	VARIANT_BOOL fwEnabled;
	INetFwAuthorizedApplication* fwApp = NULL;
	INetFwAuthorizedApplications* fwApps = NULL;

	_ASSERT(m_fwProfile != NULL);
	_ASSERT(fwProcessImageFileName != NULL);
	_ASSERT(fwAppEnabled != NULL);

	*fwAppEnabled = FALSE;

	// Retrieve the authorized application collection.
	hr = m_fwProfile->get_AuthorizedApplications(&fwApps);
	if (FAILED(hr))
		goto error;

	// Allocate a BSTR for the process image file name.
	fwBstrProcessImageFileName = SysAllocString(fwProcessImageFileName);
	if (fwBstrProcessImageFileName == NULL){
		hr = E_OUTOFMEMORY;
		goto error;
	}

	// Attempt to retrieve the authorized application.
	hr = fwApps->Item(fwBstrProcessImageFileName, &fwApp);
	if (SUCCEEDED(hr))
	{
		// Find out if the authorized application is enabled.
		hr = fwApp->get_Enabled(&fwEnabled);
		if (FAILED(hr))
			goto error;

		if (fwEnabled != VARIANT_FALSE){
			// The authorized application is enabled.
			*fwAppEnabled = TRUE;
		}
	}
	else
	{
		// The authorized application was not in the collection.
		hr = S_OK;
	}

error:

	SysFreeString(fwBstrProcessImageFileName);

	// Release the authorized application instance.
	if (fwApp != NULL)
		fwApp->Release();

	// Release the authorized application collection.
	if (fwApps != NULL)
		fwApps->Release();

	return hr;
}

//WindowsFirewallAddApp(L"%ProgramFiles%\\Messenger\\msmsgs.exe",L"Windows Messenger");

HRESULT OpFirewall::WindowsFirewallAddApp(IN const wchar_t* fwProcessImageFileName, IN const wchar_t* fwName)
{
	HRESULT hr = S_OK;
	BOOL fwAppEnabled;
	BSTR fwBstrName = NULL;
	BSTR fwBstrProcessImageFileName = NULL;
	INetFwAuthorizedApplication* fwApp = NULL;
	INetFwAuthorizedApplications* fwApps = NULL;

	_ASSERT(m_fwProfile != NULL);
	_ASSERT(fwProcessImageFileName != NULL);
	_ASSERT(fwName != NULL);

	// First check to see if the application is already authorized.
	hr = WindowsFirewallAppIsEnabled(fwProcessImageFileName, &fwAppEnabled);
	if (FAILED(hr))
		goto error;

	// Only add the application if it isn't already authorized.
	if (!fwAppEnabled)
	{
		// Retrieve the authorized application collection.
		hr = m_fwProfile->get_AuthorizedApplications(&fwApps);
		if (FAILED(hr))
			goto error;

		// Create an instance of an authorized application.
		hr = CoCreateInstance(__uuidof(NetFwAuthorizedApplication), NULL, CLSCTX_INPROC_SERVER, __uuidof(INetFwAuthorizedApplication), (void**)&fwApp);
		if (FAILED(hr))
			goto error;

		// Allocate a BSTR for the process image file name.
		fwBstrProcessImageFileName = SysAllocString(fwProcessImageFileName);
		if (fwBstrProcessImageFileName == NULL){
			hr = E_OUTOFMEMORY;
			goto error;
		}

		// Set the process image file name.
		hr = fwApp->put_ProcessImageFileName(fwBstrProcessImageFileName);
		if (FAILED(hr))
			goto error;
		
		// Allocate a BSTR for the application friendly name.
		fwBstrName = SysAllocString(fwName);
		if (SysStringLen(fwBstrName) == 0){
			hr = E_OUTOFMEMORY;
			goto error;
		}

		// Set the application friendly name.
		hr = fwApp->put_Name(fwBstrName);
		if (FAILED(hr))
			goto error;

		// Add the application to the collection.
		hr = fwApps->Add(fwApp);
		if (FAILED(hr))		
			goto error;
	}

error:

	// Free the BSTRs.
	SysFreeString(fwBstrName);
	SysFreeString(fwBstrProcessImageFileName);

	if (fwApp != NULL)
		fwApp->Release();

	if (fwApps != NULL)
		fwApps->Release();

	return hr;
}


HRESULT OpFirewall::WindowsFirewallRemoveApp(IN const wchar_t* fwProcessImageFileName)
{
	HRESULT hr = S_OK;
	BSTR fwBstrProcessImageFileName = NULL;
	INetFwAuthorizedApplication* fwApp = NULL;
	INetFwAuthorizedApplications* fwApps = NULL;

	_ASSERT(m_fwProfile != NULL);
	_ASSERT(fwProcessImageFileName != NULL);


	hr = m_fwProfile->get_AuthorizedApplications(&fwApps);

	// Allocate a BSTR for the process image file name.
	fwBstrProcessImageFileName = SysAllocString(fwProcessImageFileName);
	if (fwBstrProcessImageFileName == NULL){
		hr = E_OUTOFMEMORY;
		goto error;
	}
	
	hr = fwApps->Remove(fwBstrProcessImageFileName);
	
error:

	// Free the BSTRs.
	SysFreeString(fwBstrProcessImageFileName);

	if (fwApp != NULL)
		fwApp->Release();

	if (fwApps != NULL)
		fwApps->Release();

	return hr;
}

HRESULT OpFirewall::WindowsFirewallPortIsEnabled(IN LONG portNumber, IN NET_FW_IP_PROTOCOL ipProtocol, OUT BOOL* fwPortEnabled)
{
	HRESULT hr = S_OK;
	VARIANT_BOOL fwEnabled;
	INetFwOpenPort* fwOpenPort = NULL;
	INetFwOpenPorts* fwOpenPorts = NULL;

	_ASSERT(m_fwProfile != NULL);
	_ASSERT(fwPortEnabled != NULL);

	*fwPortEnabled = FALSE;

	// Retrieve the globally open ports collection.
	hr = m_fwProfile->get_GloballyOpenPorts(&fwOpenPorts);
	if (FAILED(hr))
		goto error;

	// Attempt to retrieve the globally open port.
	hr = fwOpenPorts->Item(portNumber, ipProtocol, &fwOpenPort);
	if (SUCCEEDED(hr)){
		// Find out if the globally open port is enabled.
		hr = fwOpenPort->get_Enabled(&fwEnabled);
		if (FAILED(hr))
			goto error;

		if (fwEnabled != VARIANT_FALSE)
			// The globally open port is enabled.
			*fwPortEnabled = TRUE;
		
	}
	else{
		// The globally open port was not in the collection.
		hr = S_OK;
	}

error:

	// Release the globally open port.
	if (fwOpenPort != NULL)
		fwOpenPort->Release();

	// Release the globally open ports collection.
	if (fwOpenPorts != NULL)
		fwOpenPorts->Release();

	return hr;
}

//WindowsFirewallPortAdd(80, NET_FW_IP_PROTOCOL_TCP, L"WWW");

HRESULT OpFirewall::WindowsFirewallPortAdd(IN LONG portNumber, IN NET_FW_IP_PROTOCOL ipProtocol, IN const wchar_t* name)
{
	HRESULT hr = S_OK;
	BOOL fwPortEnabled;
	BSTR fwBstrName = NULL;
	INetFwOpenPort* fwOpenPort = NULL;
	INetFwOpenPorts* fwOpenPorts = NULL;

	_ASSERT(m_fwProfile != NULL);
	_ASSERT(name != NULL);

	// First check to see if the port is already added.
	hr = WindowsFirewallPortIsEnabled(portNumber, ipProtocol, &fwPortEnabled);
	if (FAILED(hr))
		goto error;

	// Only add the port if it isn't already added.
	if (!fwPortEnabled)
	{
		// Retrieve the collection of globally open ports.
		hr = m_fwProfile->get_GloballyOpenPorts(&fwOpenPorts);
		if (FAILED(hr))	
			goto error;
		// Create an instance of an open port.
		hr = CoCreateInstance(__uuidof(NetFwOpenPort), NULL, CLSCTX_INPROC_SERVER, __uuidof(INetFwOpenPort), (void**)&fwOpenPort);
		if (FAILED(hr))
			goto error;

		// Set the port number.
		hr = fwOpenPort->put_Port(portNumber);
		if (FAILED(hr))
			goto error;

		// Set the IP protocol.
		hr = fwOpenPort->put_Protocol(ipProtocol);
		if (FAILED(hr))
			goto error;

		// Allocate a BSTR for the friendly name of the port.
		fwBstrName = SysAllocString(name);
		if (SysStringLen(fwBstrName) == 0){
			hr = E_OUTOFMEMORY;
			goto error;
		}

		// Set the friendly name of the port.
		hr = fwOpenPort->put_Name(fwBstrName);
		if (FAILED(hr))
			goto error;

		// Opens the port and adds it to the collection.
		hr = fwOpenPorts->Add(fwOpenPort);
		if (FAILED(hr))
			goto error;

	}

error:

	SysFreeString(fwBstrName);
	// Release the open port instance.
	if (fwOpenPort != NULL)
		fwOpenPort->Release();

	// Release the globally open ports collection.
	if (fwOpenPorts != NULL)
		fwOpenPorts->Release();

	return hr;
}

namespace snqu { namespace os{

	bool AddFireWallApp( const char* pe_file_name, const char* app_name)
	{
		if (pe_file_name == NULL || app_name == NULL)
			return false;

		OpFirewall _OpFirewall;
		if (SUCCEEDED(_OpFirewall.Initialize())){
            std::wstring fwProcessImageFileName = codec::S2W(pe_file_name);
            std::wstring fwName = codec::S2W(app_name);
			HRESULT hr = _OpFirewall.WindowsFirewallAddApp(fwProcessImageFileName.c_str(), fwName.c_str());
			if (SUCCEEDED(hr))
				return true;
		}
		return false;
	}

	bool DelFireWallApp(const char* pe_file_name)
	{
		if (pe_file_name == NULL)
			return false;

		OpFirewall _OpFirewall;
		if (SUCCEEDED(_OpFirewall.Initialize())){
            std::wstring fwProcessImageFileName = codec::S2W(pe_file_name);
			HRESULT hr = _OpFirewall.WindowsFirewallRemoveApp(fwProcessImageFileName.c_str());
			if (SUCCEEDED(hr))
				return true;
		}
		return false;
	}
}}