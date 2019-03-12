#include <sqwin/win/sqwindowmsg.h>
#include <sqwin/sqwin.h>
#include <thread>
#include <sqstd/sqsafemap.h>
#include <sqwin/thread/sqwinthread.h>
#include <sqwin/thread/sqevent.h>
#include <atomic>

namespace snqu{

#define MAX_LOADSTRING 100

SafeMap<int, WindowMsgProc::umsg_call_back_t> m_message_map;
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK CbtFilterHook(int code, WPARAM wParam, LPARAM lParam)
{
	HWND hWnd = (HWND)wParam;
	return 0L;
}

struct WindowMsgProc::impl : public WinRunnable
{
    HINSTANCE m_hInstance;
    char szTitle[MAX_LOADSTRING];				// 标题栏文本
    char szWindowClass[MAX_LOADSTRING];			// 主窗口类名
    std::atomic<HWND> m_hWnd;
    volatile bool is_quit;
    snqu::WinThread *m_thread;
    Event m_end_ev;
    APPBARDATA m_bardata;
    int m_watch_msg_id;
	std::thread::id m_loop_thd;


    impl()
    {
        m_hInstance = NULL;
        m_hWnd = NULL;
        is_quit = true;
        m_thread = NULL;
        m_watch_msg_id = -1;
		m_loop_thd;
    }

    void InitWndParam(const std::string& window_name)
    {
        if (window_name.empty())
        {
            auto temp_name = create_guid();
            strcpy_s(szTitle, MAX_LOADSTRING, temp_name.c_str());
            strcpy_s(szWindowClass, MAX_LOADSTRING, temp_name.c_str());
        }
        else
        {
            strcpy_s(szTitle, MAX_LOADSTRING, window_name.c_str());
            strcpy_s(szWindowClass, MAX_LOADSTRING, window_name.c_str());
        }
    }

    void Run()
    {
		m_hInstance = ::GetModuleHandle(NULL);
		MyRegisterClass();
		HHOOK hHookOldCbtFilter = SetWindowsHookEx(WH_CBT, CbtFilterHook, NULL, GetCurrentThreadId());
		m_hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT, 0, 500, 200, NULL, NULL, m_hInstance, NULL);
		UnhookWindowsHookEx(hHookOldCbtFilter);
		if (!m_hWnd)
		{
			return;
		}

		// 显示窗口    
		ShowWindow(m_hWnd, SW_HIDE);
		// 更新窗口    
		UpdateWindow(m_hWnd);

        //全屏消息监控
        WindowMsgProc::umsg_call_back_t func = nullptr;
        if (-1 != m_watch_msg_id && m_message_map.get(m_watch_msg_id, func))
        {
            memset(&m_bardata, 0, sizeof(APPBARDATA));
            m_bardata.cbSize = sizeof(APPBARDATA);
            m_bardata.hWnd = m_hWnd;
            m_bardata.uCallbackMessage = m_watch_msg_id;
            ::SHAppBarMessage(ABM_NEW, &m_bardata);
        }

        // 消息循环
        MSG msg;
        auto hAccelTable = LoadAccelerators(m_hInstance, MAKEINTRESOURCE(0));
        is_quit = false;
        while (GetMessage(&msg, m_hWnd, 0, 0))
        {
            if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }

            if (is_quit) break;
        }
    }

    ATOM MyRegisterClass()
    {
        WNDCLASSEXA wcex;
        wcex.cbSize = sizeof(WNDCLASSEX);
        wcex.style			= CS_HREDRAW | CS_VREDRAW;
        wcex.lpfnWndProc	= WndProc;
        wcex.cbClsExtra		= 0;
        wcex.cbWndExtra		= 0;
        wcex.hInstance		= m_hInstance;
        wcex.hIcon			= LoadIcon(m_hInstance, MAKEINTRESOURCE(0));
        wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
        wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
        wcex.lpszMenuName	= MAKEINTRESOURCEA(0);
        wcex.lpszClassName	= szWindowClass;
        wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(0));
        return RegisterClassExA(&wcex);
    }
};

WindowMsgProc::WindowMsgProc()
    : m_impl(new impl)
{
}

WindowMsgProc::~WindowMsgProc()
{
    if (m_impl)
    {
        stop();
        m_impl = nullptr;
    }
}

bool WindowMsgProc::exist_msg(int message)
{
    if (!m_impl->is_quit) return false;
    return m_message_map.exist(message);
}

void WindowMsgProc::add_msg_cb(int message, umsg_call_back_t func)
{
    if (!m_impl->is_quit) return;
    m_message_map.SetVal(message, func);
}

void WindowMsgProc::del_msg_cb(int message)
{
    if (!m_impl->is_quit) return;
    m_message_map.erase(message);
}

int WindowMsgProc::start(const std::string& window_name)
{
    if (NULL != m_impl->m_hWnd)
        return -2;

    m_impl->InitWndParam(window_name);

    int ret = 0;

    m_impl->m_thread = new snqu::WinThread(&(*m_impl));
    if (NULL == m_impl->m_thread)
        return -1;
    
    m_impl->m_thread->Start();

    return ret;
}

void WindowMsgProc::loop(const std::string& window_name)
{
    if (NULL != m_impl->m_hWnd)
        return;

	m_impl->m_loop_thd = std::this_thread::get_id();
    m_impl->InitWndParam(window_name);
    m_impl->m_end_ev.Create();
    m_impl->m_end_ev.Reset();
    m_impl->Run();
    m_impl->m_end_ev.Set();
}

bool WindowMsgProc::stop()
{
    if (!m_impl->is_quit)
    {
        if (NULL != m_impl->m_hWnd)
        {
            PostMessageA(m_impl->m_hWnd, WM_USER, NULL, NULL);
            ::DestroyWindow(m_impl->m_hWnd);
            m_impl->m_hWnd = NULL;
        }
        m_impl->is_quit = true;
		if (m_impl->m_thread != nullptr)
		{//异步等待线程退出
			if (!m_impl->m_thread->Join(5000))
				return false;
			SAFE_DELETE(m_impl->m_thread);
		}
        else
        {//同步loop
			if (m_impl->m_loop_thd != std::this_thread::get_id())
			{//非主线程等待结束
				if (!m_impl->m_end_ev.Wait(5000))
					return false;
				m_impl->m_end_ev.Destory();
			}
        }
    }
	return true;
}

HWND WindowMsgProc::get_hwnd()
{
    return m_impl->m_hWnd;
}

bool WindowMsgProc::watch_full_screen(int message, umsg_call_back_t func)
{
    if (message == -1 || m_impl->m_hWnd != NULL)
        return false;

    m_impl->m_watch_msg_id = message;
    add_msg_cb(message, func);

    return true;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{  
    PAINTSTRUCT ps;
    HDC hdc;
    WindowMsgProc::umsg_call_back_t func = nullptr;
    if (m_message_map.get(uMsg, func))
    {
        return func(wParam, lParam);
    }

    switch (uMsg)
    {
    case WM_PAINT:
        hdc = BeginPaint(hWnd, &ps);
        EndPaint(hWnd, &ps);
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
    return 0;
}

}