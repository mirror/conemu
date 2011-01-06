
/*
Copyright (c) 2009-2010 Maximus5
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
*/

#define DROP_SETCP_ON_WIN2K3R2

// Иначе не опередяется GetConsoleAliases (хотя он должен быть доступен в Win2k)
#undef _WIN32_WINNT
#define _WIN32_WINNT 0x0501

#include <windows.h>
#include <Tlhelp32.h>
#include "common.hpp"
#include "SetHook.h"

#define DebugString(x) // OutputDebugString(x)

static HMODULE hOurModule = NULL; // Хэндл нашей dll'ки (здесь хуки не ставятся)
static DWORD   nMainThreadId = 0;

static const wchar_t kernel32[] = L"kernel32.dll";
static const wchar_t user32[]   = L"user32.dll";

// Forward declarations of the hooks
static FARPROC WINAPI OnGetProcAddress(HMODULE hModule, LPCSTR lpProcName);
BOOL __stdcall OnGetClientRect(HWND hwnd,LPRECT lpRect);
int  __stdcall OnMapWindowPoints(HWND hwndFrom,HWND hwndTo,LPPOINT lpPoints,UINT cPoints);
HWND __stdcall OnCreateWindowExA(DWORD dwExStyle,LPCTSTR lpClassName,LPCTSTR lpWindowName,DWORD dwStyle,int x,int y,int nWidth,int nHeight,HWND hWndParent,HMENU hMenu,HINSTANCE hInstance,LPVOID lpParam);
BOOL __stdcall OnMoveWindow(HWND hWnd,int X,int Y,int nWidth,int nHeight,BOOL bRepaint);
BOOL __stdcall OnShowWindow(HWND hWnd,int nCmdShow);
#ifdef _DEBUG
BOOL __stdcall OnReadConsoleInputExW(HANDLE hConsoleInput, PINPUT_RECORD lpBuffer, DWORD nLength, LPDWORD lpNumberOfEventsRead, DWORD dwReserved); //(DWORD,DWORD,DWORD,DWORD,DWORD); //HANDLE hConsoleInput, PINPUT_RECORD lpBuffer, DWORD nLength, LPDWORD lpNumberOfEventsRead);
ATOM __stdcall OnRegisterClassA(CONST WNDCLASSA *lpWndClass);
#endif



#define MAX_HOOKED_PROCS 50
static HookItem Hooks[MAX_HOOKED_PROCS] = {
	/* ************************ */
	{(void*)OnGetProcAddress,		"GetProcAddress",		kernel32, 408},
	// в плагине - обман. там импортируется ReadConsoleInputExW по Ordinal.
#ifdef _DEBUG
	{(void*)OnReadConsoleInputExW,	"ReadConsoleInputExW",	kernel32, 670},
#endif
	/* ************************ */
	{(void*)OnGetClientRect,		"GetClientRect",		user32, 225},
	{(void*)OnMapWindowPoints,		"MapWindowPoints",		user32, 473},
	{(void*)OnCreateWindowExA,		"CreateWindowExA",		user32, 96},
	{(void*)OnMoveWindow,			"MoveWindow",			user32, 491},
	{(void*)OnShowWindow,			"ShowWindow",			user32, 654},
#ifdef _DEBUG
	{(void*)OnRegisterClassA,		"RegisterClassA",		user32, 534},
#endif
	/* ************************ */
	{0}
};




// Используется в том случае, если требуется выполнить оригинальную функцию, без нашей обертки
// пример в OnPeekConsoleInputW
void* GetOriginalAddress( void* OurFunction, void* DefaultFunction, BOOL abAllowModified, HookItem** ph )
{
	for( int i = 0; Hooks[i].NewAddress; i++ ) {
		if ( Hooks[i].NewAddress == OurFunction) {
			*ph = &(Hooks[i]);
			return (abAllowModified && Hooks[i].ExeOldAddress) ? Hooks[i].ExeOldAddress : Hooks[i].OldAddress;
		}
	}
    _ASSERT(FALSE); // сюда мы попадать не должны
    return DefaultFunction;
}

