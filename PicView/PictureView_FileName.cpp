
#include "PictureView.h"
#include "PictureView_FileName.h"

UnicodeFileName::UnicodeFileName()
{
	mb_IsUNC = false; mn_BufferSize = 0; ms_Buffer = NULL;
}

UnicodeFileName::~UnicodeFileName()
{
	if (ms_Buffer) {
		free(ms_Buffer);
		ms_Buffer = NULL;
	}
}

bool UnicodeFileName::IsEmpty()
{
	if (!ms_Buffer) return true;
	if (!*ms_Buffer) return true;
	return false;
}

const wchar_t* UnicodeFileName::Assign(const wchar_t* asFileName) // может содержать или НЕ содержать префикс
{
	bool lbIsFull = false, lbIsNetwork = false;
	size_t nLen = 0;
	if (asFileName && *asFileName) {
		nLen = lstrlen(asFileName);
		if (asFileName[0] == L'\\' && asFileName[1] == L'\\') {
			if ((asFileName[2] == L'?' || asFileName[2] == L'.') && asFileName[3] == L'\\') {
				lbIsFull = true;
			} else {
				lbIsNetwork = true;
			}
		}
	}
	// Присвоение своего же буфера
	if (lbIsFull && asFileName == ms_Buffer) {
		return ms_Buffer;
	}
	_ASSERTE(asFileName != ms_Buffer);
	// "\\?\" X:\...
	// "\\?\UNC\" server\share\...
	nLen += 10;
	if (!ms_Buffer || !mn_BufferSize || mn_BufferSize <= nLen) {
		if (ms_Buffer) free(ms_Buffer);
		ms_Buffer = (wchar_t*)calloc(nLen,sizeof(wchar_t));
		_ASSERTE(ms_Buffer);
	}
	if (asFileName && *asFileName) {
		if (lbIsFull) {
			lstrcpy(ms_Buffer, asFileName);
		} else {
			lstrcpy(ms_Buffer, L"\\\\?\\");
			if (lbIsNetwork) {
				lstrcpy(ms_Buffer+4, L"UNC\\");
				lstrcpy(ms_Buffer+8, asFileName+2);
			} else {
				lstrcpy(ms_Buffer+4, asFileName);
			}
		}
	} else if (ms_Buffer) {
		ms_Buffer[0] = 0; // иначе - сброс!
	}
	return ms_Buffer;
}


wchar_t* UnicodeFileName::GetBuffer(int nLen0) // Выделить буфер. nLen0 должен включать завершающий '\0'
{
	if (!ms_Buffer || (nLen0 > (int)mn_BufferSize)) {
		mn_BufferSize = nLen0;
		ms_Buffer = (wchar_t*)malloc(nLen0*sizeof(wchar_t));
		_ASSERTE(ms_Buffer);
		ms_Buffer[0] = 0; // очистить
	}
	return ms_Buffer;
}

bool UnicodeFileName::ReleaseBuffer() // Проверить буфер и если он содержит НЕ UNC - поправить
{
	if (!ms_Buffer || !*ms_Buffer)
		return false;
		
	bool lbIsFull = false, lbIsNetwork = false;

	if (ms_Buffer[0] == L'\\' && ms_Buffer[1] == L'\\') {
		if ((ms_Buffer[2] == L'?' || ms_Buffer[2] == L'.') && ms_Buffer[3] == L'\\') {
			lbIsFull = true;
		} else {
			lbIsNetwork = true;
		}
	}
	
	// Если он не UNC - сконвертируем через Assign
	if (!lbIsFull) {
		wchar_t* psz = ms_Buffer; ms_Buffer = NULL; mn_BufferSize = 0;
		Assign ( psz );
		free(psz);
	}
	
	return (ms_Buffer != NULL);
}

bool UnicodeFileName::CompareDir(const wchar_t* asDir)
{
	if (!asDir || !*asDir) return false;
	if (!ms_Buffer || !*ms_Buffer) return false;
	// откинуть префиксы
	SkipPrefix(&asDir);
	const wchar_t* psz = ms_Buffer;
	SkipPrefix(&psz);
	// после откидывания префиксов строка могла обнулиться
	if (!asDir || !*asDir) return false;
	if (!psz || !*psz) return false;
	//
	size_t nLen = lstrlen(asDir);
	TODO("Check CompareDir");
	if (_wcsnicmp(asDir, psz, nLen))
		return false;
	// Если нет слеша - слеш должен быть в файле
	if (asDir[nLen-1] != L'\\' && psz[nLen] != L'\\')
		return false;
	return true;
}


UnicodeFileName& UnicodeFileName::operator=(const wchar_t* as)
{
    Assign(as);
    return *this;
}
UnicodeFileName::operator const wchar_t* () const
{
	if (!ms_Buffer) return L"";
	return ms_Buffer;
}

void UnicodeFileName::SkipPrefix(const wchar_t** rsPath)
{
	bool lbIsFull = false, lbIsNetwork = false;
	const wchar_t* psz = *rsPath;

	if (psz[0] == L'\\' && psz[1] == L'\\') {
		if ((psz[2] == L'?' || psz[2] == L'.') && psz[3] == L'\\') {
			psz += 4;
			if (psz[0] == L'U' && psz[1] == L'N' && psz[2] == L'C' && psz[3] == L'\\') {
				psz += 3;
			}
		} else {
			psz ++; // а от сетевой шары отрезать первый слеш
		}
	}
	
	*rsPath = psz;
}
