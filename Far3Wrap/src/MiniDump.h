
#pragma once

#include <Dbghelp.h>

void DebugDump(LPCWSTR asText, LPCWSTR asTitle)
{
	typedef BOOL (WINAPI* MiniDumpWriteDump_t)(HANDLE hProcess, DWORD ProcessId, HANDLE hFile, MINIDUMP_TYPE DumpType,
	        PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam, PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam,
	        PMINIDUMP_CALLBACK_INFORMATION CallbackParam);
	MiniDumpWriteDump_t MiniDumpWriteDump_f = NULL;

	HANDLE hDmpFile = NULL;
	HMODULE hDbghelp = NULL;
	wchar_t szErrInfo[MAX_PATH*4];
	wchar_t dmpfile[MAX_PATH]; dmpfile[0] = 0;
	
	GetTempPath(MAX_PATH-32, dmpfile);
	if (dmpfile[0] && dmpfile[lstrlenW(dmpfile)-1] != L'\\')
		lstrcatW(dmpfile, L"\\");
	wsprintfW(dmpfile+lstrlenW(dmpfile), L"Far3Wrap-%u.mdmp", GetCurrentProcessId());

	hDmpFile = CreateFileW(dmpfile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL|FILE_FLAG_WRITE_THROUGH, NULL);

	if (hDmpFile == INVALID_HANDLE_VALUE)
	{
		DWORD nErr = GetLastError();
		wsprintfW(szErrInfo, L"Can't create debug dump file\n%s\nErrCode=0x%08X", dmpfile, nErr);
		MessageBoxW(NULL, szErrInfo, L"Far3wrap", MB_SYSTEMMODAL|MB_ICONSTOP);
		return;
	}

	if (!hDbghelp)
	{
		hDbghelp = LoadLibraryW(L"Dbghelp.dll");

		if (hDbghelp == NULL)
		{
			DWORD nErr = GetLastError();
			wsprintfW(szErrInfo, L"Can't load debug library 'Dbghelp.dll'\nErrCode=0x%08X", nErr);
			MessageBoxW(NULL, szErrInfo, L"Far3wrap", MB_SYSTEMMODAL|MB_ICONSTOP);
			return;
		}
	}

	if (!MiniDumpWriteDump_f)
	{
		MiniDumpWriteDump_f = (MiniDumpWriteDump_t)GetProcAddress(hDbghelp, "MiniDumpWriteDump");

		if (!MiniDumpWriteDump_f)
		{
			DWORD nErr = GetLastError();
			wsprintfW(szErrInfo, L"Can't locate 'MiniDumpWriteDump' in library 'Dbghelp.dll'", nErr);
			MessageBoxW(NULL, szErrInfo, L"Far3wrap", MB_ICONSTOP|MB_SYSTEMMODAL);
			return;
		}
	}

	if (MiniDumpWriteDump_f)
	{
		MINIDUMP_EXCEPTION_INFORMATION mei = {GetCurrentThreadId()};
		//EXCEPTION_POINTERS ep = {&evt.u.Exception.ExceptionRecord};
		//ep.ContextRecord = NULL; // Непонятно, откуда его можно взять
		mei.ExceptionPointers = NULL; //&ep;
		mei.ClientPointers = FALSE;
		PMINIDUMP_EXCEPTION_INFORMATION pmei = NULL; // пока
		BOOL lbDumpRc = MiniDumpWriteDump_f(
		                    GetCurrentProcess(), GetCurrentProcessId(),
		                    hDmpFile,
		                    MiniDumpNormal /*MiniDumpWithDataSegs*/,
		                    pmei,
		                    NULL, NULL);

		if (!lbDumpRc)
		{
			DWORD nErr = GetLastError();
			wsprintfW(szErrInfo, L"MiniDumpWriteDump failed.\nErrorCode=0x%08X", nErr);
			MessageBoxW(NULL, szErrInfo, L"Far3wrap", MB_ICONSTOP|MB_SYSTEMMODAL);
			return;
		}
	}

	if (hDmpFile != INVALID_HANDLE_VALUE && hDmpFile != NULL)
	{
		CloseHandle(hDmpFile);
	}

	if (hDbghelp)
	{
		FreeLibrary(hDbghelp);
	}

	wsprintfW(szErrInfo, L"%s\n\nMinidump saved in:\n%s", asText, dmpfile);
	MessageBoxW(NULL, szErrInfo, asTitle, MB_ICONSTOP|MB_SYSTEMMODAL);
}
