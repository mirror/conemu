
#pragma once

#ifdef _DEBUG
	#define TRACK_MEMORY_ALLOCATIONS
#endif

// Heap allocation routines

void * __cdecl operator new(size_t size);
void * __cdecl operator new[] (size_t size);
void __cdecl operator delete(void *block);
void __cdecl operator delete[] (void *ptr);

#ifdef TRACK_MEMORY_ALLOCATIONS
	typedef struct tag_xf_mem_block {
		BOOL   bBlockUsed;
		size_t nBlockSize;
		char   sCreatedFrom[32];
	} xf_mem_block;

	//#define DEBUG_NEW new(THIS_FILE, __LINE__)
	#define TRACK_MEMORY_ARGS LPCSTR lpszFileName, int nLine
	// New Operators
	//void* __cdecl operator new(size_t size, TRACK_MEMORY_ARGS);
	//void * __cdecl operator new[] (size_t size, TRACK_MEMORY_ARGS);
	//void __cdecl operator delete(void *block, TRACK_MEMORY_ARGS);
	//void __cdecl operator delete[] (void *ptr, TRACK_MEMORY_ARGS);

	void * __cdecl xf_calloc(size_t _Count, size_t _Size, LPCSTR lpszFileName, int nLine);
	void __cdecl xf_free(void * _Memory, LPCSTR lpszFileName, int nLine);
	void * __cdecl xf_malloc(size_t _Size, LPCSTR lpszFileName, int nLine);

	void __cdecl xf_dump();

	#define malloc(s) xf_malloc(s,__FILE__,__LINE__)
	#define calloc(c,s) xf_calloc(c,s,__FILE__,__LINE__)
	#define free(p) xf_free(p,__FILE__,__LINE__)

#else

	void * __cdecl xf_calloc(size_t _Count, size_t _Size);
	void __cdecl xf_free(void * _Memory);
	void * __cdecl xf_malloc(size_t _Size);

	#define malloc xf_malloc
	#define calloc xf_calloc
	#define free xf_free

	#define xf_dump()

#endif

bool __cdecl xf_validate(void * _Memory = NULL);

//#ifndef _CRT_WIDE
//#define __CRT_WIDE(_String) L ## _String
//#define _CRT_WIDE(_String) __CRT_WIDE(_String)
//#endif
void InternalInvalidOp(LPCWSTR asFile, int nLine);
#define InvalidOp() InternalInvalidOp(_CRT_WIDE(__FILE__),__LINE__)

#define countof(a) ( (sizeof((a))) / (sizeof(*(a))) )

#ifdef MVALIDATE_POINTERS
	#define SafeFree(p) if ((p)!=NULL) { if (!xf_validate((p))) InvalidOp(); free((p)); (p) = NULL; }
	#define SafeDelete(p) if ((p)!=NULL) { if (!xf_validate((p))) InvalidOp(); delete (p); (p) = NULL; }
#else
	#define SafeFree(p) if ((p)!=NULL) { xf_free((p)); (p) = NULL; }
	#define SafeDelete(p) if ((p)!=NULL) { delete (p); (p) = NULL; }
#endif



//#if (defined(__GNUC__)) || (defined(_MSC_VER) && _MSC_VER<1600)
//#define nullptr NULL
//#endif
//
//typedef unsigned __int64 u64;


#ifdef _UNICODE
	#define REGEDIT_SYNCHRO_DESC_FINISHED 100
	class REPlugin;
	typedef struct tag_SynchroArg
	{
		DWORD     nEvent;
		REPlugin *pPlugin;
	} SynchroArg;
#endif


#define MAX_REGKEY_NAME 256 // считая БЕЗ завершающего '\0'
#define MAX_FILE_NAME 255 // считая БЕЗ завершающего '\0'

#define REGEDIT_MAGIC 0x52674564 // 'RgEd'
#define REGEDIT_DEFAULTNAME L"(Default)"
#define REGEDIT_DEFAULTNAME_T _T("(Default)")
#define REGEDIT_REPLACETWODOTS _T(".. ")


