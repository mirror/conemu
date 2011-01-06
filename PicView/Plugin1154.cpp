
#include "headers/plugin1154.hpp"
#include "PictureView_FileName.h"

//#ifdef _DEBUG
//void TTT(void* fsf)
//{
//	wchar_t szDest[MAX_PATH];
//	((FarStandardFunctions*)fsf)->ConvertPath(CPM_REAL, L"P:\\Canon", szDest, MAX_PATH);
//	szDest[0] = 0;
//}
//#endif

bool PutFilePathName1154(UnicodeFileName* pFullName, const wchar_t* pFileName, void* fsf)
{
	if (!pFileName || !*pFileName || !pFullName) {
		_ASSERTE(pFileName && *pFileName);
		return false;
	}

	bool result = false;
	_ASSERTE(pFileName[0] != L'"');
	
	int nLen = (int)((FarStandardFunctions*)fsf)->ConvertPath(CPM_FULL, pFileName, NULL, 0);
	if (nLen > 0) {
		((FarStandardFunctions*)fsf)->ConvertPath(CPM_FULL, pFileName, pFullName->GetBuffer(nLen), nLen);
		result = pFullName->ReleaseBuffer();
	}
	
	return result;
}
