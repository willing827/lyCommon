#include "stdafx.h"
#include "AppUI.h"
#include "MainDlg.h"
[!if CHECKBOX_SHELLNOTIFYICON]
#include "trayicon\SShellNotifyIcon.h"
[!endif]


//��PE�ļ����أ�ע����ļ�����·��λ��
#define RES_TYPE 0
//#define SYSRES_TYPE 0
// #define RES_TYPE 0   //PE
// #define RES_TYPE 1   //ZIP
// #define RES_TYPE 2   //7z
// #define RES_TYPE 2   //�ļ�
//ȥ������֧�֣���������Եû���
#if (RES_TYPE==1)
#include "resprovider-zip\zipresprovider-param.h"
#else 
#if (RES_TYPE==2)
#include "resprovider-7zip\zip7resprovider-param.h"
#endif
#endif
#ifdef _DEBUG
#define SYS_NAMED_RESOURCE _T("soui-sys-resourced.dll")
#else
#define SYS_NAMED_RESOURCE _T("soui-sys-resource.dll")
#endif

[!if CHECKBOX_USE_LUA]
#ifdef _DEBUG
#pragma comment(lib,"lua-52d")
#pragma comment(lib,"scriptmodule-luad")
#else
#pragma comment(lib,"lua-52")
#pragma comment(lib,"scriptmodule-lua")
#endif
[!endif]

//����Ψһ��һ��R,UIRES����,ROBJ_IN_CPP��resource.h�ж���ĺꡣ
ROBJ_IN_CPP

HMODULE UI_Engine::m_hInstance = NULL;
HWND UI_Engine::m_hParent = NULL;
//////////////////////////////////////////////////////////////////////////
UI_Engine g_UIEngine;

//////////////////////////////////////////////////////////////////////////
UI_Engine::UI_Engine()
{
	
}

UI_Engine::~UI_Engine()
{
	
}

