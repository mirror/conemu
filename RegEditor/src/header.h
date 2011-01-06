
#pragma once

#ifdef _DEBUG
	#define MVALIDATE_POINTERS
	#define MVALIDATE_HEAP
#else
	#undef MVALIDATE_POINTERS
	#undef MVALIDATE_HEAP
#endif
//#undef MVALIDATE_POINTERS
//#undef MVALIDATE_HEAP


#ifdef _WIN32_WINNT
#undef _WIN32_WINNT
#endif
#define _WIN32_WINNT 0x0500

#include <windows.h>
#include <stdlib.h>
#include <tchar.h>
#ifdef _DEBUG
	#include <crtdbg.h>
#else
	#define _ASSERT(x)
	#define _ASSERTE(x)
#endif

#ifndef _CRT_WIDE
#define __CRT_WIDE(_String) L ## _String
#define _CRT_WIDE(_String) __CRT_WIDE(_String)
#endif


#if (defined(__GNUC__)) || (defined(_MSC_VER) && _MSC_VER<1600)
#define nullptr NULL
#endif

typedef unsigned __int64 u64;
typedef const BYTE *LPCBYTE;


#include "RE_Helper.h"


#ifdef _UNICODE
	#include "common/unicode/pluginW.hpp"
	#include "common/unicode/farcolor.hpp"
#else
	#include "common/ascii/pluginA.hpp"
	#include "common/ascii/farcolor.hpp"
#endif
#include "common/DlgBuilder.hpp"
#include "common/FarHelper.h"

#if defined(_MSC_VER) && defined(PRINT_FAR_VERSION)
	#ifndef FAR_VERSION_PRINTED
		#define FAR_VERSION_PRINTED
	    #define VSTRING2(x) #x
	    #define VSTRING(x) VSTRING2(x)
	    #ifdef _WIN64
	    	#define xPlatform " x64"
	    #else
	    	#define xPlatform " x86"
	    #endif
	    #define PRINT_FAR_VERSION_(m,l,b) __pragma(message ("Using plugin.hpp for FAR Manager " VSTRING(m) "." VSTRING(l) " build " VSTRING(b) xPlatform))
		PRINT_FAR_VERSION_(FARMANAGERVERSION_MAJOR,FARMANAGERVERSION_MINOR,FARMANAGERVERSION_BUILD)
	    #undef VSTRING
	    #undef VSTRING2
	#endif
#endif

#include "RegEditor_Lang.h"


extern struct PluginStartupInfo psi;
extern struct FarStandardFunctions FSF;






// Forward classes.
class RegConfig;
class REPlugin;
class MRegistryBase;
class MRegistryWinApi;
class MFileHive;
class MFileReg;
class MFileTxt;



// InternalMalloc
#include "RE_Malloc.h"
// Настройки
#include "RegConfig.h"
// Прогресс
#include "RE_Progress.h"
// Объекты для работы с реестром и *.reg файлами
#include "REG_Path.h"
#include "REG_Folder.h"
#include "REG_Cache.h"
#include "REG_Base.h"
#include "REG_FileReg.h"
#include "REG_FileTxt.h"
#include "REG_WinApi.h"
#include "REG_FileHive.h"
// Класс плагина
#include "RE_Plugin.h"
#include "RE_PluginList.h"
