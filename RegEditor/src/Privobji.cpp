/*++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Copyright (C) 1997 - 2000.  Microsoft Corporation.  All rights reserved.

   Module Name:

       PrivObSI.CPP


   Description:

       This file contains the implementation of the CPrivSecurity class

--*/

#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif

#include <aclui.h>
//#include "resource.h"
//#include "Main.h"

#define wcsnlen(pString,maxlen) lstrlenW(pString)
#define wcscpy_s(pResult,cch,pString) lstrcpynW(pResult,pString,cch)

//
// from Main.cpp
//
extern HINSTANCE g_hInstance;

//HANDLE GetClientToken( WORD wIndex );


#define INHERIT_FULL            (CONTAINER_INHERIT_ACE | OBJECT_INHERIT_ACE)

typedef enum {
    ACCESS_READ = 100,
    ACCESS_WRITE = 200
} ACCESSTYPE;

// SI_ACCESS flags
#define SI_ACCESS_SPECIFIC  0x00010000L
#define SI_ACCESS_GENERAL   0x00020000L
#define SI_ACCESS_CONTAINER 0x00040000L // general access, container-only
#define SI_ACCESS_PROPERTY  0x00080000L

// Access rights for our private objects
#define ACCESS_READ     1
#define ACCESS_MODIFY   2
#define ACCESS_DELETE   4

#define ACCESS_ALL  ACCESS_READ | ACCESS_MODIFY | ACCESS_DELETE

// define our generic mapping structure for our private objects
//
GENERIC_MAPPING ObjMap =
{
    KEY_READ,
    KEY_WRITE,
    0,
    KEY_ALL_ACCESS
};




#if defined(__GNUC__)
//#if defined(MLIBCRT)
//	#if defined(__GNUC__)
		#define PURE_CALL_NAME __cxa_pure_virtual
		#define PURE_CALL_RTYPE void
		#define PURE_CALL_RET 
//	#else
//		#define PURE_CALL_NAME _purecall
//		#define PURE_CALL_RTYPE int
//		#define PURE_CALL_RET return 0
//	#endif
//	
	#ifdef __cplusplus
	extern "C"{
	#endif
	// Pure virtual functions.
	PURE_CALL_RTYPE _cdecl PURE_CALL_NAME(void)
	{
		PURE_CALL_RET;
	}
	#ifdef __cplusplus
	};
	#endif
//#endif
#endif



//
// DESCRIPTION OF ACCESS FLAG AFFECTS
//
// SI_ACCESS_GENERAL shows up on general properties page
// SI_ACCESS_SPECIFIC shows up on advanced page
// SI_ACCESS_CONTAINER shows on general page IF object is a container
//
// The following array defines the permission names for my objects.
//
SI_ACCESS g_siObjAccesses[] =
{
	{ &GUID_NULL, 
		KEY_QUERY_VALUE|KEY_SET_VALUE|KEY_CREATE_SUB_KEY|KEY_ENUMERATE_SUB_KEYS|KEY_NOTIFY|KEY_CREATE_LINK|
		DELETE|WRITE_DAC|WRITE_OWNER|READ_CONTROL,
		L"Full Control",      SI_ACCESS_GENERAL | SI_ACCESS_SPECIFIC },
	{ &GUID_NULL,
		KEY_QUERY_VALUE|KEY_ENUMERATE_SUB_KEYS|KEY_NOTIFY|READ_CONTROL,
		L"Read",      SI_ACCESS_GENERAL },
	{ &GUID_NULL, KEY_QUERY_VALUE,      L"Query Value",      SI_ACCESS_SPECIFIC },
	{ &GUID_NULL, KEY_SET_VALUE,      L"Set Value",      SI_ACCESS_SPECIFIC },
	{ &GUID_NULL, KEY_CREATE_SUB_KEY,      L"Create Subkey",      SI_ACCESS_SPECIFIC },
	{ &GUID_NULL, KEY_ENUMERATE_SUB_KEYS,      L"Enumerate Subkeys",      SI_ACCESS_SPECIFIC },
	{ &GUID_NULL, KEY_NOTIFY,      L"Notify",      SI_ACCESS_SPECIFIC },
	{ &GUID_NULL, KEY_CREATE_LINK,      L"Create Link",      SI_ACCESS_SPECIFIC },
	{ &GUID_NULL, DELETE,      L"Delete",      SI_ACCESS_SPECIFIC },
    { &GUID_NULL, WRITE_DAC,      L"Write DAC",      SI_ACCESS_SPECIFIC },
	{ &GUID_NULL, WRITE_OWNER,      L"Write Owner",      SI_ACCESS_SPECIFIC },
	{ &GUID_NULL, READ_CONTROL,      L"Read Control",      SI_ACCESS_SPECIFIC },
};