// Для x64 это преобразовывается в 0x8000000000000000 .. 0x8000000000000007
#define HKEY__FIRST                   ( (ULONG_PTR)((LONG)0x80000000) )
#define HKEY__LAST                    ( (ULONG_PTR)((LONG)0x80000007) )
#define HKEY__PREDEFINED_MASK         ( (ULONG_PTR)((LONG)0xF0000000) )
#define HKEY__PREDEFINED_TEST         ( (ULONG_PTR)((LONG)0x80000000) )
// Для удобства просмотра HIVE-файлов
#define HKEY__HIVE                    ( ( HKEY ) (ULONG_PTR)((LONG)0x8000000A) )
#define HKEY__HIVE_NAME               L"HKEY"
#define HKEY__HIVE_NAME_LWR           L"hkey"

// Наши собственные типы данных
//typedef DWORD REGTYPE;
//#define REG__KEY     ((REGTYPE)0xFFFFFFFF)
//#define REG__COMMENT ((REGTYPE)0xFFFFFFFE)
//#define REG__INTERNAL_TYPES REG__COMMENT
typedef unsigned __int64 REGTYPE;
#define REG__KEY     ((REGTYPE)0x100000001)
#define REG__COMMENT ((REGTYPE)0x100000002)
#define REG__DELETE  ((REGTYPE)0x100000003)
#define REG__INTERNAL_TYPES REG__KEY



#ifndef HKEY_CURRENT_CONFIG
#define HKEY_CURRENT_CONFIG                 (( HKEY ) (ULONG_PTR)((LONG)0x80000005) )
#endif
#ifndef HKEY_DYN_DATA
#define HKEY_DYN_DATA                       (( HKEY ) (ULONG_PTR)((LONG)0x80000006) )
#endif
#ifndef HKEY_CURRENT_USER_LOCAL_SETTINGS
#define HKEY_CURRENT_USER_LOCAL_SETTINGS    (( HKEY ) (ULONG_PTR)((LONG)0x80000007) )
#endif


#define REG_ATTRIBUTE_DELETED   (FILE_ATTRIBUTE_OFFLINE)  // 0x1000
#define REG_ATTRIBUTE_INDIRECT  (FILE_ATTRIBUTE_ARCHIVE)

#define REGF_DELETED   REG_ATTRIBUTE_DELETED
#define REGF_INDIRECT  REG_ATTRIBUTE_INDIRECT
#define REGF_ALL_MASK  (REGF_DELETED | REGF_INDIRECT)


#define REG__OPTION_CREATE_DELETED 0x1000000
//#define REG__OPTION_ENUM_COMMENTS  0x2000000

#ifndef KEY_WOW64_64KEY
#define KEY_WOW64_32KEY         (0x0200)
#define KEY_WOW64_64KEY         (0x0100)
#endif

//#define HKEY_CLASSES_ROOT                   (( HKEY ) (ULONG_PTR)((LONG)0x80000000) )
//#define HKEY_CURRENT_USER                   (( HKEY ) (ULONG_PTR)((LONG)0x80000001) )
//#define HKEY_LOCAL_MACHINE                  (( HKEY ) (ULONG_PTR)((LONG)0x80000002) )
//#define HKEY_USERS                          (( HKEY ) (ULONG_PTR)((LONG)0x80000003) )
//#define HKEY_PERFORMANCE_DATA               (( HKEY ) (ULONG_PTR)((LONG)0x80000004) )
//-- #define HKEY_PERFORMANCE_TEXT               (( HKEY ) (ULONG_PTR)((LONG)0x80000050) )
//-- #define HKEY_PERFORMANCE_NLSTEXT            (( HKEY ) (ULONG_PTR)((LONG)0x80000060) )
#ifndef HKEY_CURRENT_CONFIG
#define HKEY_CURRENT_CONFIG                 (( HKEY ) (ULONG_PTR)((LONG)0x80000005) )
#define HKEY_DYN_DATA                       (( HKEY ) (ULONG_PTR)((LONG)0x80000006) )
#define HKEY_CURRENT_USER_LOCAL_SETTINGS    (( HKEY ) (ULONG_PTR)((LONG)0x80000007) )
#endif


