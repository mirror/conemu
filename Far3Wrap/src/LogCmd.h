
#pragma once

#ifdef LOG_COMMANDS
	class LogCmd
	{
		public:
		LPCWSTR pszInfo;
		wchar_t szFile[64];
		public:
		void Dump(LPCWSTR asFmt)
		{
			wchar_t szFull[1024];
			SYSTEMTIME st; GetLocalTime(&st); DWORD nTID = GetCurrentThreadId();
			wsprintf(szFull, asFmt, szFile, nTID, st.wHour, st.wMinute, st.wSecond, pszInfo);
			OutputDebugString(szFull);
		};
		LogCmd(LPCWSTR asFunc, LPWSTR asFile)
		{
			pszInfo = asFunc;
			lstrcpyn(szFile, asFile, ARRAYSIZE(szFile));
			Dump(L"%s:T%u(%u:%02u:%02u) %s\n");
		}
		~LogCmd()
		{
			Dump(L"%s:T%u(%u:%02u:%02u) -end- %s\n");
		};
	};
	#define LOG_CMD_(f,a1,a2,a3) \
		wchar_t szInfo[512]; wsprintf(szInfo, f, a1,a2,a3); \
		LogCmd llLogCmd(szInfo, this ? this->ms_File : L"<wpi==NULL>");
	#define LOG_CMD(f,a1,a2,a3)  //LOG_CMD_(f,a1,a2,a3)
	#define LOG_CMD0(f,a1,a2,a3) //LOG_CMD_(f,a1,a2,a3)
#else
	#define LOG_CMD(f,a1,a2,a3)
	#define LOG_CMD0(f,a1,a2,a3)
#endif