#define g_iObjDefAccess    1   // ACCESS_READ

// The following array defines the inheritance types for my containers.
SI_INHERIT_TYPE g_siObjInheritTypes[] =
{
	{&GUID_NULL, 0,                                         L"This key only"},
	{&GUID_NULL, CONTAINER_INHERIT_ACE,                     L"This key and subkeys"},
	{&GUID_NULL, CONTAINER_INHERIT_ACE | INHERIT_ONLY_ACE,  L"Subkeys only"},
};


HRESULT
LocalAllocString(LPWSTR* ppResult, LPCWSTR pString)
{
	SIZE_T cch;
	//SIZE_T maxlen = 1024;

    if (!ppResult || !pString)
        return E_INVALIDARG;

	cch = wcsnlen(pString, 1024) + 1;
    *ppResult = (LPWSTR)LocalAlloc(LPTR, cch * sizeof(WCHAR));

    if (!*ppResult)
        return E_OUTOFMEMORY;

	wcscpy_s(*ppResult, (int)cch, pString);

    return S_OK;
}

void
LocalFreeString(LPWSTR* ppString)
{
    if (ppString)
    {
        if (*ppString)
            LocalFree(*ppString);
        *ppString = NULL;
    }
}

class CObjSecurity : public ISecurityInformation
{
protected:
    ULONG                   m_cRef;
    DWORD                   m_dwSIFlags;
    PSECURITY_DESCRIPTOR    *m_ppSD;
    //WORD                    m_wClient;
    HANDLE 					mh_Token;
    LPWSTR                  m_pszServerName;
    LPWSTR                  m_pszObjectName;
	HKEY mhk_Root;
	LPCWSTR mpsz_KeyPath;
	DWORD mdw_RegFlags;

public:
    CObjSecurity() : m_cRef(1) {}
    virtual ~CObjSecurity();

    STDMETHOD(Initialize)(DWORD dwFlags,
                          PSECURITY_DESCRIPTOR *ppSD,
                          //WORD wClient,
                          HANDLE hToken,
                          LPCWSTR pszServer,
                          LPCWSTR pszKeyName,
						  HKEY hkRoot, LPCWSTR pszKeyPath, DWORD dwRegFlags);

    // IUnknown methods
    STDMETHOD(QueryInterface)(REFIID, LPVOID *);
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();

    // ISecurityInformation methods
    STDMETHOD(GetObjectInformation)(PSI_OBJECT_INFO pObjectInfo);
    STDMETHOD(GetSecurity)(SECURITY_INFORMATION si,
                           PSECURITY_DESCRIPTOR *ppSD,
                           BOOL fDefault);
    STDMETHOD(SetSecurity)(SECURITY_INFORMATION si,
                           PSECURITY_DESCRIPTOR pSD);
    STDMETHOD(GetAccessRights)(const GUID* pguidObjectType,
                               DWORD dwFlags,
                               PSI_ACCESS *ppAccess,
                               ULONG *pcAccesses,
                               ULONG *piDefaultAccess);
    STDMETHOD(MapGeneric)(const GUID *pguidObjectType,
                          UCHAR *pAceFlags,
                          ACCESS_MASK *pmask);
    STDMETHOD(GetInheritTypes)(PSI_INHERIT_TYPE *ppInheritTypes,
                               ULONG *pcInheritTypes);
    STDMETHOD(PropertySheetPageCallback)(HWND hwnd,
                                         UINT uMsg,
                                         SI_PAGE_TYPE uPage);
};

