
#pragma once


#ifdef _WIN32_WINNT
#undef _WIN32_WINNT
#endif
#define _WIN32_WINNT 0x0500

#include <windows.h>
#include <stdlib.h>
#include <tchar.h>

#ifdef _UNICODE
	#include "common/unicode/pluginW.hpp"
#else
	#include "common/ascii/pluginA.hpp"
#endif

#ifdef _UNICODE
	#define FILENAMECPY(p,sz) (p).lpwszFileName = _tcsdup(sz);
	#define FILENAMEPTR(p) (p).lpwszFileName
#else
	#define FILENAMECPY(p,sz) strcpy((p).cFileName, sz);
	#define FILENAMEPTR(p) (p).cFileName
#endif
