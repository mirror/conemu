
#include "headers/plugin.hpp"
#include "PictureView_FileName.h"

#define MAX_PIC_PATH_LEN 0x1000



bool PutFilePathName1017(UnicodeFileName* aFullName, const wchar_t* pFileName)
{
	wchar_t* pFullName = aFullName->GetBuffer(MAX_PIC_PATH_LEN);
	
	wchar_t sFullName[MAX_PATH*2], *pszPart=NULL;
	DWORD nCount = sizeof(sFullName)/sizeof(sFullName[0]);
	
	bool result = false;
	DWORD nLen = GetFullPathNameW(pFileName, nCount, sFullName, &pszPart);
	if (!nLen)
		return false; // не смогли?
	if (nLen > nCount) {
		wchar_t* pszBuffer = (wchar_t*)malloc(nLen * sizeof(wchar_t));
		_ASSERTE(pszBuffer);
		nLen = GetFullPathNameW(pFileName, nLen, pszBuffer, &pszPart);
		if (nLen) {
			aFullName->Assign(pszBuffer);
			result = true;
		}
		free(pszBuffer);
	} else {
		aFullName->Assign(sFullName);
		result = true;
	}
	
	return result;
}