///////////////////////////////////////////////////////////////////////////////
//
//  This is the entry point function called from our code that establishes
//  what the ACLUI interface is going to need to know.
//
//
///////////////////////////////////////////////////////////////////////////////

extern "C"
HRESULT
CreateObjSecurityInfo(
    DWORD dwFlags,           // e.g. SI_EDIT_ALL | SI_ADVANCED | SI_CONTAINER
    PSECURITY_DESCRIPTOR *ppSD,        // Pointer to security descriptor
    LPSECURITYINFO *ppObjSI,
    //WORD wClient,           // Index for client token
    HANDLE  hToken,
    LPCWSTR pszServerName,  // Name of server on which SIDs will be resolved
    LPCWSTR pszKeyName,     // This is the only way to name my generic objects
	HKEY hkRoot, LPCWSTR pszKeyPath, DWORD dwRegFlags)
{
    HRESULT hr;
    CObjSecurity *psi;

    *ppObjSI = NULL;

    psi = new CObjSecurity();
    if (!psi)
        return E_OUTOFMEMORY;

    hr = psi->Initialize(dwFlags, ppSD, hToken/*wClient*/, pszServerName, pszKeyName, hkRoot, pszKeyPath, dwRegFlags);

    if (SUCCEEDED(hr))
        *ppObjSI = psi;
    else
        delete psi;

    return hr;
}


CObjSecurity::~CObjSecurity()
{
    LocalFreeString(&m_pszServerName);
    LocalFreeString(&m_pszObjectName);


}

STDMETHODIMP
CObjSecurity::Initialize(DWORD                      dwFlags,
                         PSECURITY_DESCRIPTOR       *ppSD,
                         //WORD                       wClient,
                         HANDLE						hToken,
                         LPCWSTR                    pszServer,
                         LPCWSTR                    pszKeyName,
						 HKEY hkRoot, LPCWSTR pszKeyPath, DWORD dwRegFlags)
{
    HRESULT hr;

    m_dwSIFlags = dwFlags;
    m_ppSD = ppSD;
    //m_wClient = wClient;
    mh_Token = hToken;
	mhk_Root = hkRoot;
	mpsz_KeyPath = pszKeyPath;
	mdw_RegFlags = dwRegFlags;

    hr = LocalAllocString(&m_pszServerName, pszServer);

    if(SUCCEEDED(hr))
    {
        hr = LocalAllocString(&m_pszObjectName,pszKeyName);
    }

    return hr;
}


///////////////////////////////////////////////////////////
//
// IUnknown methods
//
///////////////////////////////////////////////////////////

STDMETHODIMP_(ULONG)
CObjSecurity::AddRef()
{
    return ++m_cRef;
}

STDMETHODIMP_(ULONG)
CObjSecurity::Release()
{
    if (--m_cRef == 0)
    {
        delete this;
        return 0;
    }

    return m_cRef;
}

STDMETHODIMP
CObjSecurity::QueryInterface(REFIID riid, LPVOID FAR* ppv)
{
    if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_ISecurityInformation))
    {
        *ppv = (LPSECURITYINFO)this;
        m_cRef++;
        return S_OK;
    }
    else
    {
        *ppv = NULL;
        return E_NOINTERFACE;
    }
}


///////////////////////////////////////////////////////////
//
// ISecurityInformation methods
//
///////////////////////////////////////////////////////////

STDMETHODIMP
CObjSecurity::GetObjectInformation(PSI_OBJECT_INFO pObjectInfo)
{
    pObjectInfo->dwFlags = m_dwSIFlags;
    pObjectInfo->hInstance = g_hInstance;
    pObjectInfo->pszServerName = m_pszServerName;
    pObjectInfo->pszObjectName = m_pszObjectName;

    return S_OK;
}

