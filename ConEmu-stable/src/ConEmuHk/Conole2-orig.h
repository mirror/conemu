//////////////////////////////////////////////////////////////////////////////

bool InjectHookDLL(PROCESS_INFORMATION &pi)
{
	// allocate memory for parameter in the remote process
	wstring				strHookDllPath(GetModulePath(NULL));

	if (::GetFileAttributes(strHookDllPath.c_str()) == INVALID_FILE_ATTRIBUTES) return false;

	CONTEXT		context;
	void*		mem				= NULL;
	size_t		memLen			= 0;
	UINT_PTR	fnLoadLibrary	= NULL;
	size_t		codeSize;
	BOOL		isWow64Process	= FALSE;
#ifdef _WIN64
	WOW64_CONTEXT 	wow64Context;
	DWORD			fnWow64LoadLibrary	= 0;
	::ZeroMemory(&wow64Context, sizeof(WOW64_CONTEXT));
	::IsWow64Process(pi.hProcess, &isWow64Process);
	codeSize = isWow64Process ? 20 : 91;
#else
	codeSize = 20;
#endif

	if (isWow64Process)
	{
		// starting a 32-bit process from a 64-bit console
		strHookDllPath += wstring(L"\\ConsoleHook32.dll");
	}
	else
	{
		// same bitness :-)
		strHookDllPath += wstring(L"\\ConsoleHook.dll");
	}

	::ZeroMemory(&context, sizeof(CONTEXT));
	shared_array<BYTE> code(new BYTE[codeSize + (MAX_PATH*sizeof(wchar_t))]);
	memLen = (strHookDllPath.length()+1)*sizeof(wchar_t);

	if (memLen > MAX_PATH*sizeof(wchar_t)) return false;

	::CopyMemory(code.get() + codeSize, strHookDllPath.c_str(), memLen);
	memLen += codeSize;
#ifdef _WIN64

	if (isWow64Process)
	{
		wow64Context.ContextFlags = CONTEXT_FULL;
		::Wow64GetThreadContext(pi.hThread, &wow64Context);
		mem = ::VirtualAllocEx(pi.hProcess, NULL, memLen, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
		// get 32-bit kernel32
		wstring strConsoleWowPath(GetModulePath(NULL) + wstring(L"\\ConsoleWow.exe"));
		STARTUPINFO siWow;
		::ZeroMemory(&siWow, sizeof(STARTUPINFO));
		siWow.cb			= sizeof(STARTUPINFO);
		siWow.dwFlags		= STARTF_USESHOWWINDOW;
		siWow.wShowWindow	= SW_HIDE;
		PROCESS_INFORMATION piWow;

		if (!::CreateProcess(
		            NULL,
		            const_cast<wchar_t*>(strConsoleWowPath.c_str()),
		            NULL,
		            NULL,
		            FALSE,
		            0,
		            NULL,
		            NULL,
		            &siWow,
		            &piWow))
		{
			return false;
		}

		shared_ptr<void> wowProcess(piWow.hProcess, ::CloseHandle);
		shared_ptr<void> wowThread(piWow.hThread, ::CloseHandle);

		if (::WaitForSingleObject(wowProcess.get(), 5000) == WAIT_TIMEOUT)
		{
			return false;
		}

		::GetExitCodeProcess(wowProcess.get(), reinterpret_cast<DWORD*>(&fnWow64LoadLibrary));
	}
	else
	{
		context.ContextFlags = CONTEXT_FULL;
		::GetThreadContext(pi.hThread, &context);
		mem = ::VirtualAllocEx(pi.hProcess, NULL, memLen, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
		fnLoadLibrary = (UINT_PTR)::GetProcAddress(::GetModuleHandle(L"kernel32.dll"), "LoadLibraryW");
	}

#else
	context.ContextFlags = CONTEXT_FULL;
	::GetThreadContext(pi.hThread, &context);
	mem = ::VirtualAllocEx(pi.hProcess, NULL, memLen, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	fnLoadLibrary = (UINT_PTR)::GetProcAddress(::GetModuleHandle(L"kernel32.dll"), "LoadLibraryW");
#endif
	union
	{
		PBYTE  pB;
		PINT   pI;
		PULONGLONG pL;
	} ip;
	ip.pB = code.get();
#ifdef _WIN64

	if (isWow64Process)
	{
		*ip.pB++ = 0x68;			// push  eip
		*ip.pI++ = wow64Context.Eip;
		*ip.pB++ = 0x9c;			// pushf
		*ip.pB++ = 0x60;			// pusha
		*ip.pB++ = 0x68;			// push  "path\to\our.dll"
		*ip.pI++ = (DWORD)mem + codeSize;
		*ip.pB++ = 0xe8;			// call  LoadLibraryW
		*ip.pI++ = (DWORD)fnWow64LoadLibrary - ((DWORD)mem + (ip.pB+4 - code.get()));
		*ip.pB++ = 0x61;			// popa
		*ip.pB++ = 0x9d;			// popf
		*ip.pB++ = 0xc3;			// ret
		::WriteProcessMemory(pi.hProcess, mem, code.get(), memLen, NULL);
		::FlushInstructionCache(pi.hProcess, mem, memLen);
		wow64Context.Eip = (DWORD)mem;
		::Wow64SetThreadContext(pi.hThread, &wow64Context);
	}
	else
	{
		*ip.pL++ = context.Rip;
		*ip.pL++ = fnLoadLibrary;
		*ip.pB++ = 0x9C;					// pushfq
		*ip.pB++ = 0x50;					// push  rax
		*ip.pB++ = 0x51;					// push  rcx
		*ip.pB++ = 0x52;					// push  rdx
		*ip.pB++ = 0x53;					// push  rbx
		*ip.pB++ = 0x55;					// push  rbp
		*ip.pB++ = 0x56;					// push  rsi
		*ip.pB++ = 0x57;					// push  rdi
		*ip.pB++ = 0x41; *ip.pB++ = 0x50;	// push  r8
		*ip.pB++ = 0x41; *ip.pB++ = 0x51;	// push  r9
		*ip.pB++ = 0x41; *ip.pB++ = 0x52;	// push  r10
		*ip.pB++ = 0x41; *ip.pB++ = 0x53;	// push  r11
		*ip.pB++ = 0x41; *ip.pB++ = 0x54;	// push  r12
		*ip.pB++ = 0x41; *ip.pB++ = 0x55;	// push  r13
		*ip.pB++ = 0x41; *ip.pB++ = 0x56;	// push  r14
		*ip.pB++ = 0x41; *ip.pB++ = 0x57;	// push  r15
		*ip.pB++ = 0x48;					// sub   rsp, 40
		*ip.pB++ = 0x83;
		*ip.pB++ = 0xEC;
		*ip.pB++ = 0x28;
		*ip.pB++ = 0x48;					// lea	 ecx, "path\to\our.dll"
		*ip.pB++ = 0x8D;
		*ip.pB++ = 0x0D;
		*ip.pI++ = 40;
		*ip.pB++ = 0xFF;					// call  LoadLibraryW
		*ip.pB++ = 0x15;
		*ip.pI++ = -49;
		*ip.pB++ = 0x48;					// add   rsp, 40
		*ip.pB++ = 0x83;
		*ip.pB++ = 0xC4;
		*ip.pB++ = 0x28;
		*ip.pB++ = 0x41; *ip.pB++ = 0x5F;	// pop   r15
		*ip.pB++ = 0x41; *ip.pB++ = 0x5E;	// pop   r14
		*ip.pB++ = 0x41; *ip.pB++ = 0x5D;	// pop   r13
		*ip.pB++ = 0x41; *ip.pB++ = 0x5C;	// pop   r12
		*ip.pB++ = 0x41; *ip.pB++ = 0x5B;	// pop   r11
		*ip.pB++ = 0x41; *ip.pB++ = 0x5A;	// pop   r10
		*ip.pB++ = 0x41; *ip.pB++ = 0x59;	// pop   r9
		*ip.pB++ = 0x41; *ip.pB++ = 0x58;	// pop   r8
		*ip.pB++ = 0x5F;					// pop	 rdi
		*ip.pB++ = 0x5E;					// pop	 rsi
		*ip.pB++ = 0x5D;					// pop	 rbp
		*ip.pB++ = 0x5B;					// pop	 rbx
		*ip.pB++ = 0x5A;					// pop	 rdx
		*ip.pB++ = 0x59;					// pop	 rcx
		*ip.pB++ = 0x58;					// pop	 rax
		*ip.pB++ = 0x9D;					// popfq
		*ip.pB++ = 0xff;					// jmp	 Rip
		*ip.pB++ = 0x25;
		*ip.pI++ = -91;
		::WriteProcessMemory(pi.hProcess, mem, code.get(), memLen, NULL);
		::FlushInstructionCache(pi.hProcess, mem, memLen);
		context.Rip = (UINT_PTR)mem + 16;
		::SetThreadContext(pi.hThread, &context);
	}

#else
	*ip.pB++ = 0x68;			// push  eip
	*ip.pI++ = context.Eip;
	*ip.pB++ = 0x9c;			// pushf
	*ip.pB++ = 0x60;			// pusha
	*ip.pB++ = 0x68;			// push  "path\to\our.dll"
	*ip.pI++ = (UINT_PTR)mem + codeSize;
	*ip.pB++ = 0xe8;			// call  LoadLibraryW
	*ip.pI++ = (UINT_PTR)fnLoadLibrary - ((UINT_PTR)mem + (ip.pB+4 - code.get()));
	*ip.pB++ = 0x61;			// popa
	*ip.pB++ = 0x9d;			// popf
	*ip.pB++ = 0xc3;			// ret
	::WriteProcessMemory(pi.hProcess, mem, code.get(), memLen, NULL);
	::FlushInstructionCache(pi.hProcess, mem, memLen);
	context.Eip = (UINT_PTR)mem;
	::SetThreadContext(pi.hThread, &context);
#endif
	return true;
}
