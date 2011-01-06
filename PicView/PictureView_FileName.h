
#pragma once

#ifdef _DEBUG
	#include <crtdbg.h>
#else
	#ifndef _ASSERTE
		#define _ASSERTE(a)
	#endif
#endif

class UnicodeFileName {
public:
	UnicodeFileName();
	~UnicodeFileName();
	

	const wchar_t* Assign(const wchar_t* asFileName); // может содержать или НЕ содержать префикс

	
	wchar_t* GetBuffer(int nLen0); // Выделить буфер. nLen0 должен включать завершающий '\0'
	
	bool ReleaseBuffer(); // Проверить буфер и если он содержит НЕ UNC - поправить
	
	bool CompareDir(const wchar_t* asDir);
	
	UnicodeFileName& operator=(const wchar_t* as);
	operator const wchar_t* () const;

	static void SkipPrefix(const wchar_t** rsPath);
	
	bool IsEmpty();

private:
	bool mb_IsUNC;
	size_t mn_BufferSize;
	wchar_t* ms_Buffer;
};