STDMETHODIMP
CObjSecurity::GetSecurity(SECURITY_INFORMATION si,
                           PSECURITY_DESCRIPTOR *ppSD,
                           BOOL fDefault)
{
    DWORD dwLength = 0;
    DWORD dwErr = 0;

    *ppSD = NULL;

    if (fDefault)
        return E_NOTIMPL;

    //
    // Assume that required privileges have already been enabled
    //
    GetPrivateObjectSecurity(*m_ppSD, si, NULL, 0, &dwLength);
    if (dwLength)
    {
        *ppSD = (PSECURITY_DESCRIPTOR)LocalAlloc(LPTR, dwLength);
        if (*ppSD &&
            !GetPrivateObjectSecurity(*m_ppSD, si, *ppSD, dwLength, &dwLength))
        {
            dwErr = GetLastError();
            LocalFree(*ppSD);
            *ppSD = NULL;
        }
    }
    else
        dwErr = GetLastError();


    return HRESULT_FROM_WIN32(dwErr);
}

STDMETHODIMP
CObjSecurity::SetSecurity(SECURITY_INFORMATION si,
                           PSECURITY_DESCRIPTOR pSD)
{
    DWORD dwErr = 0;

    //
    // Assume that required privileges have already been enabled
    //
    //if (!SetPrivateObjectSecurity(si, pSD, m_ppSD, &ObjMap, mh_Token))
    //    dwErr = GetLastError();

	if (mpsz_KeyPath && *mpsz_KeyPath) {
		HKEY hk;
		dwErr = RegOpenKeyExW(mhk_Root, mpsz_KeyPath, 0, mdw_RegFlags, &hk);
		if (dwErr == 0) {
			dwErr = RegSetKeySecurity(hk, DACL_SECURITY_INFORMATION, pSD);
			RegCloseKey(hk);
		}
	} else {
		dwErr = RegSetKeySecurity(mhk_Root, DACL_SECURITY_INFORMATION, pSD);
	}

	if (dwErr == 0) {
	    DWORD dwLength = 0;
	    //
	    // Assume that required privileges have already been enabled
	    //
	    GetPrivateObjectSecurity(pSD, DACL_SECURITY_INFORMATION, NULL, 0, &dwLength);
	    if (dwLength)
	    {
	    	PSECURITY_DESCRIPTOR pNewSD = (PSECURITY_DESCRIPTOR)LocalAlloc(LPTR, dwLength);
	        if (pNewSD &&
	            !GetPrivateObjectSecurity(pSD, DACL_SECURITY_INFORMATION, pNewSD, dwLength, &dwLength))
	        {
	            dwErr = GetLastError();
	            LocalFree(pNewSD);
	            pNewSD = NULL;
	        } else {
	        	LocalFree((HLOCAL)*m_ppSD);
	        	*m_ppSD = pNewSD;
	        }
	    }
	    else
	        dwErr = GetLastError();
	}

    return HRESULT_FROM_WIN32(dwErr);
}

STDMETHODIMP
CObjSecurity::GetAccessRights(const GUID* /*pguidObjectType*/,
                               DWORD /*dwFlags*/,
                               PSI_ACCESS *ppAccesses,
                               ULONG *pcAccesses,
                               ULONG *piDefaultAccess)
{
    *ppAccesses = g_siObjAccesses;
    *pcAccesses = sizeof(g_siObjAccesses)/sizeof(g_siObjAccesses[0]);
    *piDefaultAccess = g_iObjDefAccess;

    return S_OK;
}

STDMETHODIMP
CObjSecurity::MapGeneric(const GUID* /*pguidObjectType*/,
                         UCHAR * /*pAceFlags*/,
                         ACCESS_MASK *pmask)
{
    MapGenericMask(pmask, &ObjMap);

    return S_OK;
}

STDMETHODIMP
CObjSecurity::GetInheritTypes(PSI_INHERIT_TYPE *ppInheritTypes,
                               ULONG *pcInheritTypes)
{
    *ppInheritTypes = g_siObjInheritTypes;
    *pcInheritTypes = sizeof(g_siObjInheritTypes)/sizeof(g_siObjInheritTypes[0]);

    return S_OK;
}

STDMETHODIMP
CObjSecurity::PropertySheetPageCallback(HWND /*hwnd*/,
                                         UINT /*uMsg*/,
                                         SI_PAGE_TYPE /*uPage*/)
{
    return S_OK;
}