DWORD WINAPI UI_Engine::RunUI(LPVOID lpParam)
{
	HRESULT hRes = OleInitialize(NULL);
	SASSERT(SUCCEEDED(hRes));

	int nRet = 0;

	SComMgr *pComMgr = new SComMgr;
	TCHAR szCurrentDir[MAX_PATH] = { 0 };
	GetModuleFileName(NULL, szCurrentDir, sizeof(szCurrentDir));

	LPTSTR lpInsertPos = _tcsrchr(szCurrentDir, _T('\\'));
#ifdef _DEBUG
	_tcscpy(lpInsertPos + 1, _T("..\\[!output PROJECT_NAME]"));
#else
	_tcscpy(lpInsertPos + 1, _T("\0"));
#endif
	SetCurrentDirectory(szCurrentDir);
	{
		BOOL bLoaded = FALSE;
		CAutoRefPtr<SOUI::IImgDecoderFactory> pImgDecoderFactory;
		CAutoRefPtr<SOUI::IRenderFactory> pRenderFactory;
[!if RADIO_RANDER_GDI]
		bLoaded = pComMgr->CreateRender_GDI((IObjRef**)&pRenderFactory);
[!else]
		bLoaded = pComMgr->CreateRender_Skia((IObjRef**)&pRenderFactory);
[!endif]
		SASSERT_FMT(bLoaded, _T("load interface [render] failed!"));
		bLoaded = pComMgr->CreateImgDecoder((IObjRef**)&pImgDecoderFactory);
		SASSERT_FMT(bLoaded, _T("load interface [%s] failed!"), _T("imgdecoder"));

		pRenderFactory->SetImgDecoderFactory(pImgDecoderFactory);
		SApplication *theApp = new SApplication(pRenderFactory, m_hInstance);

#if (RES_TYPE != 0)
		//ѡ���˽���Release�汾�����Դ��ϵͳ��Դ��DEBUG��ʼ��ʹ��DLL����
		{
			HMODULE hModSysResource = LoadLibrary(SYS_NAMED_RESOURCE);
			if (hModSysResource)
			{
				CAutoRefPtr<IResProvider> sysResProvider;
				CreateResProvider(RES_PE, (IObjRef**)&sysResProvider);
				sysResProvider->Init((WPARAM)hModSysResource, 0);
				theApp->LoadSystemNamedResource(sysResProvider);
				FreeLibrary(hModSysResource);
			}
			else
			{
				SASSERT(0);
	}
}
#else
		//��ѡ�˸���ϵͳ��Դѡ��
		{
			CAutoRefPtr<IResProvider> pSysResProvider;
			CreateResProvider(RES_PE, (IObjRef**)&pSysResProvider);
			bLoaded = pSysResProvider->Init((WPARAM)m_hInstance, 0);
			SASSERT(bLoaded);
			bLoaded = theApp->LoadSystemNamedResource(pSysResProvider);
			SASSERT(!bLoaded);
		}
#endif

		CAutoRefPtr<IResProvider>   pResProvider;

#if (RES_TYPE != 0)		
		//ѡ���˽���Release�汾�����Դ����DEBUG��ʼ��ʹ���ļ�����
		{
			CreateResProvider(RES_FILE, (IObjRef**)&pResProvider);
			bLoaded = pResProvider->Init((LPARAM)_T("uires"), 0);
			SASSERT(bLoaded);
		}
#else
		{
[!if ResLoaderType == 0]
			CreateResProvider(RES_PE, (IObjRef**)&pResProvider);
			bLoaded = pResProvider->Init((WPARAM)m_hInstance, 0);
			SASSERT(bLoaded);
[!endif]
[!if ResLoaderType == 1]
			bLoaded = pComMgr->CreateResProvider_ZIP((IObjRef**)&pResProvider);
			SASSERT_FMT(bLoaded, _T("load interface [%s] failed!"), _T("resprovider_zip"));
			ZIPRES_PARAM param;
			param.ZipFile(pRenderFactory, _T("uires.zip"), "[!output ZIP_PSW]");
			bLoaded = pResProvider->Init((WPARAM)&param, 0);
			SASSERT(bLoaded);
[!endif]
[!if ResLoaderType == 2]
			bLoaded = pComMgr->CreateResProvider_7ZIP((IObjRef**)&pResProvider);
			SASSERT_FMT(bLoaded, _T("load interface [%s] failed!"), _T("resprovider_zip"));
			ZIP7RES_PARAM param;
			param.ZipFile(pRenderFactory, _T("uires.7z"), "[!output ZIP_PSW]");
			bLoaded = pResProvider->Init((WPARAM)&param, 0);
			SASSERT(bLoaded);
[!endif]
[!if ResLoaderType == 3]
			CreateResProvider(RES_FILE, (IObjRef**)&pResProvider);
			bLoaded = pResProvider->Init((LPARAM)_T("uires"), 0);
			SASSERT(bLoaded);
[!endif]
		}
#endif

		theApp->InitXmlNamedID(namedXmlID, ARRAYSIZE(namedXmlID), TRUE);
		theApp->AddResProvider(pResProvider);
		
[!if CHECKBOX_USE_LUA]
		//����LUA�ű�ģ�顣
#if (defined(DLL_CORE) || defined(LIB_ALL)) && !defined(_WIN64)
		//����LUA�ű�ģ�飬ע�⣬�ű�ģ��ֻ����SOUI�ں�����DLL��ʽ����ʱ����ʹ�á�
		CAutoRefPtr<SOUI::IScriptFactory> pScriptLuaFactory;
		bLoaded = pComMgr->CreateScrpit_Lua((IObjRef**)&pScriptLuaFactory);
		SASSERT_FMT(bLoaded, _T("load interface [%s] failed!"), _T("scirpt_lua"));
		theApp->SetScriptFactory(pScriptLuaFactory);
#endif//DLL_CORE
[!endif]

[!if CHECKBOX_TRANSLATOR_SUPPORT]
		//���ض����Է���ģ�顣
		CAutoRefPtr<ITranslatorMgr> trans;
		bLoaded = pComMgr->CreateTranslator((IObjRef**)&trans);
		SASSERT_FMT(bLoaded, _T("load interface [%s] failed!"), _T("translator"));
		if (trans)
		{//�������Է����
			theApp->SetTranslator(trans);
			pugi::xml_document xmlLang;
			if (theApp->LoadXmlDocment(xmlLang, _T("lang_cn"), _T("translator")))
			{
				CAutoRefPtr<ITranslator> langCN;
				trans->CreateTranslator(&langCN);
				langCN->Load(&xmlLang.child(L"language"), 1);//1=LD_XML
				trans->InstallTranslator(langCN);
			}
		}
[!endif]

		SNotifyCenter *pNotifyCenter = new SNotifyCenter;
		// BLOCK: Run application
		{
			CMainDlg dlgMain;
			dlgMain.Create(m_hParent);
			dlgMain.SendMessage(WM_INITDIALOG);
			dlgMain.CenterWindow(dlgMain.m_hWnd);
			dlgMain.ShowWindow(SW_SHOWNORMAL);

			nRet = theApp->Run(dlgMain.m_hWnd);
		}
		delete pNotifyCenter;
		delete theApp;
	}

	delete pComMgr;

	OleUninitialize();
	return nRet;
}

int UI_Engine::Init_UI(HMODULE hInstance, HWND parent)
{
	m_hInstance = hInstance;
	m_hParent = parent;

	DWORD dwThreadId, dwThrdParam = 1;
	HANDLE hThread = CreateThread(NULL, 0, RunUI, &dwThrdParam, 0, &dwThreadId);
	CloseHandle(hThread);

	return 1;
}
