
#pragma once

bool gbIgnoreAssertStruct = false;
void AssertStructSize(size_t s3, size_t s2, LPCWSTR sName,
					  LPCSTR sFile, int nLine)
{
	if (gbIgnoreAssertStruct)
		return;
	wchar_t szDbgInfo[255], szTitle[64], szFile[64];
	LPCSTR pszName = sFile ? strrchr(sFile, '\\') : "???.cpp";
	if (!pszName) pszName = sFile;
	MultiByteToWideChar(CP_ACP, 0, pszName, -1, szFile, ARRAYSIZE(szFile));
	wsprintf(szDbgInfo, L"Invalid struct %s size\nFar2: %u, Far3: %u\nFile: %s, Line: %u\nPress <Cancel> to stop buzzing",
		sName, (DWORD)s2, (DWORD)s3, szFile, nLine);
	wsprintf(szTitle, L"Far3Wrapper %u.%u #%u", MVV_1,MVV_2,MVV_3);
	if (MessageBox(NULL, szDbgInfo, szTitle, MB_OKCANCEL|MB_ICONSTOP|MB_SYSTEMMODAL) == IDCANCEL)
		gbIgnoreAssertStruct = true;
}
#define ASSERTSTRUCT(s) { if (sizeof(Far2::s)!=sizeof(s)) AssertStructSize(sizeof(s), sizeof(Far2::s), L#s, __FILE__, __LINE__); }
#define ASSERTSTRUCTGT(s) { if (sizeof(Far2::s)>sizeof(s)) AssertStructSize(sizeof(s), sizeof(Far2::s), L#s, __FILE__, __LINE__); }
