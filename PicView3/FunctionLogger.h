
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


class CFunctionLogger
{
public:
	~CFunctionLogger();
	CFunctionLogger(LPCWSTR asFunc);
	CFunctionLogger(LPCWSTR asFuncFormat, int nArg1);
	CFunctionLogger(LPCWSTR asFuncFormat, LPCWSTR asArg1);
	static void FunctionLogger(LPCWSTR asFunc);
	static void FunctionLogger(LPCWSTR asFuncFormat, int nArg1);
	static void FunctionLogger(LPCWSTR asFuncFormat, LPCWSTR asArg1);
private:
	wchar_t sInfo[MAX_PATH];
	static void WriteLog(LPCWSTR pszText);
};

#ifdef _DEBUG
	//#define FUNCLOGGER(asFunc) CFunctionLogger flog(asFunc);
	//#define FUNCLOGGERI(asFormat,nArg1) CFunctionLogger flog(asFormat,nArg1);
	//#define FUNCLOGGERS(asFormat,sArg1) CFunctionLogger flog(asFormat,sArg1);
	#define FUNCLOGGER(asFunc) CFunctionLogger::FunctionLogger(asFunc);
	#define FUNCLOGGERI(asFormat,nArg1) CFunctionLogger::FunctionLogger(asFormat,nArg1);
	#define FUNCLOGGERS(asFormat,sArg1) CFunctionLogger::FunctionLogger(asFormat,sArg1);
#else
	#define FUNCLOGGER(asFunc) CFunctionLogger::FunctionLogger(asFunc);
	#define FUNCLOGGERI(asFormat,nArg1) CFunctionLogger::FunctionLogger(asFormat,nArg1);
	#define FUNCLOGGERS(asFormat,sArg1) CFunctionLogger::FunctionLogger(asFormat,sArg1);
#endif
