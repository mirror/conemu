
#pragma once

/**************************************************************************
Copyright (c) 2010 Maximus5
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. The name of the authors may not be used to endorse or promote products
   derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**************************************************************************/



#include "PVDInterface/usetodo.hpp"

#define STRING2(x) #x
#define STRING(x) STRING2(x)
#define FILE_LINE __FILE__ "(" STRING(__LINE__) "): "
#ifdef HIDE_TODO
#define TODO(s) 
#define WARNING(s) 
#else
#define TODO(s) __pragma(message (FILE_LINE "TODO: " s))
#define WARNING(s) __pragma(message (FILE_LINE "warning: " s))
#endif
#define PRAGMA_ERROR(s) __pragma(message (FILE_LINE "error: " s))

#ifdef _DEBUG
	#include <crtdbg.h>

	int MyCrtDbgReportW(int _ReportType, const wchar_t * _Filename, int _LineNumber, const wchar_t * _ModuleName, const wchar_t * _Format);

	#undef _ASSERT_EXPR
	#undef _ASSERTE
	#define _ASSERT_EXPR(expr, msg) \
		(void) ((!!(expr)) || \
		(1 != MyCrtDbgReportW(_CRT_ASSERT, _CRT_WIDE(__FILE__), __LINE__, NULL, msg)) || \
		(_CrtDbgBreak(), 0))
	#define _ASSERTE(expr)  _ASSERT_EXPR((expr), _CRT_WIDE(#expr))

#else
	#ifndef _ASSERTE
	#define _ASSERTE(a)
	#endif
#endif

#undef _ASSERT
#define _ASSERT _ASSERTE

#ifdef _DEBUG

	#include "/VCProject/MLib/MHeap.h"

	#ifdef DEFINE_OPTIONS
		#include "/VCProject/MLib/MHeap.cpp"
	#endif
	
#endif

#ifdef DEFINE_OPTIONS
int MyCrtDbgReportW(int _ReportType, const wchar_t * _Filename, int _LineNumber, const wchar_t * _ModuleName, const wchar_t * _Format)
{
	return _CrtDbgReportW(_ReportType, _Filename, _LineNumber, _ModuleName, _Format);
}
#endif
