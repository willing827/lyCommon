#include "include/cef_client.h"
#include "ClientAppRenderer.h"
#include "HtmlEventHandler.h"

const char kFocusedNodeChangedMessage[] = "ClientRenderer.FocusedNodeChanged";

int ClientAppRenderer::CefAppInitialize()
{
	CefRefPtr<ClientAppRenderer>  pApp = Instance();

	// get arguments
	CefMainArgs mainArgs(GetModuleHandle(NULL));

	// Execute the secondary process, if any.
	int exit_code = CefExecuteProcess(mainArgs, pApp.get(), NULL);
	if (exit_code >= 0)
		return exit_code;

	CefSettings settings;
	CefString(&settings.log_file) = ("logs\\cef3_render.log");
	settings.no_sandbox = true;
	settings.multi_threaded_message_loop = TRUE;
	settings.remote_debugging_port = 5050;
	settings.log_severity = LOGSEVERITY_DISABLE;

	CefString(&settings.cache_path) = "cache\0\0";

	//CEF Initiaized
	CefInitialize(mainArgs, settings, pApp.get(), NULL);

	return true;
}

void ClientAppRenderer::CefAppDeInitialize()
{
	CefShutdown();
}

void ClientAppRenderer::OnWebKitInitialized()  {
	// Create the renderer-side router for query handling.
	CefMessageRouterConfig config;
	message_router_ = CefMessageRouterRendererSide::Create(config);
}

void ClientAppRenderer::OnBeforeCommandLineProcessing(
	const CefString& process_type,
	CefRefPtr<CefCommandLine> command_line)
{
	// 使用系统的flash
	command_line->AppendSwitch("--disable-web-security");
	command_line->AppendSwitch("--enable-system-flash");

	command_line->AppendSwitch("--disable-gpu");
	command_line->AppendSwitchWithValue("--renderer-process-limit", "1");
}

CefRefPtr<ClientAppRenderer>& ClientAppRenderer::Instance()
{
	static CefRefPtr<ClientAppRenderer> pApp;
	if (pApp == NULL)
	{
		pApp = new ClientAppRenderer();
	}

	return pApp;
}

void ClientAppRenderer::OnContextCreated(CefRefPtr<CefBrowser> browser,
	CefRefPtr<CefFrame> frame,
	CefRefPtr<CefV8Context> context)
{
	// Register function handlers with the 'window' object.
	message_router_->OnContextCreated(browser, frame, context);

	CefRefPtr<CefV8Value> object = context->GetGlobal();
	CefRefPtr<CefV8Handler> handler = new HtmlEventHandler();
	CefRefPtr<CefV8Value> func = CefV8Value::CreateFunction(JS2CPP_FUNC_NAME, handler);
	object->SetValue(JS2CPP_FUNC_NAME, func, V8_PROPERTY_ATTRIBUTE_NONE);
}

void ClientAppRenderer::OnUncaughtException(CefRefPtr<CefBrowser> browser,
	CefRefPtr<CefFrame> frame,
	CefRefPtr<CefV8Context> context,
	CefRefPtr<CefV8Exception> exception,
	CefRefPtr<CefV8StackTrace> stackTrace)
{
}

bool ClientAppRenderer::OnProcessMessageReceived(
	CefRefPtr<CefBrowser> browser,
	CefProcessId source_process,
	CefRefPtr<CefProcessMessage> message)
{
	return true;
}

void ClientAppRenderer::OnFocusedNodeChanged(CefRefPtr<CefBrowser> browser,
	CefRefPtr<CefFrame> frame,
	CefRefPtr<CefDOMNode> node)  
{
	bool is_editable = (node.get() && node->IsEditable());
	if (is_editable != last_node_is_editable_) {
		// Notify the browser of the change in focused element type.
		last_node_is_editable_ = is_editable;
		CefRefPtr<CefProcessMessage> message =
			CefProcessMessage::Create(kFocusedNodeChangedMessage);
		message->GetArgumentList()->SetBool(0, is_editable);
		browser->SendProcessMessage(PID_BROWSER, message);
	}
}