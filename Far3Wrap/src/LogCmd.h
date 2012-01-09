
#pragma once

#ifdef LOG_COMMANDS
	extern DWORD gnMainThreadId;

	class LogCmd
	{
		protected:
		static int nNestLevel;
		public:
		LPCWSTR pszInfo;
		wchar_t szFile[64], szLevel[129];
		public:
		void Dump(LPCWSTR asFmt)
		{
			wchar_t szFull[1024];
			SYSTEMTIME st; GetLocalTime(&st); DWORD nTID = GetCurrentThreadId();
			wsprintf(szFull, asFmt, szFile, nTID, st.wHour, st.wMinute, st.wSecond, szLevel, pszInfo);
			OutputDebugString(szFull);
		};
		LogCmd(LPCWSTR asFunc, LPWSTR asFile)
		{
			pszInfo = asFunc;
			lstrcpyn(szFile, asFile, ARRAYSIZE(szFile));
			szLevel[0] = 0;
			for (int i = 0, j = 0; (i+3) < ARRAYSIZE(szLevel) && j < nNestLevel; i+=2, j++)
			{
				szLevel[i] = szLevel[i+1] = _T(' '); szLevel[i+2] = 0;
			}
			Dump(L"%-24.24s:T%u(%u:%02u:%02u) %s%s\n");
			if (gnMainThreadId == GetCurrentThreadId())
				nNestLevel++;
		}
		~LogCmd()
		{
			Dump(L"%-24.24s:T%u(%u:%02u:%02u) %s-end- %s\n");
			if (gnMainThreadId == GetCurrentThreadId() && nNestLevel > 0)
				nNestLevel--;
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