bool StringKeyToHKey(LPCWSTR asKey, HKEY* phkey);
bool HKeyToStringKey(HKEY hkey, wchar_t* pszKey, int nMaxLen);
LPCWSTR HKeyToStringKey(HKEY hkey);



enum eCommandAction {
	aInvalid = 0,
	aBrowseLocal,
	aBrowseRemote,
	aBrowseFileReg,
	aBrowseFileHive,
	aExportKeysValues,
	aImportRegFile,
	aMountHive,
	aUnmountHive,
};

enum eExportFormat {
	eExportReg4 = 0,
	eExportReg5 = 1,
	eExportHive = 2,
};

enum KeyFirstEnum {
	eValuesFirst = 0,
	eKeysFirst = 1,
	eKeysOnly = 2,
};

enum RegWorkType {
	RE_UNDEFINED = 0,  // не инициализировано
	RE_WINAPI,         // Работа с реестром локальной машины через WinApi
	RE_REGFILE,        // *.reg
	RE_HIVE,           // «hive» files
};

enum RegKeyOpenRights {
	eRightsSimple = 0,
	eRightsBackup,
	eRightsBackupRestore,
	eRightsNoAdjust,
};

typedef union _REGFILETIME {
	ULONGLONG QuadPart;
	struct {
		DWORD LowPart;
		DWORD HighPart;
	} DUMMYSTRUCTNAME;

	//PFILETIME operator & () {
	//	return (PFILETIME)this;
	//};
	operator FILETIME () {
		return *((PFILETIME)this);
	};
} REGFILETIME;




// Heap checking
#if defined(_DEBUG)
	#ifdef MVALIDATE_HEAP
		#define MCHKHEAP _ASSERT(xf_validate());
	#else
		#define MCHKHEAP
	#endif
#elif defined(MVALIDATE_HEAP)
	#define MCHKHEAP xf_validate();
#else
	#define MCHKHEAP
#endif



// String routines
#undef _tcslen

#ifdef __GNUC__
	#define wmemcmp(p1,p2,s) memcmp(p1,p2,2*(s))
	#define wmemmove(d,s,l) memmove(d,s,l*2)
#else
	#ifndef _UNICODE
		#define wmemcmp(p1,p2,s) memcmp(p1,p2,2*(s))
		#define wmemmove(d,s,l) memmove(d,s,l*2)
	#endif
#endif

#ifdef _UNICODE
	#define _tcslen lstrlenW
    #define _tcsscanf swscanf
    #define _tcstoi _wtoi
	#define _tmemcmp wmemcmp
#else
	#define _tcslen lstrlenA
    #define _tcsscanf sscanf
    #define _tcstoi atoi
	#define _tmemcmp memcmp
#endif

wchar_t* lstrdup(LPCWSTR asText);
wchar_t* lstrdup_w(LPCSTR asText);
#ifndef _UNICODE
	char* lstrdup(LPCSTR asText);
#else
	#define lstrdup_w lstrdup
#endif
TCHAR* lstrdup_t(LPCWSTR asText);
void lstrcpy_t(TCHAR* pszDst, int cMaxSize, const wchar_t* pszSrc);
void lstrcpy_t(wchar_t* pszDst, int cMaxSize, const char* pszSrc);
void CopyFileName(wchar_t* pszDst, int cMaxSize, const wchar_t* pszSrc);
#ifndef _UNICODE
	void CopyFileName(char* pszDst, int cMaxSize, const wchar_t* pszSrc);
