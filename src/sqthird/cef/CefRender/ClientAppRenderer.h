#pragma once

#include "include/cef_app.h"
#include "include/wrapper/cef_message_router.h"

class CefClient;

class ClientAppRenderer : public CefApp
	, public CefRenderProcessHandler
{
public:

	ClientAppRenderer() 
		: last_node_is_editable_(false)
	{}
	~ClientAppRenderer() {}

	static CefRefPtr<ClientAppRenderer>& Instance();
	static int  CefAppInitialize();
	static void CefAppDeInitialize();

	virtual void OnBeforeCommandLineProcessing(
		const CefString& process_type,
		CefRefPtr<CefCommandLine> command_line) OVERRIDE;

	// CefApp methods:
	//virtual CefRefPtr<CefBrowserProcessHandler> GetBrowserProcessHandler() OVERRIDE { return this; }
	virtual CefRefPtr<CefRenderProcessHandler> GetRenderProcessHandler() OVERRIDE { return this; }

	//
	// CefRenderProcessHandler methods:
	//

	virtual void OnContextCreated(CefRefPtr<CefBrowser> browser,
		CefRefPtr<CefFrame> frame,
		CefRefPtr<CefV8Context> context) OVERRIDE;

	virtual void OnUncaughtException(CefRefPtr<CefBrowser> browser,
		CefRefPtr<CefFrame> frame,
		CefRefPtr<CefV8Context> context,
		CefRefPtr<CefV8Exception> exception,
		CefRefPtr<CefV8StackTrace> stackTrace) OVERRIDE;

	virtual bool OnProcessMessageReceived(
		CefRefPtr<CefBrowser> browser,
		CefProcessId source_process,
		CefRefPtr<CefProcessMessage> message) OVERRIDE;

	virtual void OnFocusedNodeChanged(CefRefPtr<CefBrowser> browser,
		CefRefPtr<CefFrame> frame,
		CefRefPtr<CefDOMNode> node) OVERRIDE;

	void OnWebKitInitialized() OVERRIDE;

	// Handles the renderer side of query routing.
	CefRefPtr<CefMessageRouterRendererSide> message_router_;

private:
	IMPLEMENT_REFCOUNTING(ClientAppRenderer);

	bool last_node_is_editable_;
};