class CInFuncCall
{
public:
	int *mpn_Counter;
public:
	CInFuncCall() {
		mpn_Counter = NULL;
	}
	
	BOOL Inc(int* pnCounter) {
		BOOL lbFirstCall = FALSE;
		mpn_Counter = pnCounter;
		if (mpn_Counter) {
			lbFirstCall = (*mpn_Counter) == 0;
			(*mpn_Counter)++;
		}
		return lbFirstCall;
	}

	~CInFuncCall() {
		if (mpn_Counter && (*mpn_Counter)>0) (*mpn_Counter)--;
	}
};

#define ORIGINAL(n) \
	static HookItem *ph = NULL; \
	BOOL bMainThread = (GetCurrentThreadId() == nMainThreadId); \
	void* f##n = NULL; /* static низя - функции могут различаться по потокам */ \
	static int nMainThCounter = 0; CInFuncCall CounterLocker; \
	BOOL bAllowModified = bMainThread; \
	if (bMainThread) bAllowModified = CounterLocker.Inc(&nMainThCounter); \
	if (bAllowModified) { \
		static void* f##n##Mod = NULL; \
		if ((f##n##Mod)==NULL) f##n##Mod = (void*)GetOriginalAddress((void*)(On##n) , (void*)n , TRUE, &ph); \
		f##n = f##n##Mod; \
	} else { \
		static void* f##n##Org = NULL; \
		if ((f##n##Org)==NULL) f##n##Org = (void*)GetOriginalAddress((void*)(On##n) , (void*)n , FALSE, &ph); \
		f##n = f##n##Org; \
	} \
	_ASSERTE((void*)(On##n)!=(void*)(f##n) && (void*)(f##n)!=NULL);
	
#define ORIGINALFAST(n) \
	static HookItem *ph = NULL; \
	static void* f##n = NULL; \
	if ((f##n)==NULL) f##n = (void*)GetOriginalAddress((void*)(On##n) , (void*)n , FALSE, &ph); \
	_ASSERTE((void*)(On##n)!=(void*)(f##n) && (void*)(f##n)!=NULL);

#define F(n) ((On##n##_t)f##n)


#define SETARGS(r) HookCallbackArg args = {bMainThread}; args.lpResult = (LPVOID)(r)
#define SETARGS1(r,a1) SETARGS(r); args.lArguments[0] = (DWORD_PTR)(a1)
#define SETARGS2(r,a1,a2) SETARGS1(r,a1); args.lArguments[1] = (DWORD_PTR)(a2)
#define SETARGS3(r,a1,a2,a3) SETARGS2(r,a1,a2); args.lArguments[2] = (DWORD_PTR)(a3)
#define SETARGS4(r,a1,a2,a3,a4) SETARGS3(r,a1,a2,a3); args.lArguments[3] = (DWORD_PTR)(a4)
#define SETARGS5(r,a1,a2,a3,a4,a5) SETARGS4(r,a1,a2,a3,a4); args.lArguments[4] = (DWORD_PTR)(a5)
#define SETARGS6(r,a1,a2,a3,a4,a5,a6) SETARGS5(r,a1,a2,a3,a4,a5); args.lArguments[5] = (DWORD_PTR)(a6)
#define SETARGS7(r,a1,a2,a3,a4,a5,a6,a7) SETARGS6(r,a1,a2,a3,a4,a5,a6); args.lArguments[6] = (DWORD_PTR)(a7)
	

// Заполнить поле HookItem.OldAddress (реальные процедуры из внешних библиотек)
// apHooks->Name && apHooks->DllName MUST be static for a lifetime
bool __stdcall InitHooks( HookItem* apHooks )
{
	int i, j;
	bool skip;
	
    if( apHooks )
    {
    	for (i = 0; apHooks[i].NewAddress; i++) {
    		if (apHooks[i].Name==NULL || apHooks[i].DllName==NULL) {
    			_ASSERTE(apHooks[i].Name!=NULL && apHooks[i].DllName!=NULL);
    			break;
    		}
    		skip = false;

    		for (j = 0; Hooks[j].NewAddress; j++) {
    			if (Hooks[j].NewAddress == apHooks[i].NewAddress) {
    				skip = true; break;
    			}
    		}
    		if (skip) continue;

    		j = 0; // using while, because of j
    		while (Hooks[j].NewAddress) {
    			if (!lstrcmpiA(Hooks[j].Name, apHooks[i].Name)
    				&& !lstrcmpiW(Hooks[j].DllName, apHooks[i].DllName))
				{
					Hooks[j].NewAddress = apHooks[i].NewAddress;
					skip = true; break;
				}
				j++;
    		}
    		if (skip) continue;
    		
    		if (j >= 0) {
    			if ((j+1) >= MAX_HOOKED_PROCS) {
    				// Превышено допустимое количество
    				_ASSERTE((j+1) < MAX_HOOKED_PROCS);
    				continue; // может какие другие хуки удастся обновить, а не добавить
    			}
    			Hooks[j].Name = apHooks[i].Name;
    			Hooks[j].DllName = apHooks[i].DllName;
				Hooks[j].NewAddress = apHooks[i].NewAddress;
				Hooks[j+1].Name = NULL; // на всякий
				Hooks[j+1].NewAddress = NULL; // на всякий
    		}
    	}
    }

    for (i = 0; Hooks[i].NewAddress; i++)
    {
        if( !Hooks[i].OldAddress )
        {
        	// Сейчас - не загружаем
            HMODULE mod = GetModuleHandleW( Hooks[i].DllName );
            if( mod ) {
                Hooks[i].OldAddress = (void*)GetProcAddress( mod, Hooks[i].Name );
                Hooks[i].hDll = mod;
            }
        }
    }
    
    return true;
}


bool cmpmod(const char* pszA, const wchar_t* pszW)
{
	int i;
	for (i = 0; i<14; i++) {
		if ((pszA[i] & ~0x20) != (pszW[i] & ~0x20))
			return false;
		if (pszA[i] == 0 || pszW[i] ==0)
			return true;
	}
	return true;
}

#define GetPtrFromRVA(rva,pNTHeader,imageBase) (PVOID)((imageBase)+(rva))

// Подменить Импортируемые функции в модуле
static bool SetHook( HMODULE Module, BOOL abExecutable )
{
    IMAGE_IMPORT_DESCRIPTOR* Import = 0;
    DWORD Size = 0;
    //HMODULE hExecutable = GetModuleHandle( 0 );
    //if( !Module )
    //    Module = hExecutable;
    //BOOL bExecutable = abExecutable;
    IMAGE_DOS_HEADER* dos_header = (IMAGE_DOS_HEADER*)Module;
    IMAGE_NT_HEADERS* nt_header = NULL;
    if( dos_header->e_magic == IMAGE_DOS_SIGNATURE /*'ZM'*/ )
    {
        nt_header = (IMAGE_NT_HEADERS*)((char*)Module + dos_header->e_lfanew);
        if( nt_header->Signature != 0x004550 )
            return false;
        else
        {
            Import = (IMAGE_IMPORT_DESCRIPTOR*)((char*)Module +
                                         (DWORD)(nt_header->OptionalHeader.
                                         DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].
                                         VirtualAddress));
            Size = nt_header->OptionalHeader.
                                         DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size;
        }
    }
    else
        return false;

    // if wrong module or no import table
    if( Module == INVALID_HANDLE_VALUE || !Import )
        return false;

#ifdef _DEBUG
	PIMAGE_SECTION_HEADER section = IMAGE_FIRST_SECTION(nt_header);
#endif

	#ifdef _WIN64
	_ASSERTE(sizeof(DWORD_PTR)==8);
	#else
	_ASSERTE(sizeof(DWORD_PTR)==4);
	#endif

	#ifdef _WIN64
		#define TOP_SHIFT 60
	#else
		#define TOP_SHIFT 28
	#endif

	TODO("!!! Сохранять ORDINAL процедур !!!");

    bool res = false, bHooked = false;
	int i;
	int nCount = Size / sizeof(IMAGE_IMPORT_DESCRIPTOR);
	//_ASSERTE(Size == (nCount * sizeof(IMAGE_IMPORT_DESCRIPTOR))); -- ровно быть не обязано
    for( i = 0; i < nCount; i++ )
    {
		if (Import[i].Name == 0)
			break;
        //DebugString( ToTchar( (char*)Module + Import[i].Name ) );
		//#ifdef _DEBUG
		char* mod_name = (char*)Module + Import[i].Name;
		//#endif

		DWORD_PTR rvaINT = Import[i].OriginalFirstThunk;
		DWORD_PTR rvaIAT = Import[i].FirstThunk;
		if ( rvaINT == 0 )   // No Characteristics field?
		{
			// Yes! Gotta have a non-zero FirstThunk field then.
			rvaINT = rvaIAT;
			if ( rvaINT == 0 ) {  // No FirstThunk field?  Ooops!!!
				_ASSERTE(rvaINT!=0);
				break;
			}
		}

		//PIMAGE_IMPORT_BY_NAME pOrdinalName = NULL, pOrdinalNameO = NULL;
		//PIMAGE_IMPORT_BY_NAME pOrdinalNameO = NULL;
		//IMAGE_IMPORT_BY_NAME** byname = (IMAGE_IMPORT_BY_NAME**)((char*)Module + rvaINT);
        //IMAGE_THUNK_DATA* thunk = (IMAGE_THUNK_DATA*)((char*)Module + rvaIAT);
		IMAGE_THUNK_DATA* thunk = (IMAGE_THUNK_DATA*)GetPtrFromRVA( rvaIAT, nt_header, (PBYTE)Module );
		IMAGE_THUNK_DATA* thunkO = (IMAGE_THUNK_DATA*)GetPtrFromRVA( rvaINT, nt_header, (PBYTE)Module );
		if (!thunk ||  !thunkO) {
			_ASSERTE(thunk && thunkO);
			continue;
		}

		int f = 0;
        for(f = 0 ; thunk->u1.Function; thunk++, thunkO++, f++)
        {
			//const char* pszFuncName = NULL;
			WORD ordinalO = -1;

			if (thunk->u1.Function!=thunkO->u1.Function)
			{
				PWORD pOrdinal = (PWORD)GetPtrFromRVA(thunkO->u1.Ordinal, nt_header, (PBYTE)Module);
				if (pOrdinal)
					ordinalO = *pOrdinal;

				//if ( IMAGE_SNAP_BY_ORDINAL(thunkO->u1.Ordinal) ) {
				//	ordinalO = IMAGE_ORDINAL(thunkO->u1.Ordinal);
				//	pOrdinalNameO = NULL;
				//}
				//TODO("Возможно стоит искать имя функции не только для EXE, но и для всех dll");
				//if (bExecutable) {
				//	if (!IMAGE_SNAP_BY_ORDINAL(thunkO->u1.Ordinal)) {
				//		pOrdinalNameO = (PIMAGE_IMPORT_BY_NAME)GetPtrFromRVA(thunkO->u1.AddressOfData, nt_header, (PBYTE)Module);
				//		BOOL lbValidPtr = !IsBadReadPtr(pOrdinalNameO, sizeof(IMAGE_IMPORT_BY_NAME));
				//		_ASSERTE(lbValidPtr);
				//		if (lbValidPtr) {
				//			lbValidPtr = !IsBadStringPtrA((LPCSTR)pOrdinalNameO->Name, 10);
				//			_ASSERTE(lbValidPtr);
				//			if (lbValidPtr)
				//				pszFuncName = (LPCSTR)pOrdinalNameO->Name;
				//		}
				//	}
				//}
			}

			int j = 0;
			for( j = 0; Hooks[j].Name; j++ )
			{
				//if (Hooks[j].NewAddress == (void*)thunk->u1.Function) {
				//	res = true; // это уже захучено
				//	break;
				//}
				//WARNING("??? сомнение в этом условии");
                //if( !Hooks[j].OldAddress || (void*)thunk->u1.Function != Hooks[j].OldAddress )
				//{
				//	if (!pszFuncName || !bExecutable) {
				//		continue;
				//	} else {
				//		if (strcmp(pszFuncName, Hooks[j].Name))
				//			continue;
				//	}
				//	// OldAddress уже может отличаться от оригинального экспорта библиотеки
				//	// Это происходит например с PeekConsoleIntputW при наличии плагина Anamorphosis
				//	Hooks[j].ExeOldAddress = (void*)thunk->u1.Function;
				//}
				//if (Hooks[j].nOrdinal == 0 && ordinalO != (ULONGLONG)-1)
				//	Hooks[j].nOrdinal = (DWORD)ordinalO;

				if (Hooks[j].nOrdinal != ordinalO || !cmpmod(mod_name,Hooks[j].DllName))
				{
					continue;
				}
				if (Hooks[j].NewAddress == (void*)thunk->u1.Function) {
					res = true; // это уже захучено
					break;
				}


				bHooked = true;
				DWORD old_protect = 0;
				VirtualProtect( &thunk->u1.Function, sizeof( thunk->u1.Function ),
					PAGE_READWRITE, &old_protect );
				thunk->u1.Function = (DWORD_PTR)Hooks[j].NewAddress;
				VirtualProtect( &thunk->u1.Function, sizeof( DWORD ), old_protect, &old_protect );
				#ifdef _DEBUG
				if (bExecutable)
					Hooks[j].ReplacedInExe = TRUE;
				#endif
				//DebugString( ToTchar( Hooks[j].Name ) );
				res = true;
				break;
			}
        }
    }

//#ifdef _DEBUG
//	if (bHooked) {
//		wchar_t szDbg[MAX_PATH*3], szModPath[MAX_PATH*2]; szModPath[0] = 0;
//		GetModuleFileNameW(Module, szModPath, MAX_PATH*2);
//		lstrcpyW(szDbg, L"  ## Hooks was set by conemu: ");
//		lstrcatW(szDbg, szModPath);
//		lstrcatW(szDbg, L"\n");
//		OutputDebugStringW(szDbg);
//	}
//#endif

    return res;
}

// Подменить Импортируемые функции в 0PictureView.dl_
bool __stdcall SetAllHooks( HMODULE ahOurDll, HMODULE ahPicView )
{
	// т.к. SetAllHooks может быть вызван из разных dll - запоминаем однократно
	if (!hOurModule) hOurModule = ahOurDll;
	
	InitHooks ( NULL );

#ifdef _DEBUG
	char szHookProc[128];
	for (int i = 0; Hooks[i].NewAddress; i++)
	{
		wsprintfA(szHookProc, "## %s -> 0x%08X (exe: 0x%X)\n", Hooks[i].Name, (DWORD)Hooks[i].NewAddress, (DWORD)Hooks[i].ExeOldAddress);
		OutputDebugStringA(szHookProc);
	}
#endif
	

    // ID основной нити (должна быть текущей)
    nMainThreadId = GetCurrentThreadId();

	// Теперь - замена
    SetHook( ahPicView, TRUE /* чтобы по имени тоже шел */ );

    return true;
}















/* **************************** *
 *                              *
 *  Далее идут собственно хуки  *
 *                              *
 * **************************** */

typedef FARPROC (WINAPI* OnGetProcAddress_t)(HMODULE hModule, LPCSTR lpProcName);
static FARPROC WINAPI OnGetProcAddress(HMODULE hModule, LPCSTR lpProcName)
{
	ORIGINAL(GetProcAddress);
	FARPROC lpfn = NULL;

	
	if (((DWORD_PTR)lpProcName) <= 0xFFFF) {
		TODO("!!! Обрабатывать и ORDINAL values !!!");

	} else {
		for (int i = 0; Hooks[i].Name; i++)
		{
    		// The spelling and case of a function name pointed to by lpProcName must be identical
    		// to that in the EXPORTS statement of the source DLL's module-definition (.Def) file
			if (Hooks[i].hDll == hModule
        		&& strcmp(Hooks[i].Name, lpProcName) == 0)
			{
        		lpfn = (FARPROC)Hooks[i].NewAddress;
        		break;
			}
		}
	}
    
    if (!lpfn)
    	lpfn = F(GetProcAddress)(hModule, lpProcName);
    
    return lpfn;
}
