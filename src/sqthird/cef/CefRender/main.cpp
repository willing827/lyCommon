#include "include/cef_client.h"
#include <DbgHelp.h>
#include <shlobj.h>
#pragma comment(lib, "dbghelp.lib")
#pragma comment( linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"" )
#include "ClientAppRenderer.h"

void CreateDumpFile(LPCWSTR lpstrDumpFilePathName, EXCEPTION_POINTERS *pException)
{
	// ����Dump�ļ�   
	HANDLE hDumpFile = CreateFileW(lpstrDumpFilePathName,
		GENERIC_WRITE,
		0,
		NULL,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	// Dump��Ϣ   
	MINIDUMP_EXCEPTION_INFORMATION dumpInfo;
	dumpInfo.ExceptionPointers = pException;
	dumpInfo.ThreadId = GetCurrentThreadId();
	dumpInfo.ClientPointers = TRUE;

	// д��Dump�ļ�����   
	MiniDumpWriteDump(GetCurrentProcess(),
		GetCurrentProcessId(),
		hDumpFile,
		MiniDumpNormal,
		&dumpInfo,
		NULL,
		NULL);

	CloseHandle(hDumpFile);
}

LONG ApplicationCrashHandler(EXCEPTION_POINTERS *pException)
{
	TCHAR szCurrentDir[MAX_PATH + 1] = { 0 };
	GetModuleFileName(NULL, szCurrentDir, MAX_PATH);
	(wcsrchr(szCurrentDir, TEXT('\\')))[1] = 0;

	TCHAR szDumpFileDir[MAX_PATH];
	wsprintf(szDumpFileDir, TEXT("%s\\Log\\DumpLog"), szCurrentDir);
	SHCreateDirectoryEx(NULL, szDumpFileDir, NULL);

	TCHAR szDumpFilePath[MAX_PATH];
	wsprintf(szDumpFilePath, TEXT("%s\\WebProcess.dmp"), szDumpFileDir);
	CreateDumpFile(szDumpFilePath, pException);

	MessageBox(NULL, szDumpFilePath, TEXT("ѽ����������ˣ�-_-!"), 0);
	TerminateProcess(GetCurrentProcess(), 0);

	return EXCEPTION_EXECUTE_HANDLER;
}

int main(int argc, char* argv[])
{
	SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)ApplicationCrashHandler);

	CefMainArgs main_args(GetModuleHandle(NULL));

	CefRefPtr<ClientAppRenderer> app(new ClientAppRenderer());

	// ִ���ӽ����߼�����ʱ�����ֱ���ӽ����˳���
	return CefExecuteProcess(main_args, app.get(), NULL);
}