#endif
bool IsUNCPath(const char* pszSrc);
bool IsUNCPath(const wchar_t* pszSrc);
bool IsNetworkPath(const char* pszSrc);
bool IsNetworkPath(const wchar_t* pszSrc);
bool IsNetworkUNCPath(const char* pszSrc);
bool IsNetworkUNCPath(const wchar_t* pszSrc);
wchar_t* MakeUNCPath(const char* pszSrc);
wchar_t* MakeUNCPath(const wchar_t* pszSrc);
int CopyUNCPath(wchar_t* pszDest, int cchDestMax, const wchar_t* pszSrc);
wchar_t* UnmakeUNCPath_w(const wchar_t* pszSrc);
#ifndef _UNICODE
char* UnmakeUNCPath_t(const wchar_t* pszSrc);
#else
#define UnmakeUNCPath_t UnmakeUNCPath_w
#endif
const char* PointToName(const char* asFileOrPath);
const wchar_t* PointToName(const wchar_t* asFileOrPath);
const char* PointToExt(const char* asFileOrPath);
const wchar_t* PointToExt(const wchar_t* asFileOrPath);
wchar_t* ExpandPath(LPCWSTR asPathStart);
wchar_t* ExpandPath(LPCSTR asPathStart);
void DebracketRegPath(wchar_t* asRegPath);
bool ValidatePath(LPCWSTR asFullOrRelPath);

BOOL DetectFileFormat(const unsigned char *Data,int DataSize, int* pnFormat, BOOL* pbUnicode, BOOL* pbBOM);
BOOL DetectFileFormat(LPCWSTR apszFileName, int* pnFormat, BOOL* pbUnicode, BOOL* pbBOM);

void FormatDataVisual(REGTYPE nDataType, LPBYTE pData, DWORD dwDataSize, TCHAR* szDesc/*[128]*/);

#ifdef _DEBUG
BOOL SearchTokenGroupsForSID (VOID);
#endif

bool CHREQ(const wchar_t* s, int i, wchar_t c);





class REPlugin;
extern REPlugin *gpActivePlugin; // Устанавливается на время вызова из ФАР функций нашего плагина
extern DWORD gnOpMode;
// Дополнительные флаги для gnOpMode
#define OPM_DISABLE_NONMODAL_EDIT 0x20000

// Установить на время работы функции переменную gpActivePlugin = hPlugin
class CPluginActivator
{
public:
	REPlugin* pBefore;
	REPlugin* pWasSet;
	DWORD nBeforeOpMode;
public:
	CPluginActivator(HANDLE hPlugin, DWORD anOpMode);
	~CPluginActivator();
};





// FAR Macro and Routines
extern TCHAR *GetMsg(int MsgId);

#ifdef _UNICODE
    #define SETMENUTEXT(itm,txt) itm.Text = txt;
    #define F757NA 0, (LONG_PTR)
    #define _GetCheck(i) (int)psi.SendDlgMessage(hDlg,DM_GETCHECK,i,0)
    #define GetDataPtr(i) ((const TCHAR *)psi.SendDlgMessage(hDlg,DM_GETCONSTTEXTPTR,i,0))
    #define SETTEXT(itm,txt) itm.PtrData = txt
    #define SETTEXTPRINT(itm,fmt,arg) wsprintf(pszBuf, fmt, arg); SETTEXT(itm,pszBuf); pszBuf+=lstrlen(pszBuf)+2;
	#define FILENAMEPTR(p) (p).lpwszFileName
#else
    #define SETMENUTEXT(itm,txt) lstrcpy(itm.Text, txt);
    #define F757NA (void*)
    #define _GetCheck(i) items[i].Selected
    #define GetDataPtr(i) items[i].Ptr.PtrData
    #define SETTEXT(itm,txt) lstrcpy(itm.Data, txt)
    #define SETTEXTPRINT(itm,fmt,arg) wsprintf(itm.Data, fmt, arg)
	#define FILENAMEPTR(p) (p).cFileName
#endif
