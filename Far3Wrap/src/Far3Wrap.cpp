
/*
Copyright (c) 2011 Maximus5
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

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ''AS IS'' AND ANY EXPRESS OR
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


#include <windows.h>
#include <rpc.h>
#include <TCHAR.h>
#include <map>
#include <crtdbg.h>

#define _FAR_NO_NAMELESS_UNIONS

namespace Far2
{
//int gi1 = 0;
#undef __PLUGIN_HPP__
#include "pluginW.hpp"
#undef __FARKEYS_HPP__
#include "farkeys.hpp"
//int gi2 = 0;
};

//namespace Far3
//{
//int gi1 = 0;
#undef __PLUGIN_HPP__
#include "pluginW3.hpp"
//int gi2 = 0;
//};


#include <malloc.h>
#include <TChar.h>


GUID guid_DefPlugin = { /* 3a446422-cc89-4d74-87f9-a5e0db2c4486 */                                                  
    0x3a446422,                                                                                               
    0xcc89,                                                                                                   
    0x4d74,                                                                                                   
    {0x87, 0xf9, 0xa5, 0xe0, 0xdb, 0x2c, 0x44, 0x86}                                                          
};

GUID guid_DefPluginMenu = { /* d96d46a6-3a38-435c-ad74-a70cfd1719a7 */                                                  
    0xd96d46a6,                                                                                               
    0x3a38,                                                                                                   
    0x435c,                                                                                                   
    {0xad, 0x74, 0xa7, 0x0c, 0xfd, 0x17, 0x19, 0xa7}                                                          
};

GUID guid_DefPluginDiskMenu = { /* 925bb55c-edf6-47f8-aed9-1aa66fbf0d69 */
    0x925bb55c,
    0xedf6,
    0x47f8,
    {0xae, 0xd9, 0x1a, 0xa6, 0x6f, 0xbf, 0x0d, 0x69}
};

GUID guid_DefPluginConfigMenu = { /* 7c0d344e-8030-49f0-81df-f776ef95dde0 */                                                  
    0x7c0d344e,                                                                                               
    0x8030,                                                                                                   
    0x49f0,                                                                                                   
    {0x81, 0xdf, 0xf7, 0x76, 0xef, 0x95, 0xdd, 0xe0}                                                          
};

GUID guid_DefDialogs = { /* c0f2566f-358e-4c81-9947-55cf8bebe103 */
    0xc0f2566f,
    0x358e,
    0x4c81,
    {0x99, 0x47, 0x55, 0xcf, 0x8b, 0xeb, 0xe1, 0x03}
};


#define InvalidOp()

HMODULE ghInstance = NULL;

//#if defined(__GNUC__)
//extern "C"{
//	BOOL   WINAPI DllMain( HANDLE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved );
//	int    WINAPI AnalyseW(const AnalyseInfo *Info);
//	void   WINAPI ClosePanelW(HANDLE hPanel);
//	int    WINAPI CompareW(const CompareInfo *Info);
//	int    WINAPI ConfigureW(const GUID* Guid);
//	int    WINAPI DeleteFilesW(const DeleteFilesInfo *Info);
//	void   WINAPI ExitFARW(void);
//	void   WINAPI FreeFindDataW(const FreeFindDataInfo *Info);
//	void   WINAPI FreeVirtualFindDataW(const FreeFindDataInfo *Info);
//	int    WINAPI GetFilesW(GetFilesInfo *Info);
//	int    WINAPI GetFindDataW(GetFindDataInfo *Info);
//	void   WINAPI GetGlobalInfoW(GlobalInfo *Info);
//	void   WINAPI GetOpenPanelInfoW(OpenPanelInfo *Info);
//	void   WINAPI GetPluginInfoW(PluginInfo *Info);
//	int    WINAPI GetVirtualFindDataW(GetVirtualFindDataInfo *Info);
//	int    WINAPI MakeDirectoryW(MakeDirectoryInfo *Info);
//	HANDLE WINAPI OpenW(const OpenInfo *Info);
//	int    WINAPI ProcessDialogEventW(int Event,void *Param);
//	int    WINAPI ProcessEditorEventW(int Event,void *Param);
//	int    WINAPI ProcessEditorInputW(const INPUT_RECORD *Rec);
//	int    WINAPI ProcessEventW(HANDLE hPanel,int Event,void *Param);
//	int    WINAPI ProcessHostFileW(const ProcessHostFileInfo *Info);
//	int    WINAPI ProcessKeyW(HANDLE hPanel,const INPUT_RECORD *Rec);
//	int    WINAPI ProcessSynchroEventW(int Event,void *Param);
//	int    WINAPI ProcessViewerEventW(int Event,void *Param);
//	int    WINAPI PutFilesW(const PutFilesInfo *Info);
//	int    WINAPI SetDirectoryW(const SetDirectoryInfo *Info);
//	int    WINAPI SetFindListW(const SetFindListInfo *Info);
//	void   WINAPI SetStartupInfoW(const PluginStartupInfo *Info);
//};
//#endif

//void f1()
//{
//	gi1 = 0;
//}

BOOL lbPsi2 = FALSE;
PluginStartupInfo psi3;
FarStandardFunctions FSF3;
Far2::PluginStartupInfo psi2;
Far2::FarStandardFunctions FSF2;
struct WrapPluginInfo;
WrapPluginInfo* wpi = NULL;

int OpMode_3_2(OPERATION_MODES OpMode3)
{
	int OpMode2 = 0;
	if (OpMode3 & OPM_SILENT)
		OpMode2 |= Far2::OPM_SILENT;
	if (OpMode3 & OPM_FIND)
		OpMode2 |= Far2::OPM_FIND;
	if (OpMode3 & OPM_VIEW)
		OpMode2 |= Far2::OPM_VIEW;
	if (OpMode3 & OPM_EDIT)
		OpMode2 |= Far2::OPM_EDIT;
	if (OpMode3 & OPM_TOPLEVEL)
		OpMode2 |= Far2::OPM_TOPLEVEL;
	if (OpMode3 & OPM_DESCR)
		OpMode2 |= Far2::OPM_DESCR;
	if (OpMode3 & OPM_QUICKVIEW)
		OpMode2 |= Far2::OPM_QUICKVIEW;
	//if (OpMode3 & OPM_PGDN) -- в Far2 отсутствует
	//	OpMode2 |= Far2::OPM_PGDN;
	return OpMode2;
}
//OPERATION_MODES OpMode_2_3(int OpMode2)
//{
//}

int OpenFrom_3_2(OPENFROM OpenFrom3)
{
	int OpenFrom2 = 0;
	
	switch ((OpenFrom3 & OPEN_FROM_MASK))
	{
		case OPEN_LEFTDISKMENU:
			OpenFrom2 |= Far2::OPEN_DISKMENU; break;
		case OPEN_PLUGINSMENU:
			OpenFrom2 |= Far2::OPEN_PLUGINSMENU; break;
		case OPEN_FINDLIST:
			OpenFrom2 |= Far2::OPEN_FINDLIST; break;
		case OPEN_SHORTCUT:
			OpenFrom2 |= Far2::OPEN_SHORTCUT; break;
		case OPEN_COMMANDLINE:
			OpenFrom2 |= Far2::OPEN_COMMANDLINE; break;
		case OPEN_EDITOR:
			OpenFrom2 |= Far2::OPEN_EDITOR; break;
		case OPEN_VIEWER:
			OpenFrom2 |= Far2::OPEN_VIEWER; break;
		case OPEN_FILEPANEL:
			OpenFrom2 |= Far2::OPEN_FILEPANEL; break;
		case OPEN_DIALOG:
			OpenFrom2 |= Far2::OPEN_DIALOG; break;
		case OPEN_ANALYSE:
			OpenFrom2 |= Far2::OPEN_ANALYSE; break;
		//case OPEN_RIGHTDISKMENU: -- в Far2 отсутсвует
	}
	
	if (OpenFrom3 & OPEN_FROMMACRO_MASK)
		OpenFrom2 |= Far2::OPEN_FROMMACRO;
	
	return OpenFrom2;
}

OPENPANELINFO_FLAGS OpenPanelInfoFlags_2_3(DWORD Flags2)
{
	OPENPANELINFO_FLAGS Flags3 = OPIF_NONE;
	
	if (!(Flags2 & Far2::OPIF_USEFILTER)) Flags3 |= OPIF_DISABLEFILTER;
	if (!(Flags2 & Far2::OPIF_USESORTGROUPS)) Flags3 |= OPIF_DISABLESORTGROUPS;
	if (!(Flags2 & Far2::OPIF_USEHIGHLIGHTING)) Flags3 |= OPIF_DISABLEHIGHLIGHTING;
	if ((Flags2 & Far2::OPIF_ADDDOTS)) Flags3 |= OPIF_ADDDOTS;
	if ((Flags2 & Far2::OPIF_RAWSELECTION)) Flags3 |= OPIF_RAWSELECTION;
	if ((Flags2 & Far2::OPIF_REALNAMES)) Flags3 |= OPIF_REALNAMES;
	if ((Flags2 & Far2::OPIF_SHOWNAMESONLY)) Flags3 |= OPIF_SHOWNAMESONLY;
	if ((Flags2 & Far2::OPIF_SHOWRIGHTALIGNNAMES)) Flags3 |= OPIF_SHOWRIGHTALIGNNAMES;
	if ((Flags2 & Far2::OPIF_SHOWPRESERVECASE)) Flags3 |= OPIF_SHOWPRESERVECASE;
	if ((Flags2 & Far2::OPIF_COMPAREFATTIME)) Flags3 |= OPIF_COMPAREFATTIME;
	if ((Flags2 & Far2::OPIF_EXTERNALGET)) Flags3 |= OPIF_EXTERNALGET;
	if ((Flags2 & Far2::OPIF_EXTERNALPUT)) Flags3 |= OPIF_EXTERNALPUT;
	if ((Flags2 & Far2::OPIF_EXTERNALDELETE)) Flags3 |= OPIF_EXTERNALDELETE;
	if ((Flags2 & Far2::OPIF_EXTERNALMKDIR)) Flags3 |= OPIF_EXTERNALMKDIR;
	if ((Flags2 & Far2::OPIF_USEATTRHIGHLIGHTING)) Flags3 |= OPIF_USEATTRHIGHLIGHTING;
	
	return Flags3;
}

Far2::OPENPLUGININFO_SORTMODES SortMode_3_2(OPENPANELINFO_SORTMODES Mode3)
{
	Far2::OPENPLUGININFO_SORTMODES Mode2 = Far2::SM_DEFAULT;
	switch (Mode3)
	{
		case SM_UNSORTED: Mode2 = Far2::SM_UNSORTED; break;
		case SM_NAME: Mode2 = Far2::SM_NAME; break;
		case SM_EXT: Mode2 = Far2::SM_EXT; break;
		case SM_MTIME: Mode2 = Far2::SM_MTIME; break;
		case SM_CTIME: Mode2 = Far2::SM_CTIME; break;
		case SM_ATIME: Mode2 = Far2::SM_ATIME; break;
		case SM_SIZE: Mode2 = Far2::SM_SIZE; break;
		case SM_DESCR: Mode2 = Far2::SM_DESCR; break;
		case SM_OWNER: Mode2 = Far2::SM_OWNER; break;
		case SM_COMPRESSEDSIZE: Mode2 = Far2::SM_COMPRESSEDSIZE; break;
		case SM_NUMLINKS: Mode2 = Far2::SM_NUMLINKS; break;
		case SM_NUMSTREAMS: Mode2 = Far2::SM_NUMSTREAMS; break;
		case SM_STREAMSSIZE: Mode2 = Far2::SM_STREAMSSIZE; break;
		case SM_FULLNAME: Mode2 = Far2::SM_FULLNAME; break;
	}
	return Mode2;
}

OPENPANELINFO_SORTMODES SortMode_2_3(/*Far2::OPENPLUGININFO_SORTMODES*/int Mode2)
{
	OPENPANELINFO_SORTMODES Mode3 = SM_DEFAULT;
	switch (Mode2)
	{
		case Far2::SM_UNSORTED: Mode3 = SM_UNSORTED; break;
		case Far2::SM_NAME: Mode3 = SM_NAME; break;
		case Far2::SM_EXT: Mode3 = SM_EXT; break;
		case Far2::SM_MTIME: Mode3 = SM_MTIME; break;
		case Far2::SM_CTIME: Mode3 = SM_CTIME; break;
		case Far2::SM_ATIME: Mode3 = SM_ATIME; break;
		case Far2::SM_SIZE: Mode3 = SM_SIZE; break;
		case Far2::SM_DESCR: Mode3 = SM_DESCR; break;
		case Far2::SM_OWNER: Mode3 = SM_OWNER; break;
		case Far2::SM_COMPRESSEDSIZE: Mode3 = SM_COMPRESSEDSIZE; break;
		case Far2::SM_NUMLINKS: Mode3 = SM_NUMLINKS; break;
		case Far2::SM_NUMSTREAMS: Mode3 = SM_NUMSTREAMS; break;
		case Far2::SM_STREAMSSIZE: Mode3 = SM_STREAMSSIZE; break;
		case Far2::SM_FULLNAME: Mode3 = SM_FULLNAME; break;
	}
	return Mode3;
}

PLUGINPANELITEMFLAGS PluginPanelItemFlags_2_3(DWORD Flags2)
{
	PLUGINPANELITEMFLAGS Flags3 = PPIF_NONE;
	if (Flags2 & Far2::PPIF_PROCESSDESCR)
		Flags3 |= PPIF_PROCESSDESCR;
	if (Flags2 & Far2::PPIF_USERDATA)
		Flags3 |= PPIF_USERDATA;
	if (Flags2 & Far2::PPIF_SELECTED)
		Flags3 |= PPIF_SELECTED;
	return Flags3;
}

DWORD PluginPanelItemFlags_3_2(PLUGINPANELITEMFLAGS Flags3)
{
	DWORD Flags2 = 0;
	if (Flags3 & PPIF_PROCESSDESCR)
		Flags2 |= Far2::PPIF_PROCESSDESCR;
	if (Flags3 & PPIF_USERDATA)
		Flags2 |= Far2::PPIF_USERDATA;
	if (Flags3 & PPIF_SELECTED)
		Flags2 |= Far2::PPIF_SELECTED;
	return Flags2;
}

void PluginPanelItem_2_3(const Far2::PluginPanelItem* p2, PluginPanelItem* p3)
{
	p3->FileAttributes = p2->FindData.dwFileAttributes;
	p3->CreationTime = p2->FindData.ftCreationTime;
	p3->LastAccessTime = p2->FindData.ftLastAccessTime;
	p3->LastWriteTime = p2->FindData.ftLastWriteTime;
	p3->ChangeTime = p2->FindData.ftLastWriteTime;
	p3->FileSize = p2->FindData.nFileSize;
	p3->PackSize = p2->FindData.nPackSize;
	p3->FileName = p2->FindData.lpwszFileName;
	p3->AlternateFileName = p2->FindData.lpwszAlternateFileName;
	p3->Flags = PluginPanelItemFlags_2_3(p2->Flags);
	p3->NumberOfLinks = p2->NumberOfLinks;
	p3->Description = p2->Description;
	p3->Owner = p2->Owner;
	p3->CustomColumnData = p2->CustomColumnData;
	p3->CustomColumnNumber = p2->CustomColumnNumber;
	p3->UserData = p2->UserData;
	p3->CRC32 = p2->CRC32;
}

PluginPanelItem* PluginPanelItems_2_3(const Far2::PluginPanelItem* pItems, int ItemsNumber)
{
	PluginPanelItem* p3 = NULL;
	if (pItems && ItemsNumber > 0)
	{
		p3 = (PluginPanelItem*)calloc(ItemsNumber, sizeof(*p3));
		if (p3)
		{
			for (int i = 0; i < ItemsNumber; i++)
				PluginPanelItem_2_3(pItems+i, p3+i);
		}
	}
	return p3;
}

void PluginPanelItem_3_2(const PluginPanelItem* p3, Far2::PluginPanelItem* p2)
{
	p2->FindData.dwFileAttributes = p3->FileAttributes;
	p2->FindData.ftCreationTime = p3->CreationTime;
	p2->FindData.ftLastAccessTime = p3->LastAccessTime;
	p2->FindData.ftLastWriteTime = p3->LastWriteTime;
	//p2->FindData.ftLastWriteTime = p3->ChangeTime;
	p2->FindData.nFileSize = p3->FileSize;
	p2->FindData.nPackSize = p3->PackSize;
	p2->FindData.lpwszFileName = p3->FileName;
	p2->FindData.lpwszAlternateFileName = p3->AlternateFileName;
	p2->Flags = PluginPanelItemFlags_3_2(p3->Flags);
	p2->NumberOfLinks = p3->NumberOfLinks;
	p2->Description = p3->Description;
	p2->Owner = p3->Owner;
	p2->CustomColumnData = p3->CustomColumnData;
	p2->CustomColumnNumber = p3->CustomColumnNumber;
	p2->UserData = p3->UserData;
	p2->CRC32 = p3->CRC32;
}

Far2::PluginPanelItem* PluginPanelItems_3_2(const PluginPanelItem* pItems, int ItemsNumber)
{
	Far2::PluginPanelItem* p2 = NULL;
	if (pItems && ItemsNumber > 0)
	{
		p2 = (Far2::PluginPanelItem*)calloc(ItemsNumber, sizeof(*p2));
		if (p2)
		{
			for (int i = 0; i < ItemsNumber; i++)
			{
				PluginPanelItem_3_2(pItems+i, p2+i);
			}
		}
	}
	return p2;
}

//FarKey FarKey_2_3(int Key2)
//{
//	FarKey Key3 = {0};
//	//TODO:
//	return Key3;
//}

DWORD FarKey_3_2(KEY_EVENT_RECORD Key)
{
	int Key2 = 0;
	//TODO:
	return Key2;
}

FARDIALOGITEMTYPES DialogItemTypes_2_3(int ItemType2)
{
	FARDIALOGITEMTYPES ItemType3 = DI_TEXT;
	switch (ItemType2)
	{
	case Far2::DI_TEXT: ItemType3 = DI_TEXT; break;
	case Far2::DI_VTEXT: ItemType3 = DI_VTEXT; break;
	case Far2::DI_SINGLEBOX: ItemType3 = DI_SINGLEBOX; break;
	case Far2::DI_DOUBLEBOX: ItemType3 = DI_DOUBLEBOX; break;
	case Far2::DI_EDIT: ItemType3 = DI_EDIT; break;
	case Far2::DI_PSWEDIT: ItemType3 = DI_PSWEDIT; break;
	case Far2::DI_FIXEDIT: ItemType3 = DI_FIXEDIT; break;
	case Far2::DI_BUTTON: ItemType3 = DI_BUTTON; break;
	case Far2::DI_CHECKBOX: ItemType3 = DI_CHECKBOX; break;
	case Far2::DI_RADIOBUTTON: ItemType3 = DI_RADIOBUTTON; break;
	case Far2::DI_COMBOBOX: ItemType3 = DI_COMBOBOX; break;
	case Far2::DI_LISTBOX: ItemType3 = DI_LISTBOX; break;
	case Far2::DI_USERCONTROL: ItemType3 = DI_USERCONTROL; break;
	}
	return ItemType3;
}

FarDialogItemFlags FarDialogItemFlags_2_3(DWORD Flags2)
{
	_ASSERTE(Far2::DIF_COLORMASK == DIF_COLORMASK);
	FarDialogItemFlags Flags3 = (Flags2 & Far2::DIF_COLORMASK);

	if (Flags2 & Far2::DIF_SETCOLOR)
		Flags3 |= DIF_SETCOLOR;
	if (Flags2 & Far2::DIF_BOXCOLOR)
		Flags3 |= DIF_BOXCOLOR;
	if (Flags2 & Far2::DIF_GROUP)
		Flags3 |= DIF_GROUP;
	if (Flags2 & Far2::DIF_LEFTTEXT)
		Flags3 |= DIF_LEFTTEXT;
	if (Flags2 & Far2::DIF_MOVESELECT)
		Flags3 |= DIF_MOVESELECT;
	if (Flags2 & Far2::DIF_SHOWAMPERSAND)
		Flags3 |= DIF_SHOWAMPERSAND;
	if (Flags2 & Far2::DIF_CENTERGROUP)
		Flags3 |= DIF_CENTERGROUP;
	if (Flags2 & Far2::DIF_NOBRACKETS/* == DIF_MANUALADDHISTORY*/)
		Flags3 |= DIF_NOBRACKETS/* == DIF_MANUALADDHISTORY*/;
	if (Flags2 & Far2::DIF_SEPARATOR)
		Flags3 |= DIF_SEPARATOR;
	if (Flags2 & Far2::DIF_SEPARATOR2/* == DIF_EDITOR == DIF_LISTNOAMPERSAND*/)
		Flags3 |= DIF_SEPARATOR2/* == DIF_EDITOR == DIF_LISTNOAMPERSAND*/;
	if (Flags2 & Far2::DIF_LISTNOBOX/* == DIF_HISTORY == DIF_BTNNOCLOSE == DIF_CENTERTEXT*/)
		Flags3 |= DIF_LISTNOBOX/* == DIF_HISTORY == DIF_BTNNOCLOSE == DIF_CENTERTEXT*/;
	if (Flags2 & Far2::DIF_SETSHIELD/* == DIF_EDITEXPAND*/)
		Flags3 |= DIF_SETSHIELD/* == DIF_EDITEXPAND*/;
	if (Flags2 & Far2::DIF_DROPDOWNLIST)
		Flags3 |= DIF_DROPDOWNLIST;
	if (Flags2 & Far2::DIF_USELASTHISTORY)
		Flags3 |= DIF_USELASTHISTORY;
	if (Flags2 & Far2::DIF_MASKEDIT)
		Flags3 |= DIF_MASKEDIT;
	if (Flags2 & Far2::DIF_SELECTONENTRY/* == DIF_3STATE*/)
		Flags3 |= DIF_SELECTONENTRY/* == DIF_3STATE*/;
	if (Flags2 & Far2::DIF_SELECTONENTRY/* == DIF_3STATE*/)
		Flags3 |= DIF_SELECTONENTRY/* == DIF_3STATE*/;
	if (Flags2 & Far2::DIF_NOAUTOCOMPLETE/* == DIF_LISTAUTOHIGHLIGHT*/)
		Flags3 |= DIF_NOAUTOCOMPLETE/* == DIF_LISTAUTOHIGHLIGHT*/;
	if (Flags2 & Far2::DIF_LISTNOCLOSE)
		Flags3 |= DIF_LISTNOCLOSE;
	if (Flags2 & Far2::DIF_HIDDEN)
		Flags3 |= DIF_HIDDEN;
	if (Flags2 & Far2::DIF_READONLY)
		Flags3 |= DIF_READONLY;
	if (Flags2 & Far2::DIF_NOFOCUS)
		Flags3 |= DIF_NOFOCUS;
	if (Flags2 & Far2::DIF_DISABLE)
		Flags3 |= DIF_DISABLE;
	if (Flags2 & Far2::DIF_DISABLE)
		Flags3 |= DIF_DISABLE;

	return Flags3;
}

int FarMessage_2_3(int Msg2, LONG_PTR& Param)
{
	int Msg3 = 0;
	if (Msg2 >= Far2::DM_USER)
		Msg3 = Msg2;
	else switch (Msg2)
	{
		case Far2::DM_FIRST: Msg3 = DM_FIRST; break;
		case Far2::DM_CLOSE: Msg3 = DM_CLOSE; break;
		case Far2::DM_ENABLE: Msg3 = DM_ENABLE; break;
		case Far2::DM_ENABLEREDRAW: Msg3 = DM_ENABLEREDRAW; break;
		case Far2::DM_GETDLGDATA: Msg3 = DM_GETDLGDATA; break;
		case Far2::DM_GETDLGITEM: Msg3 = DM_GETDLGITEM; break;
		case Far2::DM_GETDLGRECT: Msg3 = DM_GETDLGRECT; break;
		case Far2::DM_GETTEXT: Msg3 = DM_GETTEXT; break;
		case Far2::DM_GETTEXTLENGTH: Msg3 = DM_GETTEXTLENGTH; break;
		case Far2::DM_KEY:
			// Аргументы?
			_ASSERTE(Msg2 != Far2::DM_KEY);
			Msg3 = DM_KEY; break;
		case Far2::DM_MOVEDIALOG: Msg3 = DM_MOVEDIALOG; break;
		case Far2::DM_SETDLGDATA: Msg3 = DM_SETDLGDATA; break;
		case Far2::DM_SETDLGITEM: Msg3 = DM_SETDLGITEM; break;
		case Far2::DM_SETFOCUS: Msg3 = DM_SETFOCUS; break;
		case Far2::DM_REDRAW: Msg3 = DM_REDRAW; break;
		//DM_SETREDRAW=DM_REDRAW,
		case Far2::DM_SETTEXT: Msg3 = DM_SETTEXT; break;
		case Far2::DM_SETMAXTEXTLENGTH: Msg3 = DM_SETMAXTEXTLENGTH; break;
		//DM_SETTEXTLENGTH=DM_SETMAXTEXTLENGTH,
		case Far2::DM_SHOWDIALOG: Msg3 = DM_SHOWDIALOG; break;
		case Far2::DM_GETFOCUS: Msg3 = DM_GETFOCUS; break;
		case Far2::DM_GETCURSORPOS: Msg3 = DM_GETCURSORPOS; break;
		case Far2::DM_SETCURSORPOS: Msg3 = DM_SETCURSORPOS; break;
		case Far2::DM_GETTEXTPTR: Msg3 = DM_GETTEXTPTR; break;
		case Far2::DM_SETTEXTPTR: Msg3 = DM_SETTEXTPTR; break;
		case Far2::DM_SHOWITEM: Msg3 = DM_SHOWITEM; break;
		case Far2::DM_ADDHISTORY: Msg3 = DM_ADDHISTORY; break;
		case Far2::DM_GETCHECK: Msg3 = DM_GETCHECK; break;
		case Far2::DM_SETCHECK: Msg3 = DM_SETCHECK; break;
		case Far2::DM_SET3STATE: Msg3 = DM_SET3STATE; break;
		case Far2::DM_LISTSORT: Msg3 = DM_LISTSORT; break;
		case Far2::DM_LISTGETITEM: Msg3 = DM_LISTGETITEM; break;
		case Far2::DM_LISTGETCURPOS: Msg3 = DM_LISTGETCURPOS; break;
		case Far2::DM_LISTSETCURPOS: Msg3 = DM_LISTSETCURPOS; break;
		case Far2::DM_LISTDELETE: Msg3 = DM_LISTDELETE; break;
		case Far2::DM_LISTADD: Msg3 = DM_LISTADD; break;
		case Far2::DM_LISTADDSTR: Msg3 = DM_LISTADDSTR; break;
		case Far2::DM_LISTUPDATE: Msg3 = DM_LISTUPDATE; break;
		case Far2::DM_LISTINSERT: Msg3 = DM_LISTINSERT; break;
		case Far2::DM_LISTFINDSTRING: Msg3 = DM_LISTFINDSTRING; break;
		case Far2::DM_LISTINFO: Msg3 = DM_LISTINFO; break;
		case Far2::DM_LISTGETDATA: Msg3 = DM_LISTGETDATA; break;
		case Far2::DM_LISTSETDATA: Msg3 = DM_LISTSETDATA; break;
		case Far2::DM_LISTSETTITLES: Msg3 = DM_LISTSETTITLES; break;
		case Far2::DM_LISTGETTITLES: Msg3 = DM_LISTGETTITLES; break;
		case Far2::DM_RESIZEDIALOG: Msg3 = DM_RESIZEDIALOG; break;
		case Far2::DM_SETITEMPOSITION: Msg3 = DM_SETITEMPOSITION; break;
		case Far2::DM_GETDROPDOWNOPENED: Msg3 = DM_GETDROPDOWNOPENED; break;
		case Far2::DM_SETDROPDOWNOPENED: Msg3 = DM_SETDROPDOWNOPENED; break;
		case Far2::DM_SETHISTORY: Msg3 = DM_SETHISTORY; break;
		case Far2::DM_GETITEMPOSITION: Msg3 = DM_GETITEMPOSITION; break;
		case Far2::DM_SETMOUSEEVENTNOTIFY: Msg3 = DM_SETMOUSEEVENTNOTIFY; break;
		case Far2::DM_EDITUNCHANGEDFLAG: Msg3 = DM_EDITUNCHANGEDFLAG; break;
		case Far2::DM_GETITEMDATA: Msg3 = DM_GETITEMDATA; break;
		case Far2::DM_SETITEMDATA: Msg3 = DM_SETITEMDATA; break;
		case Far2::DM_LISTSET: Msg3 = DM_LISTSET; break;
		case Far2::DM_LISTSETMOUSEREACTION: Msg3 = DM_LISTSETMOUSEREACTION; break;
		case Far2::DM_GETCURSORSIZE: Msg3 = DM_GETCURSORSIZE; break;
		case Far2::DM_SETCURSORSIZE: Msg3 = DM_SETCURSORSIZE; break;
		case Far2::DM_LISTGETDATASIZE: Msg3 = DM_LISTGETDATASIZE; break;
		case Far2::DM_GETSELECTION: Msg3 = DM_GETSELECTION; break;
		case Far2::DM_SETSELECTION: Msg3 = DM_SETSELECTION; break;
		case Far2::DM_GETEDITPOSITION: Msg3 = DM_GETEDITPOSITION; break;
		case Far2::DM_SETEDITPOSITION: Msg3 = DM_SETEDITPOSITION; break;
		case Far2::DM_SETCOMBOBOXEVENT: Msg3 = DM_SETCOMBOBOXEVENT; break;
		case Far2::DM_GETCOMBOBOXEVENT: Msg3 = DM_GETCOMBOBOXEVENT; break;
		case Far2::DM_GETCONSTTEXTPTR: Msg3 = DM_GETCONSTTEXTPTR; break;
		case Far2::DM_GETDLGITEMSHORT: Msg3 = DM_GETDLGITEMSHORT; break;
		case Far2::DM_SETDLGITEMSHORT: Msg3 = DM_SETDLGITEMSHORT; break;
		case Far2::DM_GETDIALOGINFO: Msg3 = DM_GETDIALOGINFO; break;
		case Far2::DN_FIRST: Msg3 = DN_FIRST; break;
		case Far2::DN_BTNCLICK: Msg3 = DN_BTNCLICK; break;
		case Far2::DN_CTLCOLORDIALOG: Msg3 = DN_CTLCOLORDIALOG; break;
		case Far2::DN_CTLCOLORDLGITEM: Msg3 = DN_CTLCOLORDLGITEM; break;
		case Far2::DN_CTLCOLORDLGLIST: Msg3 = DN_CTLCOLORDLGLIST; break;
		case Far2::DN_DRAWDIALOG: Msg3 = DN_DRAWDIALOG; break;
		case Far2::DN_DRAWDLGITEM: Msg3 = DN_DRAWDLGITEM; break;
		case Far2::DN_EDITCHANGE: Msg3 = DN_EDITCHANGE; break;
		case Far2::DN_ENTERIDLE: Msg3 = DN_ENTERIDLE; break;
		case Far2::DN_GOTFOCUS: Msg3 = DN_GOTFOCUS; break;
		case Far2::DN_HELP: Msg3 = DN_HELP; break;
		case Far2::DN_HOTKEY: Msg3 = DN_HOTKEY; break;
		case Far2::DN_INITDIALOG: Msg3 = DN_INITDIALOG; break;
		case Far2::DN_KILLFOCUS: Msg3 = DN_KILLFOCUS; break;
		case Far2::DN_LISTCHANGE: Msg3 = DN_LISTCHANGE; break;
		case Far2::DN_MOUSECLICK:
			// DN_KEY и DN_MOUSECLICK объеденены в DN_CONTROLINPUT. 
			// Param2 указывает на INPUT_RECORD. в будущем планируется приход и других событий.
			_ASSERTE(Msg2 != Far2::DN_MOUSECLICK);
			Msg3 = DN_CONTROLINPUT;
			Msg3 = 0;
			break;
		case Far2::DN_DRAGGED: Msg3 = DN_DRAGGED; break;
		case Far2::DN_RESIZECONSOLE: Msg3 = DN_RESIZECONSOLE; break;
		case Far2::DN_MOUSEEVENT:
			// DN_MOUSEEVENT переименовано в DN_INPUT. Param2 указывает на INPUT_RECORD. 
			// в будущем планируется приход не только мышиных событий, поэтому настоятельно 
			// рекомендуется проверять EventType.
			_ASSERTE(Msg2 != Far2::DN_MOUSEEVENT);
			Msg3 = DN_INPUT;
			Msg3 = 0;
			break;
		case Far2::DN_DRAWDIALOGDONE: Msg3 = DN_DRAWDIALOGDONE; break;
		case Far2::DN_LISTHOTKEY: Msg3 = DN_LISTHOTKEY; break;
		//DN_GETDIALOGINFO=DM_GETDIALOGINFO,
		//DN_CLOSE=DM_CLOSE,
		//DN_KEY=DM_KEY, // 3. DM_KEY больше не равна DN_KEY.
	}
	return Msg3;
}

int FarMessage_3_2(int Msg3, INT_PTR& Param)
{
	int Msg2 = 0;
	if (Msg3 >= DM_USER)
		Msg2 = Msg3;
	else switch (Msg3)
	{
		case DN_INPUT:
		case DN_CONTROLINPUT:
			if (!Param)
			{
				_ASSERTE(Param!=0);
				Msg2 = 0;
			}
			else
			{
				const INPUT_RECORD* p = (const INPUT_RECORD*)Param;
				if (p->EventType == MOUSE_EVENT)
				{
					static MOUSE_EVENT_RECORD mer;
					mer = p->Event.MouseEvent;
					Param = (INT_PTR)&mer;
					if (!mer.dwEventFlags)
						Msg2 = Far2::DN_MOUSECLICK;
					else
						Msg2 = Far2::DN_MOUSEEVENT;
				}
				else if (p->EventType == KEY_EVENT)
				{
					Param = FarKey_3_2(p->Event.KeyEvent);
					Msg2 = Far2::DN_KEY;
				}
				else
					Msg2 = 0;
			}
			break;

		case DM_FIRST: Msg2 = Far2::DM_FIRST; break;
		case DM_CLOSE: Msg2 = Far2::DM_CLOSE; break;
		case DM_ENABLE: Msg2 = Far2::DM_ENABLE; break;
		case DM_ENABLEREDRAW: Msg2 = Far2::DM_ENABLEREDRAW; break;
		case DM_GETDLGDATA: Msg2 = Far2::DM_GETDLGDATA; break;
		case DM_GETDLGITEM: Msg2 = Far2::DM_GETDLGITEM; break;
		case DM_GETDLGRECT: Msg2 = Far2::DM_GETDLGRECT; break;
		case DM_GETTEXT: Msg2 = Far2::DM_GETTEXT; break;
		case DM_GETTEXTLENGTH: Msg2 = Far2::DM_GETTEXTLENGTH; break;
		case DM_KEY: Msg2 = Far2::DM_KEY; break;
		case DM_MOVEDIALOG: Msg2 = Far2::DM_MOVEDIALOG; break;
		case DM_SETDLGDATA: Msg2 = Far2::DM_SETDLGDATA; break;
		case DM_SETDLGITEM: Msg2 = Far2::DM_SETDLGITEM; break;
		case DM_SETFOCUS: Msg2 = Far2::DM_SETFOCUS; break;
		case DM_REDRAW: Msg2 = Far2::DM_REDRAW; break;
		case DM_SETTEXT: Msg2 = Far2::DM_SETTEXT; break;
		case DM_SETMAXTEXTLENGTH: Msg2 = Far2::DM_SETMAXTEXTLENGTH; break;
		case DM_SHOWDIALOG: Msg2 = Far2::DM_SHOWDIALOG; break;
		case DM_GETFOCUS: Msg2 = Far2::DM_GETFOCUS; break;
		case DM_GETCURSORPOS: Msg2 = Far2::DM_GETCURSORPOS; break;
		case DM_SETCURSORPOS: Msg2 = Far2::DM_SETCURSORPOS; break;
		case DM_GETTEXTPTR: Msg2 = Far2::DM_GETTEXTPTR; break;
		case DM_SETTEXTPTR: Msg2 = Far2::DM_SETTEXTPTR; break;
		case DM_SHOWITEM: Msg2 = Far2::DM_SHOWITEM; break;
		case DM_ADDHISTORY: Msg2 = Far2::DM_ADDHISTORY; break;
		case DM_GETCHECK: Msg2 = Far2::DM_GETCHECK; break;
		case DM_SETCHECK: Msg2 = Far2::DM_SETCHECK; break;
		case DM_SET3STATE: Msg2 = Far2::DM_SET3STATE; break;
		case DM_LISTSORT: Msg2 = Far2::DM_LISTSORT; break;
		case DM_LISTGETITEM: Msg2 = Far2::DM_LISTGETITEM; break;
		case DM_LISTGETCURPOS: Msg2 = Far2::DM_LISTGETCURPOS; break;
		case DM_LISTSETCURPOS: Msg2 = Far2::DM_LISTSETCURPOS; break;
		case DM_LISTDELETE: Msg2 = Far2::DM_LISTDELETE; break;
		case DM_LISTADD: Msg2 = Far2::DM_LISTADD; break;
		case DM_LISTADDSTR: Msg2 = Far2::DM_LISTADDSTR; break;
		case DM_LISTUPDATE: Msg2 = Far2::DM_LISTUPDATE; break;
		case DM_LISTINSERT: Msg2 = Far2::DM_LISTINSERT; break;
		case DM_LISTFINDSTRING: Msg2 = Far2::DM_LISTFINDSTRING; break;
		case DM_LISTINFO: Msg2 = Far2::DM_LISTINFO; break;
		case DM_LISTGETDATA: Msg2 = Far2::DM_LISTGETDATA; break;
		case DM_LISTSETDATA: Msg2 = Far2::DM_LISTSETDATA; break;
		case DM_LISTSETTITLES: Msg2 = Far2::DM_LISTSETTITLES; break;
		case DM_LISTGETTITLES: Msg2 = Far2::DM_LISTGETTITLES; break;
		case DM_RESIZEDIALOG: Msg2 = Far2::DM_RESIZEDIALOG; break;
		case DM_SETITEMPOSITION: Msg2 = Far2::DM_SETITEMPOSITION; break;
		case DM_GETDROPDOWNOPENED: Msg2 = Far2::DM_GETDROPDOWNOPENED; break;
		case DM_SETDROPDOWNOPENED: Msg2 = Far2::DM_SETDROPDOWNOPENED; break;
		case DM_SETHISTORY: Msg2 = Far2::DM_SETHISTORY; break;
		case DM_GETITEMPOSITION: Msg2 = Far2::DM_GETITEMPOSITION; break;
		case DM_SETMOUSEEVENTNOTIFY: Msg2 = Far2::DM_SETMOUSEEVENTNOTIFY; break;
		case DM_EDITUNCHANGEDFLAG: Msg2 = Far2::DM_EDITUNCHANGEDFLAG; break;
		case DM_GETITEMDATA: Msg2 = Far2::DM_GETITEMDATA; break;
		case DM_SETITEMDATA: Msg2 = Far2::DM_SETITEMDATA; break;
		case DM_LISTSET: Msg2 = Far2::DM_LISTSET; break;
		case DM_LISTSETMOUSEREACTION: Msg2 = Far2::DM_LISTSETMOUSEREACTION; break;
		case DM_GETCURSORSIZE: Msg2 = Far2::DM_GETCURSORSIZE; break;
		case DM_SETCURSORSIZE: Msg2 = Far2::DM_SETCURSORSIZE; break;
		case DM_LISTGETDATASIZE: Msg2 = Far2::DM_LISTGETDATASIZE; break;
		case DM_GETSELECTION: Msg2 = Far2::DM_GETSELECTION; break;
		case DM_SETSELECTION: Msg2 = Far2::DM_SETSELECTION; break;
		case DM_GETEDITPOSITION: Msg2 = Far2::DM_GETEDITPOSITION; break;
		case DM_SETEDITPOSITION: Msg2 = Far2::DM_SETEDITPOSITION; break;
		case DM_SETCOMBOBOXEVENT: Msg2 = Far2::DM_SETCOMBOBOXEVENT; break;
		case DM_GETCOMBOBOXEVENT: Msg2 = Far2::DM_GETCOMBOBOXEVENT; break;
		case DM_GETCONSTTEXTPTR: Msg2 = Far2::DM_GETCONSTTEXTPTR; break;
		case DM_GETDLGITEMSHORT: Msg2 = Far2::DM_GETDLGITEMSHORT; break;
		case DM_SETDLGITEMSHORT: Msg2 = Far2::DM_SETDLGITEMSHORT; break;
		case DM_GETDIALOGINFO: Msg2 = Far2::DM_GETDIALOGINFO; break;
		case DN_FIRST: Msg2 = Far2::DN_FIRST; break;
		case DN_BTNCLICK: Msg2 = Far2::DN_BTNCLICK; break;
		case DN_CTLCOLORDIALOG: Msg2 = Far2::DN_CTLCOLORDIALOG; break;
		case DN_CTLCOLORDLGITEM: Msg2 = Far2::DN_CTLCOLORDLGITEM; break;
		case DN_CTLCOLORDLGLIST: Msg2 = Far2::DN_CTLCOLORDLGLIST; break;
		case DN_DRAWDIALOG: Msg2 = Far2::DN_DRAWDIALOG; break;
		case DN_DRAWDLGITEM: Msg2 = Far2::DN_DRAWDLGITEM; break;
		case DN_EDITCHANGE: Msg2 = Far2::DN_EDITCHANGE; break;
		case DN_ENTERIDLE: Msg2 = Far2::DN_ENTERIDLE; break;
		case DN_GOTFOCUS: Msg2 = Far2::DN_GOTFOCUS; break;
		case DN_HELP: Msg2 = Far2::DN_HELP; break;
		case DN_HOTKEY: Msg2 = Far2::DN_HOTKEY; break;
		case DN_INITDIALOG: Msg2 = Far2::DN_INITDIALOG; break;
		case DN_KILLFOCUS: Msg2 = Far2::DN_KILLFOCUS; break;
		case DN_LISTCHANGE: Msg2 = Far2::DN_LISTCHANGE; break;
		case DN_DRAGGED: Msg2 = Far2::DN_DRAGGED; break;
		case DN_RESIZECONSOLE: Msg2 = Far2::DN_RESIZECONSOLE; break;
		case DN_DRAWDIALOGDONE: Msg2 = Far2::DN_DRAWDIALOGDONE; break;
		case DN_LISTHOTKEY: Msg2 = Far2::DN_LISTHOTKEY; break;
		case DN_CLOSE: Msg2 = Far2::DN_CLOSE; break;
	}
	return Msg2;
}

struct Far2Dialog
{
	// Far3
	HANDLE hDlg3;
	
	// Far2
    int m_X1, m_Y1, m_X2, m_Y2;
    wchar_t *m_HelpTopic;
    Far2::FarDialogItem *m_Items2;
    FarDialogItem *m_Items3;
    UINT m_ItemsNumber;
    DWORD m_Flags;
    Far2::FARWINDOWPROC m_DlgProc;
    LONG_PTR m_Param;
    BOOL m_GuidChecked;
    GUID m_PluginGuid, m_Guid, m_DefGuid;
    
    void FreeDlg();
	static INT_PTR WINAPI Far3DlgProc(HANDLE hDlg, int Msg, int Param1, INT_PTR Param2);
    int RunDlg();
    
	Far2Dialog(int X1, int Y1, int X2, int Y2,
	    const wchar_t *HelpTopic, Far2::FarDialogItem *Items, UINT ItemsNumber,
	    DWORD Flags, Far2::FARWINDOWPROC DlgProc, LONG_PTR Param,
	    GUID PluginGuid, GUID DefGuid);
	~Far2Dialog();
};

struct WrapPluginInfo
{
	HMODULE hDll;
	wchar_t PluginDll[MAX_PATH+1], IniFile[MAX_PATH+1];
	wchar_t Title[MAX_PATH+1], Desc[256], Author[256];
	wchar_t RegRoot[1024];
	VersionInfo Version;
	GUID guid_Plugin, guid_Dialogs;
	int nPluginMenu; GUID* guids_PluginMenu;
	int nPluginDisks; GUID* guids_PluginDisks;
	int nPluginConfig; GUID* guids_PluginConfig;
	Far2::PluginInfo Info;
	InfoPanelLine *InfoLines; size_t InfoLinesNumber; // Far3
	PanelMode     *PanelModesArray; size_t PanelModesNumber; // Far3
	KeyBarTitles   KeyBar; // Far3
	
	AnalyseInfo* pAnalyze;
	//;; 1 - (AnalyzeW [ & OpenW]) -> OpenFilePluginW
	//;; 2 - (AnalyzeW [ & OpenW]) -> OpenFilePluginW & ClosePluginW [& OpenFilePluginW]
	int AnalyzeMode;
	
	std::map<PluginPanelItem*,Far2::PluginPanelItem*> MapPanelItems;
	
	Far2Dialog* LastFar2Dlg;
	std::map<Far2Dialog*,HANDLE> MapDlg_2_3;
	std::map<HANDLE,Far2Dialog*> MapDlg_3_2;
	
	~WrapPluginInfo()
	{
		if (hDll)
		{
			FreeLibrary(hDll);
			hDll = NULL;
		}
		if (guids_PluginMenu)
			free(guids_PluginMenu);
		if (guids_PluginDisks)
			free(guids_PluginDisks);
		if (guids_PluginConfig)
			free(guids_PluginConfig);
		if (pAnalyze)
			free(pAnalyze);
		if (InfoLines)
			free(InfoLines);
		if (PanelModesArray)
			free(PanelModesArray);
		if (KeyBar.Labels)
			free(KeyBar.Labels);
	}

	WrapPluginInfo()
	{
		hDll = NULL;
		PluginDll[0] = IniFile[0] = Title[0] = Desc[0] = Author[0] = RegRoot[0] = 0;
		memset(&Version, 0, sizeof(Version));
		memset(&guid_Plugin, 0, sizeof(guid_Plugin));
		memset(&guid_Dialogs, 0, sizeof(guid_Dialogs));
		nPluginMenu = nPluginDisks = nPluginConfig = 0;
		guids_PluginMenu = guids_PluginDisks = guids_PluginConfig = NULL;
		memset(&Info, 0, sizeof(Info));
		InfoLines = NULL; InfoLinesNumber = 0;
		PanelModesArray = NULL; PanelModesNumber = 0;
		memset(&KeyBar, 0, sizeof(KeyBar));
		LastFar2Dlg = NULL;

		AnalyseW = NULL;
		ClosePluginW = NULL;
		CompareW = NULL;
		ConfigureW = NULL;
		DeleteFilesW = NULL;
		ExitFARW = NULL;
		FreeFindDataW = NULL;
		FreeVirtualFindDataW = NULL;
		GetFilesW = NULL;
		GetFindDataW = NULL;
		GetMinFarVersionW = NULL;
		GetOpenPluginInfoW = NULL;
		GetPluginInfoW = NULL;
		GetVirtualFindDataW = NULL;
		MakeDirectoryW = NULL;
		OpenFilePluginW = NULL;
		OpenPluginW = NULL;
		ProcessDialogEventW = NULL;
		ProcessEditorEventW = NULL;
		ProcessEditorInputW = NULL;
		ProcessEventW = NULL;
		ProcessHostFileW = NULL;
		ProcessKeyW = NULL;
		ProcessSynchroEventW = NULL;
		ProcessViewerEventW = NULL;
		PutFilesW = NULL;
		SetDirectoryW = NULL;
		SetFindListW  = NULL;
		SetStartupInfoW = NULL;
		pAnalyze = NULL;
	};
	
	// Exported Function
	typedef int    (WINAPI* _AnalyseW)(const struct AnalyseInfo *Info);
	_AnalyseW AnalyseW;
	typedef void   (WINAPI* _ClosePluginW)(HANDLE hPlugin);
	_ClosePluginW ClosePluginW;
	typedef int    (WINAPI* _CompareW)(HANDLE hPlugin,const Far2::PluginPanelItem *Item1,const Far2::PluginPanelItem *Item2,unsigned int Mode);
	_CompareW CompareW;
	typedef int    (WINAPI* _ConfigureW)(int ItemNumber);
	_ConfigureW ConfigureW;
	typedef int    (WINAPI* _DeleteFilesW)(HANDLE hPlugin,Far2::PluginPanelItem *PanelItem,int ItemsNumber,int OpMode);
	_DeleteFilesW DeleteFilesW;
	typedef void   (WINAPI* _ExitFARW)(void);
	_ExitFARW ExitFARW;
	typedef void   (WINAPI* _FreeFindDataW)(HANDLE hPlugin,Far2::PluginPanelItem *PanelItem,int ItemsNumber);
	_FreeFindDataW FreeFindDataW;
	typedef void   (WINAPI* _FreeVirtualFindDataW)(HANDLE hPlugin,Far2::PluginPanelItem *PanelItem,int ItemsNumber);
	_FreeVirtualFindDataW FreeVirtualFindDataW;
	typedef int    (WINAPI* _GetFilesW)(HANDLE hPlugin,Far2::PluginPanelItem *PanelItem,int ItemsNumber,int Move,const wchar_t **DestPath,int OpMode);
	_GetFilesW GetFilesW;
	typedef int    (WINAPI* _GetFindDataW)(HANDLE hPlugin,Far2::PluginPanelItem **pPanelItem,int *pItemsNumber,int OpMode);
	_GetFindDataW GetFindDataW;
	typedef int    (WINAPI* _GetMinFarVersionW)(void);
	_GetMinFarVersionW GetMinFarVersionW;
	typedef void   (WINAPI* _GetOpenPluginInfoW)(HANDLE hPlugin,Far2::OpenPluginInfo *Info);
	_GetOpenPluginInfoW GetOpenPluginInfoW;
	typedef void   (WINAPI* _GetPluginInfoW)(Far2::PluginInfo *Info);
	_GetPluginInfoW GetPluginInfoW;
	typedef int    (WINAPI* _GetVirtualFindDataW)(HANDLE hPlugin,Far2::PluginPanelItem **pPanelItem,int *pItemsNumber,const wchar_t *Path);
	_GetVirtualFindDataW GetVirtualFindDataW;
	typedef int    (WINAPI* _MakeDirectoryW)(HANDLE hPlugin,const wchar_t **Name,int OpMode);
	_MakeDirectoryW MakeDirectoryW;
	typedef HANDLE (WINAPI* _OpenFilePluginW)(const wchar_t *Name,const unsigned char *Data,int DataSize,int OpMode);
	_OpenFilePluginW OpenFilePluginW;
	typedef HANDLE (WINAPI* _OpenPluginW)(int OpenFrom,INT_PTR Item);
	_OpenPluginW OpenPluginW;
	typedef int    (WINAPI* _ProcessDialogEventW)(int Event,void *Param);
	_ProcessDialogEventW ProcessDialogEventW;
	typedef int    (WINAPI* _ProcessEditorEventW)(int Event,void *Param);
	_ProcessEditorEventW ProcessEditorEventW;
	typedef int    (WINAPI* _ProcessEditorInputW)(const INPUT_RECORD *Rec);
	_ProcessEditorInputW ProcessEditorInputW;
	typedef int    (WINAPI* _ProcessEventW)(HANDLE hPlugin,int Event,void *Param);
	_ProcessEventW ProcessEventW;
	typedef int    (WINAPI* _ProcessHostFileW)(HANDLE hPlugin,Far2::PluginPanelItem *PanelItem,int ItemsNumber,int OpMode);
	_ProcessHostFileW ProcessHostFileW;
	typedef int    (WINAPI* _ProcessKeyW)(HANDLE hPlugin,int Key,unsigned int ControlState);
	_ProcessKeyW ProcessKeyW;
	typedef int    (WINAPI* _ProcessSynchroEventW)(int Event,void *Param);
	_ProcessSynchroEventW ProcessSynchroEventW;
	typedef int    (WINAPI* _ProcessViewerEventW)(int Event,void *Param);
	_ProcessViewerEventW ProcessViewerEventW;
	typedef int    (WINAPI* _PutFilesW)(HANDLE hPlugin,Far2::PluginPanelItem *PanelItem,int ItemsNumber,int Move,const wchar_t *SrcPath,int OpMode);
	_PutFilesW PutFilesW;
	typedef int    (WINAPI* _SetDirectoryW)(HANDLE hPlugin,const wchar_t *Dir,int OpMode);
	_SetDirectoryW SetDirectoryW;
	typedef int    (WINAPI* _SetFindListW)(HANDLE hPlugin,const Far2::PluginPanelItem *PanelItem,int ItemsNumber);
	_SetFindListW SetFindListW;
	typedef void   (WINAPI* _SetStartupInfoW)(const Far2::PluginStartupInfo *Info);
	_SetStartupInfoW SetStartupInfoW;
	
	// Changed functions
	static LONG_PTR WINAPI FarApiDefDlgProc
	(
	    HANDLE   hDlg,
	    int      Msg,
	    int      Param1,
	    LONG_PTR Param2
	)
	{
		LONG_PTR lRc = 0;
		HANDLE hDlg3 = wpi->MapDlg_2_3[(Far2Dialog*)hDlg];
		if (hDlg3)
		{
			//TODO: Скорее всего потребуется переработка аргументов для некоторых сообщений
			int Msg3 = FarMessage_2_3(Msg, Param2);
			lRc = psi3.DefDlgProc(hDlg3, Msg3, Param1, Param2);
		}
		else
		{
			_ASSERTE(hDlg3!=NULL);
		}
		return lRc;
	}
	static LONG_PTR WINAPI FarApiSendDlgMessage
	(
	    HANDLE   hDlg,
	    int      Msg,
	    int      Param1,
	    LONG_PTR Param2
	)
	{
		LONG_PTR lRc = 0;
		HANDLE hDlg3 = wpi->MapDlg_2_3[(Far2Dialog*)hDlg];
		if (hDlg3)
		{
			//TODO: Скорее всего потребуется переработка аргументов для некоторых сообщений
			int Msg3 = FarMessage_2_3(Msg, Param2);
			lRc = psi3.SendDlgMessage(hDlg3, Msg3, Param1, Param2);
		}
		else
		{
			_ASSERTE(hDlg3!=NULL);
		}
		return lRc;
	}
	static BOOL WINAPI FarApiShowHelp
	(
	    const wchar_t *ModuleName,
	    const wchar_t *Topic,
	    DWORD Flags
	)
	{
		FARHELPFLAGS Flags3 = 0
			| ((Flags & Far2::FHELP_NOSHOWERROR) ? FHELP_NOSHOWERROR : 0)
			| ((Flags & Far2::FHELP_FARHELP) ? FHELP_FARHELP : 0)
			| ((Flags & Far2::FHELP_CUSTOMFILE) ? FHELP_CUSTOMFILE : 0)
			| ((Flags & Far2::FHELP_CUSTOMPATH) ? FHELP_CUSTOMPATH : 0)
			| ((Flags & Far2::FHELP_USECONTENTS) ? FHELP_USECONTENTS : 0);
		int iRc = psi3.ShowHelp(ModuleName, Topic, Flags3);
		return iRc;
	}
	static HANDLE WINAPI FarApiSaveScreen(int X1, int Y1, int X2, int Y2)
	{
		return psi3.SaveScreen(X1,Y1,X2,Y2);
	}
	static void WINAPI FarApiRestoreScreen(HANDLE hScreen)
	{
		psi3.RestoreScreen(hScreen);
	}
	static void WINAPI FarApiText
	(
	    int X,
	    int Y,
	    int Color,
	    const wchar_t *Str
	)
	{
		psi3.Text(X,Y,Color,Str);
	}
	static int WINAPI FarApiEditor
	(
	    const wchar_t *FileName,
	    const wchar_t *Title,
	    int X1,
	    int Y1,
	    int X2,
	    int Y2,
	    DWORD Flags,
	    int StartLine,
	    int StartChar,
	    UINT CodePage
	)
	{
		EDITOR_FLAGS Flags3 = 0
			| ((Flags & Far2::EF_NONMODAL) ? EF_NONMODAL : 0)
			| ((Flags & Far2::EF_CREATENEW) ? EF_CREATENEW : 0)
			| ((Flags & Far2::EF_ENABLE_F6) ? EF_ENABLE_F6 : 0)
			| ((Flags & Far2::EF_DISABLEHISTORY) ? EF_DISABLEHISTORY : 0)
			| ((Flags & Far2::EF_DELETEONCLOSE) ? EF_DELETEONCLOSE : 0)
			| ((Flags & Far2::EF_IMMEDIATERETURN) ? EF_IMMEDIATERETURN : 0)
			| ((Flags & Far2::EF_DELETEONLYFILEONCLOSE) ? EF_DELETEONLYFILEONCLOSE : 0);
		int iRc = psi3.Editor(FileName, Title, X1,Y1,X2,Y2, Flags3, StartLine, StartChar, CodePage);
		return iRc;
	}
	static int WINAPI FarApiViewer
	(
	    const wchar_t *FileName,
	    const wchar_t *Title,
	    int X1,
	    int Y1,
	    int X2,
	    int Y2,
	    DWORD Flags,
	    UINT CodePage
	)
	{
		VIEWER_FLAGS Flags3 = 0
			| ((Flags & Far2::VF_NONMODAL) ? VF_NONMODAL : 0)
			| ((Flags & Far2::VF_DELETEONCLOSE) ? VF_DELETEONCLOSE : 0)
			| ((Flags & Far2::VF_ENABLE_F6) ? VF_ENABLE_F6 : 0)
			| ((Flags & Far2::VF_DISABLEHISTORY) ? VF_DISABLEHISTORY : 0)
			| ((Flags & Far2::VF_IMMEDIATERETURN) ? VF_IMMEDIATERETURN : 0)
			| ((Flags & Far2::VF_DELETEONLYFILEONCLOSE) ? VF_DELETEONLYFILEONCLOSE : 0);
		int iRc = psi3.Viewer(FileName, Title, X1,Y1,X2,Y2, Flags3, CodePage);
		return iRc;
	};
	static int WINAPI FarApiMenu
	(
	    INT_PTR             PluginNumber,
	    int                 X,
	    int                 Y,
	    int                 MaxHeight,
	    DWORD               Flags,
	    const wchar_t      *Title,
	    const wchar_t      *Bottom,
	    const wchar_t      *HelpTopic,
	    const int          *BreakKeys,
	    int                *BreakCode,
	    const struct Far2::FarMenuItem *Item,
	    int                 ItemsNumber
	)
	{
		int iRc = -1;
		FarMenuItem *pItems3 = NULL;
		FarKey *pBreak3 = NULL;
		
		if (Item && ItemsNumber > 0)
		{
			pItems3 = (FarMenuItem*)calloc(ItemsNumber, sizeof(*pItems3));
			if (!(Flags & Far2::FMENU_USEEXT))
			{
				const Far2::FarMenuItem* p2 = Item;
				FarMenuItem* p3 = pItems3;
				for (int i = 0; i < ItemsNumber; i++, p2++, p3++)
				{
					p3->Text = p2->Text;
					p3->Flags = 0
						| (p2->Selected ? MIF_SELECTED : 0)
						| (p2->Checked ? MIF_CHECKED : 0)
						| (p2->Separator ? MIF_SEPARATOR : 0);
				}
			}
			else
			{
				const Far2::FarMenuItemEx* p2 = (const Far2::FarMenuItemEx*)Item;
				FarMenuItem* p3 = pItems3;
				for (int i = 0; i < ItemsNumber; i++, p2++, p3++)
				{
					p3->Text = p2->Text;
					p3->Flags = 0
						| ((p2->Flags & Far2::MIF_SELECTED) ? MIF_SELECTED : 0)
						| ((p2->Flags & Far2::MIF_CHECKED) ? MIF_CHECKED : 0)
						| ((p2->Flags & Far2::MIF_SEPARATOR) ? MIF_SEPARATOR : 0)
						| ((p2->Flags & Far2::MIF_DISABLE) ? MIF_DISABLE : 0)
						| ((p2->Flags & Far2::MIF_GRAYED) ? MIF_GRAYED : 0)
						| ((p2->Flags & Far2::MIF_HIDDEN) ? MIF_HIDDEN : 0);
					p3->AccelKey = p2->AccelKey;
					p3->Reserved = p2->Reserved;
					p3->UserData = p2->UserData;
				}
			}
			
			if (BreakKeys)
			{
				int cnt = 0;
				for (int i = 0; BreakKeys[i]; i++)
					cnt++;
				pBreak3 = (FarKey*)calloc(cnt+1, sizeof(*pBreak3));
				for (int i = 0; BreakKeys[i]; i++)
				{
					pBreak3[i].VirtualKeyCode = LOWORD(BreakKeys[i]);
					pBreak3[i].ControlKeyState = 0
						| ((HIWORD(BreakKeys[i]) & Far2::PKF_CONTROL) ? LEFT_CTRL_PRESSED : 0)
						| ((HIWORD(BreakKeys[i]) & Far2::PKF_ALT) ? LEFT_ALT_PRESSED : 0)
						| ((HIWORD(BreakKeys[i]) & Far2::PKF_SHIFT) ? SHIFT_PRESSED : 0);
				}
			}
			
			FARMENUFLAGS Flags3 = 0
				| ((Flags & Far2::FMENU_SHOWAMPERSAND) ? FMENU_SHOWAMPERSAND : 0)
				| ((Flags & Far2::FMENU_WRAPMODE) ? FMENU_WRAPMODE : 0)
				| ((Flags & Far2::FMENU_AUTOHIGHLIGHT) ? FMENU_AUTOHIGHLIGHT : 0)
				| ((Flags & Far2::FMENU_REVERSEAUTOHIGHLIGHT) ? FMENU_REVERSEAUTOHIGHLIGHT : 0)
				| ((Flags & Far2::FMENU_CHANGECONSOLETITLE) ? FMENU_CHANGECONSOLETITLE : 0);
			
			iRc = psi3.Menu(&wpi->guid_Plugin, X, Y, MaxHeight, Flags3,
					Title, Bottom, HelpTopic, pBreak3, BreakCode, pItems3, ItemsNumber);
		}
		
		if (pItems3)
			free(pItems3);
		if (pBreak3)
			free(pBreak3);
		return iRc;
	};
	static int WINAPI FarApiMessage
	(
	    INT_PTR PluginNumber,
	    DWORD Flags,
	    const wchar_t *HelpTopic,
	    const wchar_t * const *Items,
	    int ItemsNumber,
	    int ButtonsNumber
	)
	{
		int iRc = -1;

		DWORD Far3Flags = 0
			| ((Flags & Far2::FMSG_WARNING) ? FMSG_WARNING : 0)
			| ((Flags & Far2::FMSG_ERRORTYPE) ? FMSG_ERRORTYPE : 0)
			| ((Flags & Far2::FMSG_KEEPBACKGROUND) ? FMSG_KEEPBACKGROUND : 0)
			| ((Flags & Far2::FMSG_LEFTALIGN) ? FMSG_LEFTALIGN : 0)
			| ((Flags & Far2::FMSG_ALLINONE) ? FMSG_ALLINONE : 0);
			
		if ((Flags & 0x000F0000) == Far2::FMSG_MB_OK)
			Far3Flags |= FMSG_MB_OK;
		else if ((Flags & 0x000F0000) == Far2::FMSG_MB_OKCANCEL)
			Far3Flags |= FMSG_MB_OKCANCEL;
		else if ((Flags & 0x000F0000) == Far2::FMSG_MB_ABORTRETRYIGNORE)
			Far3Flags |= FMSG_MB_ABORTRETRYIGNORE;
		else if ((Flags & 0x000F0000) == Far2::FMSG_MB_YESNO)
			Far3Flags |= FMSG_MB_YESNO;
		else if ((Flags & 0x000F0000) == Far2::FMSG_MB_YESNOCANCEL)
			Far3Flags |= FMSG_MB_YESNOCANCEL;
		else if ((Flags & 0x000F0000) == Far2::FMSG_MB_RETRYCANCEL)
			Far3Flags |= FMSG_MB_RETRYCANCEL;
		
		iRc = psi3.Message(&wpi->guid_Plugin, Far3Flags, 
    				HelpTopic, Items, ItemsNumber, ButtonsNumber);
    	return iRc;
	};
	static HANDLE WINAPI FarApiDialogInit
	(
	    INT_PTR               PluginNumber,
	    int                   X1,
	    int                   Y1,
	    int                   X2,
	    int                   Y2,
	    const wchar_t        *HelpTopic,
	    struct Far2::FarDialogItem *Item,
	    unsigned int          ItemsNumber,
	    DWORD                 Reserved,
	    DWORD                 Flags,
	    Far2::FARWINDOWPROC   DlgProc,
	    LONG_PTR              Param
	)
	{
		Far2Dialog *p = new Far2Dialog(X1, Y1, X2, Y2,
	    	HelpTopic, Item, ItemsNumber, Flags, DlgProc, Param,
	    	wpi->guid_Plugin, wpi->guid_Dialogs);
		return (HANDLE)p;
	};
	static int WINAPI FarApiDialogRun
	(
	    HANDLE hDlg
	)
	{
		Far2Dialog *p = (Far2Dialog*)hDlg;
		if (!p)
			return -1;
		return p->RunDlg();
	}
	static void WINAPI FarApiDialogFree
	(
	    HANDLE hDlg
	)
	{
		Far2Dialog *p = (Far2Dialog*)hDlg;
		if (!p)
			return;
		delete p;
	};
	static int WINAPI FarApiControl
	(
	    HANDLE hPlugin,
	    int Command,
	    int Param1,
	    LONG_PTR Param2
	)
	{
		//TODO:
		return 0;
	};
	static int WINAPI FarApiGetDirList
	(
	    const wchar_t *Dir,
	    struct Far2::FAR_FIND_DATA **pPanelItem,
	    int *pItemsNumber
	)
	{
		//TODO:
		return 0;
	};
	static void WINAPI FarApiFreeDirList(struct Far2::FAR_FIND_DATA *PanelItem, int nItemsNumber)
	{
		//TODO:
	};
	static int WINAPI FarApiGetPluginDirList
	(
	    INT_PTR PluginNumber,
	    HANDLE hPlugin,
	    const wchar_t *Dir,
	    struct Far2::PluginPanelItem **pPanelItem,
	    int *pItemsNumber
	)
	{
		//TODO:
		return 0;
	};
	static void WINAPI FarApiFreePluginDirList(struct Far2::PluginPanelItem *PanelItem, int nItemsNumber)
	{
		//TODO:
	};
	static int WINAPI FarApiCmpName
	(
	    const wchar_t *Pattern,
	    const wchar_t *String,
	    int SkipPath
	)
	{
		//TODO:
		return 0;
	};
	static const wchar_t* WINAPI FarApiGetMsg
	(
	    INT_PTR PluginNumber,
	    int MsgId
	)
	{
		if (((INT_PTR)wpi->hDll) == PluginNumber)
			return psi3.GetMsg(&wpi->guid_Plugin, MsgId);
		return L"";
	};
	static INT_PTR WINAPI FarApiAdvControl
	(
	    INT_PTR ModuleNumber,
	    int Command,
	    void *Param
	)
	{
		//TODO:
		return 0;
	};
	static int WINAPI FarApiViewerControl
	(
	    int Command,
	    void *Param
	)
	{
		//TODO:
		return 0;
	};
	static int WINAPI FarApiEditorControl
	(
	    int Command,
	    void *Param
	)
	{
		//TODO:
		return 0;
	};
	static int WINAPI FarApiInputBox
	(
	    const wchar_t *Title,
	    const wchar_t *SubTitle,
	    const wchar_t *HistoryName,
	    const wchar_t *SrcText,
	    wchar_t *DestText,
	    int   DestLength,
	    const wchar_t *HelpTopic,
	    DWORD Flags
	)
	{
		//TODO:
		return 0;
	};
	static int WINAPI FarApiPluginsControl
	(
	    HANDLE hHandle,
	    int Command,
	    int Param1,
	    LONG_PTR Param2
	)
	{
		//TODO:
		return 0;
	};
	static int WINAPI FarApiFileFilterControl
	(
	    HANDLE hHandle,
	    int Command,
	    int Param1,
	    LONG_PTR Param2
	)
	{
		//TODO:
		return 0;
	};
	static int WINAPI FarApiRegExpControl
	(
	    HANDLE hHandle,
	    int Command,
	    LONG_PTR Param
	)
	{
		//TODO:
		return 0;
	};
	//struct int WINAPIV FARSTDSPRINTF(wchar_t *Buffer,const wchar_t *Format,...);
	//struct int WINAPIV FARSTDSNPRINTF(wchar_t *Buffer,size_t Sizebuf,const wchar_t *Format,...);
	//struct int WINAPIV FARSTDSSCANF(const wchar_t *Buffer, const wchar_t *Format,...);
	//struct void WINAPI FARSTDQSORT(void *base, size_t nelem, size_t width, int (__cdecl *fcmp)(const void *, const void *));
	//struct void WINAPI FARSTDQSORTEX(void *base, size_t nelem, size_t width, int (__cdecl *fcmp)(const void *, const void *,void *userparam),void *userparam);
	//struct void   *WINAPI FARSTDBSEARCH(const void *key, const void *base, size_t nelem, size_t width, int (__cdecl *fcmp)(const void *, const void *));
	static int WINAPI FarStdGetFileOwner(const wchar_t *Computer,const wchar_t *Name,wchar_t *Owner,int Size)
	{
		//TODO:
		return 0;
	};
	//struct int WINAPI FARSTDGETNUMBEROFLINKS(const wchar_t *Name);
	//struct int WINAPI FARSTDATOI(const wchar_t *s);
	//struct __int64 WINAPI FARSTDATOI64(const wchar_t *s);
	//struct wchar_t   *WINAPI FARSTDITOA64(__int64 value, wchar_t *string, int radix);
	//struct wchar_t   *WINAPI FARSTDITOA(int value, wchar_t *string, int radix);
	//struct wchar_t   *WINAPI FARSTDLTRIM(wchar_t *Str);
	//struct wchar_t   *WINAPI FARSTDRTRIM(wchar_t *Str);
	//struct wchar_t   *WINAPI FARSTDTRIM(wchar_t *Str);
	//struct wchar_t   *WINAPI FARSTDTRUNCSTR(wchar_t *Str,int MaxLength);
	//struct wchar_t   *WINAPI FARSTDTRUNCPATHSTR(wchar_t *Str,int MaxLength);
	//struct wchar_t   *WINAPI FARSTDQUOTESPACEONLY(wchar_t *Str);
	//struct const wchar_t*WINAPI FARSTDPOINTTONAME(const wchar_t *Path);
	static int WINAPI FarStdGetPathRoot(const wchar_t *Path,wchar_t *Root, int DestSize)
	{
		//TODO:
		return 0;
	};
	//struct BOOL WINAPI FARSTDADDENDSLASH(wchar_t *Path);
	//struct int WINAPI FARSTDCOPYTOCLIPBOARD(const wchar_t *Data);
	//struct wchar_t *WINAPI FARSTDPASTEFROMCLIPBOARD(void);
	//struct int WINAPI FARSTDINPUTRECORDTOKEY(const INPUT_RECORD *r);
	//struct int WINAPI FARSTDLOCALISLOWER(wchar_t Ch);
	//struct int WINAPI FARSTDLOCALISUPPER(wchar_t Ch);
	//struct int WINAPI FARSTDLOCALISALPHA(wchar_t Ch);
	//struct int WINAPI FARSTDLOCALISALPHANUM(wchar_t Ch);
	//struct wchar_t WINAPI FARSTDLOCALUPPER(wchar_t LowerChar);
	//struct wchar_t WINAPI FARSTDLOCALLOWER(wchar_t UpperChar);
	//struct void WINAPI FARSTDLOCALUPPERBUF(wchar_t *Buf,int Length);
	//struct void WINAPI FARSTDLOCALLOWERBUF(wchar_t *Buf,int Length);
	//struct void WINAPI FARSTDLOCALSTRUPR(wchar_t *s1);
	//struct void WINAPI FARSTDLOCALSTRLWR(wchar_t *s1);
	//struct int WINAPI FARSTDLOCALSTRICMP(const wchar_t *s1,const wchar_t *s2);
	//struct int WINAPI FARSTDLOCALSTRNICMP(const wchar_t *s1,const wchar_t *s2,int n);
	static wchar_t* WINAPI FarStdXlat(wchar_t *Line,int StartPos,int EndPos,DWORD Flags)
	{
		//TODO:
		return NULL;
	};
	static void WINAPI FarStdRecursiveSearch(const wchar_t *InitDir,const wchar_t *Mask,Far2::FRSUSERFUNC Func,DWORD Flags,void *Param)
	{
		//TODO:
	};
	static int WINAPI FarStdMkTemp(wchar_t *Dest, DWORD size, const wchar_t *Prefix)
	{
		//TODO:
		return 0;
	};
	static int WINAPI FarStdProcessName(const wchar_t *param1, wchar_t *param2, DWORD size, DWORD flags)
	{
		//TODO:
		return 0;
	};
	static int WINAPI FarStdMkLink(const wchar_t *Src,const wchar_t *Dest,DWORD Flags)
	{
		//TODO:
		return 0;
	};
	static int WINAPI FarConvertPath(enum Far2::CONVERTPATHMODES Mode, const wchar_t *Src, wchar_t *Dest, int DestSize)
	{
		//TODO:
		return 0;
	};
	static int WINAPI FarGetReparsePointInfo(const wchar_t *Src, wchar_t *Dest,int DestSize)
	{
		//TODO:
		return 0;
	};
	static DWORD WINAPI FarGetCurrentDirectory(DWORD Size,wchar_t* Buffer)
	{
		//TODO:
		return 0;
	};
};

InfoPanelLine* InfoLines_2_3(const Far2::InfoPanelLine *InfoLines, int InfoLinesNumber)
{
	if (wpi->InfoLines)
	{
		free(wpi->InfoLines);
		wpi->InfoLines = NULL;
	}
	if (InfoLines && InfoLinesNumber > 0)
	{	
		wpi->InfoLines = (InfoPanelLine*)calloc(InfoLinesNumber, sizeof(*wpi->InfoLines));
		for (int i = 0; i < InfoLinesNumber; i++)
		{
			wpi->InfoLines[i].Text = InfoLines[i].Text;
			wpi->InfoLines[i].Data = InfoLines[i].Data;
			wpi->InfoLines[i].Separator = InfoLines[i].Separator;
		}
	}
	return wpi->InfoLines;
}

PanelMode* PanelModes_2_3(const Far2::PanelMode *PanelModesArray, int PanelModesNumber)
{
	if (wpi->PanelModesArray)
	{
		free(wpi->PanelModesArray);
		wpi->PanelModesArray = NULL;
	}
	if (PanelModesArray && PanelModesNumber > 0)
	{
		wpi->PanelModesArray = (PanelMode*)calloc(PanelModesNumber, sizeof(*wpi->PanelModesArray));
		for (int i = 0; i < PanelModesNumber; i++)
		{
			wpi->PanelModesArray[i].StructSize = sizeof(*wpi->PanelModesArray);
			wpi->PanelModesArray[i].ColumnTypes = PanelModesArray[i].ColumnTypes;
			wpi->PanelModesArray[i].ColumnWidths = PanelModesArray[i].ColumnWidths;
			wpi->PanelModesArray[i].ColumnTitles = PanelModesArray[i].ColumnTitles;
			wpi->PanelModesArray[i].StatusColumnTypes = PanelModesArray[i].StatusColumnTypes;
			wpi->PanelModesArray[i].StatusColumnWidths = PanelModesArray[i].StatusColumnWidths;
			wpi->PanelModesArray[i].Flags = 
				(PanelModesArray[i].FullScreen ? PMFLAGS_FULLSCREEN : 0) |
				(PanelModesArray[i].DetailedStatus ? PMFLAGS_DETAILEDSTATUS : 0) |
				(PanelModesArray[i].AlignExtensions ? PMFLAGS_ALIGNEXTENSIONS : 0) |
				(PanelModesArray[i].CaseConversion ? PMFLAGS_CASECONVERSION : 0);
		}
	}
	return wpi->PanelModesArray;
}

KeyBarTitles* KeyBarTitles_2_3(const Far2::KeyBarTitles* KeyBar)
{
	if (wpi->KeyBar.Labels)
	{
		free(wpi->KeyBar.Labels);
		wpi->KeyBar.Labels = NULL;
	}

	size_t cnt = 0; // Max count. Real count may be less
	
	struct { wchar_t* const* Titles; DWORD ctrl; } src[] = 
	{
		{KeyBar->Titles, 0},
		{KeyBar->CtrlTitles, LEFT_CTRL_PRESSED},
		{KeyBar->AltTitles, LEFT_ALT_PRESSED},
		{KeyBar->ShiftTitles, SHIFT_PRESSED},
		{KeyBar->CtrlShiftTitles, LEFT_CTRL_PRESSED|SHIFT_PRESSED},
		{KeyBar->AltShiftTitles, LEFT_ALT_PRESSED|SHIFT_PRESSED},
		{KeyBar->CtrlAltTitles, LEFT_CTRL_PRESSED|LEFT_ALT_PRESSED}
	};
	
	for (int i = 0; i < ARRAYSIZE(src); i++)
	{
		if (src[i].Titles) cnt += 12;
	}
	
	wpi->KeyBar.CountLabels = 0;
	
	if (cnt > 0)
	{
		wpi->KeyBar.Labels = (KeyBarLabel*)calloc(cnt, sizeof(*wpi->KeyBar.Labels));
		KeyBarLabel *p = wpi->KeyBar.Labels;
		for (int i = 0; i < ARRAYSIZE(src); i++)
		{
			if (!src[i].Titles)
				continue;
			for (int j = 0; j < 12; j++)
			{
				if (!src[i].Titles[j])
					continue;
				p->Key.VirtualKeyCode = VK_F1 + j;
				p->Key.ControlKeyState = src[i].ctrl;
				p->Text = p->LongText = src[i].Titles[j];
				p++;
			}
		}
		wpi->KeyBar.CountLabels = p - wpi->KeyBar.Labels;
	}

	return &wpi->KeyBar;
}

void LoadPluginInfo()
{
	if (wpi)
		return; // уже
		
	BOOL lbRc = FALSE;
	wchar_t szSelf[MAX_PATH+1]; szSelf[0] = 0;
	wchar_t szIni[MAX_PATH+1], szTemp[2048];
	wpi = new WrapPluginInfo;
	
	if (GetModuleFileName(ghInstance, szSelf, ARRAYSIZE(szSelf)-4))
	{
		lstrcpy(szIni, szSelf);
		wchar_t* pszSlash = wcsrchr(szIni, L'\\');
		if (!pszSlash) pszSlash = szIni;
		wchar_t* pszDot = wcsrchr(pszSlash, L'.');
		if (pszDot)
			*pszDot = 0;
		lstrcat(szIni, L".ini");
		lstrcpy(wpi->IniFile, szIni);

		lstrcpy(wpi->PluginDll, szSelf);
		pszSlash = wcsrchr(wpi->PluginDll, L'\\');
		if (pszSlash) pszSlash++; else pszSlash = wpi->PluginDll;
		if (!GetPrivateProfileString(L"Plugin", L"PluginFile", L"", pszSlash, ARRAYSIZE(wpi->PluginDll)-lstrlen(wpi->PluginDll), szIni))
			wpi->PluginDll[0] = 0;
			
		wpi->AnalyzeMode = GetPrivateProfileInt(L"Plugin", L"AnalyzeMode", 2, szIni);
		if (wpi->AnalyzeMode != 1 && wpi->AnalyzeMode != 2)
			wpi->AnalyzeMode = 2;
		
		if (GetPrivateProfileString(L"Plugin", L"Title", L"Sample Far3 plugin", szTemp, ARRAYSIZE(szTemp), szIni))
			lstrcpyn(wpi->Title, szTemp, ARRAYSIZE(wpi->Title));
		if (GetPrivateProfileString(L"Plugin", L"Description", L"Far2->Far3 plugin wrapper", szTemp, ARRAYSIZE(szTemp), szIni))
			lstrcpyn(wpi->Desc, szTemp, ARRAYSIZE(wpi->Desc));
		if (GetPrivateProfileString(L"Plugin", L"Author", L"Maximus5", szTemp, ARRAYSIZE(szTemp), szIni))
			lstrcpyn(wpi->Author, szTemp, ARRAYSIZE(wpi->Author));
		if (GetPrivateProfileString(L"Plugin", L"RegRoot", L"Software\\Far Manager\\Plugins", szTemp, ARRAYSIZE(szTemp), szIni))
			lstrcpyn(wpi->RegRoot, szTemp, ARRAYSIZE(wpi->RegRoot));
		if (GetPrivateProfileString(L"Plugin", L"Version", L"1.0.0.0", szTemp, ARRAYSIZE(szTemp), szIni))
		{
			//TODO: Обработка версии
		}
		GUID guid;
		if (GetPrivateProfileString(L"Plugin", L"GUID", L"", szTemp, ARRAYSIZE(szTemp), szIni))
		{
			if (UuidFromStringW((RPC_WSTR)szTemp, &guid) == RPC_S_OK)
				wpi->guid_Plugin = guid;
			else
				wpi->guid_Plugin = guid_DefPlugin;
		}
		//if (GetPrivateProfileString(L"Plugin", L"MenuGUID", L"", szTemp, ARRAYSIZE(szTemp), szIni))
		//{
		//	if (UuidFromStringW(szTemp, &guid) == RPC_S_OK)
		//		wpi->guid_PluginMenu = guid;
		//	else
		//		wpi->guid_PluginMenu = ::guid_DefPluginMenu;
		//}
		//if (GetPrivateProfileString(L"Plugin", L"ConfigGUID", L"", szTemp, ARRAYSIZE(szTemp), szIni))
		//{
		//	if (UuidFromStringW(szTemp, &guid) == RPC_S_OK)
		//		wpi->guid_PluginConfigMenu = guid;
		//	else
		//		wpi->guid_PluginConfigMenu = ::guid_DefPluginConfigMenu;
		//}
		if (GetPrivateProfileString(L"Plugin", L"DialogsGUID", L"", szTemp, ARRAYSIZE(szTemp), szIni))
		{
			if (UuidFromStringW((RPC_WSTR)szTemp, &guid) == RPC_S_OK)
				wpi->guid_Dialogs = guid;
			else
				wpi->guid_Dialogs = ::guid_DefDialogs;
		}
		lbRc = TRUE;
	}
	
	if (!lbRc)
	{
		lstrcpyn(wpi->Title, szSelf[0] ? szSelf : L"Far3Wrap", ARRAYSIZE(wpi->Title));
		lstrcpy(wpi->Desc, L"Far2->Far3 plugin wrapper");
		lstrcpy(wpi->Author, L"Maximus5");
		lstrcpy(wpi->RegRoot, L"Software\\Far Manager\\Plugins");
		wpi->guid_Plugin = guid_DefPlugin;
		//wpi->guid_PluginMenu = ::guid_DefPluginMenu;
		//wpi->guid_PluginConfigMenu = ::guid_DefPluginConfigMenu;
		wpi->guid_Dialogs = ::guid_DefDialogs;
	}
}

BOOL LoadPlugin(BOOL abSilent)
{
	if (!wpi || !*wpi->PluginDll)
	{
		return FALSE;
	}
	
	if (wpi->hDll == NULL)
	{
		wpi->hDll = LoadLibrary(wpi->PluginDll);
		if (wpi->hDll == NULL)
		{
			//TODO: Обработка ошибок загрузки
		}
		else
		{
			wpi->AnalyseW = (WrapPluginInfo::_AnalyseW)GetProcAddress(wpi->hDll, "AnalyseW");
			wpi->ClosePluginW = (WrapPluginInfo::_ClosePluginW)GetProcAddress(wpi->hDll, "ClosePluginW");
			wpi->CompareW = (WrapPluginInfo::_CompareW)GetProcAddress(wpi->hDll, "CompareW");
			wpi->ConfigureW = (WrapPluginInfo::_ConfigureW)GetProcAddress(wpi->hDll, "ConfigureW");
			wpi->DeleteFilesW = (WrapPluginInfo::_DeleteFilesW)GetProcAddress(wpi->hDll, "DeleteFilesW");
			wpi->ExitFARW = (WrapPluginInfo::_ExitFARW)GetProcAddress(wpi->hDll, "ExitFARW");
			wpi->FreeFindDataW = (WrapPluginInfo::_FreeFindDataW)GetProcAddress(wpi->hDll, "FreeFindDataW");
			wpi->FreeVirtualFindDataW = (WrapPluginInfo::_FreeVirtualFindDataW)GetProcAddress(wpi->hDll, "FreeVirtualFindDataW");
			wpi->GetFilesW = (WrapPluginInfo::_GetFilesW)GetProcAddress(wpi->hDll, "GetFilesW");
			wpi->GetFindDataW = (WrapPluginInfo::_GetFindDataW)GetProcAddress(wpi->hDll, "GetFindDataW");
			wpi->GetMinFarVersionW = (WrapPluginInfo::_GetMinFarVersionW)GetProcAddress(wpi->hDll, "GetMinFarVersionW");
			wpi->GetOpenPluginInfoW = (WrapPluginInfo::_GetOpenPluginInfoW)GetProcAddress(wpi->hDll, "GetOpenPluginInfoW");
			wpi->GetPluginInfoW = (WrapPluginInfo::_GetPluginInfoW)GetProcAddress(wpi->hDll, "GetPluginInfoW");
			wpi->GetVirtualFindDataW = (WrapPluginInfo::_GetVirtualFindDataW)GetProcAddress(wpi->hDll, "GetVirtualFindDataW");
			wpi->MakeDirectoryW = (WrapPluginInfo::_MakeDirectoryW)GetProcAddress(wpi->hDll, "MakeDirectoryW");
			wpi->OpenFilePluginW = (WrapPluginInfo::_OpenFilePluginW)GetProcAddress(wpi->hDll, "OpenFilePluginW");
			wpi->OpenPluginW = (WrapPluginInfo::_OpenPluginW)GetProcAddress(wpi->hDll, "OpenPluginW");
			wpi->ProcessDialogEventW = (WrapPluginInfo::_ProcessDialogEventW)GetProcAddress(wpi->hDll, "ProcessDialogEventW");
			wpi->ProcessEditorEventW = (WrapPluginInfo::_ProcessEditorEventW)GetProcAddress(wpi->hDll, "ProcessEditorEventW");
			wpi->ProcessEditorInputW = (WrapPluginInfo::_ProcessEditorInputW)GetProcAddress(wpi->hDll, "ProcessEditorInputW");
			wpi->ProcessEventW = (WrapPluginInfo::_ProcessEventW)GetProcAddress(wpi->hDll, "ProcessEventW");
			wpi->ProcessHostFileW = (WrapPluginInfo::_ProcessHostFileW)GetProcAddress(wpi->hDll, "ProcessHostFileW");
			wpi->ProcessKeyW = (WrapPluginInfo::_ProcessKeyW)GetProcAddress(wpi->hDll, "ProcessKeyW");
			wpi->ProcessSynchroEventW = (WrapPluginInfo::_ProcessSynchroEventW)GetProcAddress(wpi->hDll, "ProcessSynchroEventW");
			wpi->ProcessViewerEventW = (WrapPluginInfo::_ProcessViewerEventW)GetProcAddress(wpi->hDll, "ProcessViewerEventW");
			wpi->PutFilesW = (WrapPluginInfo::_PutFilesW)GetProcAddress(wpi->hDll, "PutFilesW");
			wpi->SetDirectoryW = (WrapPluginInfo::_SetDirectoryW)GetProcAddress(wpi->hDll, "SetDirectoryW");
			wpi->SetFindListW = (WrapPluginInfo::_SetFindListW)GetProcAddress(wpi->hDll, "SetFindListW");
			wpi->SetStartupInfoW = (WrapPluginInfo::_SetStartupInfoW)GetProcAddress(wpi->hDll, "SetStartupInfoW");
		}
	}
	
	return (wpi->hDll != NULL);
}


BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
                     )
{
    if (ghInstance == NULL)
    {
		ghInstance = (HMODULE)hModule;

		lbPsi2 = FALSE;
		memset(&psi3, 0, sizeof(psi3));
		memset(&psi2, 0, sizeof(psi2));
		memset(&FSF3, 0, sizeof(FSF3));
		memset(&FSF2, 0, sizeof(FSF2));

		wpi = NULL;
		LoadPluginInfo();
    }
    if (ul_reason_for_call == DLL_PROCESS_DETACH)
    {
    	if (wpi)
    	{
    		delete wpi;
    		wpi = NULL;
    	}
    }
    return TRUE;
}


void WINAPI SetStartupInfoW(PluginStartupInfo *Info)
{
	memmove(&psi3, Info, sizeof(psi3));
	memmove(&FSF3, Info->FSF, sizeof(FSF3));
	psi3.FSF = &FSF3;
	
	LoadPluginInfo();
	
	if (!LoadPlugin(TRUE))
		return;
	
	// перенести в FSF2 & psi2
	
	// *** FARSTANDARDFUNCTIONS *** 
	FSF2.StructSize = sizeof(FSF2);
	FSF2.atoi = FSF3.atoi;
	FSF2.atoi64 = FSF3.atoi64;
	FSF2.itoa = FSF3.itoa;
	FSF2.itoa64 = FSF3.itoa64;
	// <C&C++>
	FSF2.sprintf = FSF3.sprintf;
	FSF2.sscanf = FSF3.sscanf;
	// </C&C++>
	FSF2.qsort = FSF3.qsort;
	FSF2.bsearch = FSF3.bsearch;
	FSF2.qsortex = FSF3.qsortex;
	// <C&C++>
	FSF2.snprintf = FSF3.snprintf;
	// </C&C++>
	FSF2.LIsLower = FSF3.LIsLower;
	FSF2.LIsUpper = FSF3.LIsUpper;
	FSF2.LIsAlpha = FSF3.LIsAlpha;
	FSF2.LIsAlphanum = FSF3.LIsAlphanum;
	FSF2.LUpper = FSF3.LUpper;
	FSF2.LLower = FSF3.LLower;
	FSF2.LUpperBuf = FSF3.LUpperBuf;
	FSF2.LLowerBuf = FSF3.LLowerBuf;
	FSF2.LStrupr = FSF3.LStrupr;
	FSF2.LStrlwr = FSF3.LStrlwr;
	FSF2.LStricmp = FSF3.LStricmp;
	FSF2.LStrnicmp = FSF3.LStrnicmp;
	FSF2.Unquote = FSF3.Unquote;
	FSF2.LTrim = FSF3.LTrim;
	FSF2.RTrim = FSF3.RTrim;
	FSF2.Trim = FSF3.Trim;
	FSF2.TruncStr = FSF3.TruncStr;
	FSF2.TruncPathStr = FSF3.TruncPathStr;
	FSF2.QuoteSpaceOnly = FSF3.QuoteSpaceOnly;
	FSF2.PointToName = FSF3.PointToName;
	FSF2.GetPathRoot = WrapPluginInfo::FarStdGetPathRoot;
	FSF2.AddEndSlash = FSF3.AddEndSlash;
	FSF2.CopyToClipboard = FSF3.CopyToClipboard;
	FSF2.PasteFromClipboard = FSF3.PasteFromClipboard;
	FSF2.FarKeyToName = FSF3.FarKeyToName;
	FSF2.FarNameToKey = FSF3.FarNameToKey;
	FSF2.FarInputRecordToKey = FSF3.FarInputRecordToKey;
	FSF2.XLat = WrapPluginInfo::FarStdXlat;
	FSF2.GetFileOwner = WrapPluginInfo::FarStdGetFileOwner;
	FSF2.GetNumberOfLinks = FSF3.GetNumberOfLinks;
	FSF2.FarRecursiveSearch = WrapPluginInfo::FarStdRecursiveSearch;
	FSF2.MkTemp = WrapPluginInfo::FarStdMkTemp;
	FSF2.DeleteBuffer = FSF3.DeleteBuffer;
	FSF2.ProcessName = WrapPluginInfo::FarStdProcessName;
	FSF2.MkLink = WrapPluginInfo::FarStdMkLink;
	FSF2.ConvertPath = WrapPluginInfo::FarConvertPath;
	FSF2.GetReparsePointInfo = WrapPluginInfo::FarGetReparsePointInfo;
	FSF2.GetCurrentDirectory = WrapPluginInfo::FarGetCurrentDirectory;
	
	// *** PluginStartupInfo ***
	psi2.StructSize = sizeof(psi2);
	psi2.ModuleName = wpi->PluginDll;
	psi2.ModuleNumber = (INT_PTR)wpi->hDll;
	psi2.RootKey = wpi->RegRoot;
	psi2.Menu = WrapPluginInfo::FarApiMenu;
	psi2.Message = WrapPluginInfo::FarApiMessage;
	psi2.GetMsg = WrapPluginInfo::FarApiGetMsg;
	psi2.Control = WrapPluginInfo::FarApiControl;
	psi2.SaveScreen = WrapPluginInfo::FarApiSaveScreen;
	psi2.RestoreScreen = WrapPluginInfo::FarApiRestoreScreen;
	psi2.GetDirList = WrapPluginInfo::FarApiGetDirList;
	psi2.GetPluginDirList = WrapPluginInfo::FarApiGetPluginDirList;
	psi2.FreeDirList = WrapPluginInfo::FarApiFreeDirList;
	psi2.FreePluginDirList = WrapPluginInfo::FarApiFreePluginDirList;
	psi2.Viewer = WrapPluginInfo::FarApiViewer;
	psi2.Editor = WrapPluginInfo::FarApiEditor;
	psi2.CmpName = WrapPluginInfo::FarApiCmpName;
	psi2.Text = WrapPluginInfo::FarApiText;
	psi2.EditorControl = WrapPluginInfo::FarApiEditorControl;

	psi2.FSF = &FSF2;

	psi2.ShowHelp = WrapPluginInfo::FarApiShowHelp;
	psi2.AdvControl = WrapPluginInfo::FarApiAdvControl;
	psi2.InputBox = WrapPluginInfo::FarApiInputBox;
	psi2.DialogInit = WrapPluginInfo::FarApiDialogInit;
	psi2.DialogRun = WrapPluginInfo::FarApiDialogRun;
	psi2.DialogFree = WrapPluginInfo::FarApiDialogFree;

	psi2.SendDlgMessage = WrapPluginInfo::FarApiSendDlgMessage;
	psi2.DefDlgProc = WrapPluginInfo::FarApiDefDlgProc;
	//DWORD_PTR              Reserved;
	psi2.ViewerControl = WrapPluginInfo::FarApiViewerControl;
	psi2.PluginsControl = WrapPluginInfo::FarApiPluginsControl;
	psi2.FileFilterControl = WrapPluginInfo::FarApiFileFilterControl;
	psi2.RegExpControl = WrapPluginInfo::FarApiRegExpControl;

	lbPsi2 = TRUE;
	
	if (wpi->SetStartupInfoW && lbPsi2)
	{
		wpi->SetStartupInfoW(&psi2);
	}
}

void WINAPI GetGlobalInfoW(GlobalInfo *Info)
{
	LoadPluginInfo();
	
	Info->StructSize = sizeof(GlobalInfo);
	Info->MinFarVersion = FARMANAGERVERSION;

	Info->Version = wpi->Version;
	Info->Guid = wpi->guid_Plugin;
	Info->Title = wpi->Title;
	Info->Description = wpi->Desc;
	Info->Author = wpi->Author;
}



void WINAPI GetPluginInfoW(PluginInfo *Info)
{
    memset(Info, 0, sizeof(PluginInfo));
    
	Info->StructSize = sizeof(*Info);

    if (wpi->GetPluginInfoW)
    {
		wpi->Info.StructSize = sizeof(wpi->Info);
    	wpi->GetPluginInfoW(&wpi->Info);
    	
    	if (wpi->Info.Flags & Far2::PF_PRELOAD)
    		Info->Flags |= PF_PRELOAD;
    	if (wpi->Info.Flags & Far2::PF_DISABLEPANELS)
    		Info->Flags |= PF_DISABLEPANELS;
    	if (wpi->Info.Flags & Far2::PF_EDITOR)
    		Info->Flags |= PF_EDITOR;
    	if (wpi->Info.Flags & Far2::PF_VIEWER)
    		Info->Flags |= PF_VIEWER;
    	if (wpi->Info.Flags & Far2::PF_FULLCMDLINE)
    		Info->Flags |= PF_FULLCMDLINE;
    	if (wpi->Info.Flags & Far2::PF_DIALOG)
    		Info->Flags |= PF_DIALOG;

		//if (GetPrivateProfileString(L"Plugin", L"MenuGUID", L"", szTemp, ARRAYSIZE(szTemp), szIni))
		//{
		//	if (
		//		wpi->guid_PluginMenu = guid;
		//	else
		//		wpi->guid_PluginMenu = ::guid_DefPluginMenu;
		//}

		wchar_t szValName[64], szGUID[128]; GUID guid;
		struct { LPCWSTR Fmt; int Count; const wchar_t * const * Strings;
				 PluginMenuItem *Menu; int* GuidCount; GUID** Guids; }
			Menus[] = 
		{
			{L"PluginMenuGUID%i", wpi->Info.PluginMenuStringsNumber, wpi->Info.PluginMenuStrings, 
				&Info->PluginMenu, &wpi->nPluginMenu, &wpi->guids_PluginMenu},
			{L"DiskMenuGUID%i", wpi->Info.DiskMenuStringsNumber, wpi->Info.DiskMenuStrings, 
				&Info->DiskMenu, &wpi->nPluginDisks, &wpi->guids_PluginDisks},
			{L"PluginConfigGUID%i", wpi->Info.PluginConfigStringsNumber, wpi->Info.PluginConfigStrings, 
				&Info->PluginConfig, &wpi->nPluginConfig, &wpi->guids_PluginConfig}
		};
		for (int k = 0; k < ARRAYSIZE(Menus); k++)
		{
			if (Menus[k].Count < 1)
				continue;
			
    		if (*Menus[k].GuidCount < Menus[k].Count || !*Menus[k].Guids)
    		{
    			if (*Menus[k].Guids)
    				free(*Menus[k].Guids);
    			*Menus[k].GuidCount = 0;
    			*Menus[k].Guids = (GUID*)calloc(sizeof(GUID),Menus[k].Count);
    			for (int i = 1; i <= Menus[k].Count; i++)
    			{
    				wsprintf(szValName, Menus[k].Fmt, i);
    				if (!GetPrivateProfileString(L"Plugin", szValName, L"", szGUID, ARRAYSIZE(szGUID), wpi->IniFile))
    					break;
    				if (UuidFromStringW((RPC_WSTR)szGUID, &guid) != RPC_S_OK)
    					break;
    				// OK
    				*(Menus[k].Guids[i-1]) = guid;
    				(*Menus[k].GuidCount)++;
    			}
    			Menus[k].Count = *Menus[k].GuidCount; // могло не на всех гуидов хватить
    		}
    		if (Menus[k].Count > 0)
    		{
				Menus[k].Menu->Guids = *Menus[k].Guids;
				Menus[k].Menu->Strings = Menus[k].Strings;
				Menus[k].Menu->Count = Menus[k].Count;
			}
    	}
		//const wchar_t * const *DiskMenuStrings;
		//int *DiskMenuNumbers;
		//int DiskMenuStringsNumber;
		// -- >struct PluginMenuItem DiskMenu;
		//const wchar_t * const *PluginMenuStrings;
		//int PluginMenuStringsNumber;
		// -->struct PluginMenuItem PluginMenu;
		//const wchar_t * const *PluginConfigStrings;
		//int PluginConfigStringsNumber;
		// -->struct PluginMenuItem PluginConfig;
		
		Info->CommandPrefix = wpi->Info.CommandPrefix;
    }
}

HANDLE WINAPI OpenW(const OpenInfo *Info)
{
	HANDLE h = INVALID_HANDLE_VALUE;

	if ((Info->OpenFrom & OPEN_FROM_MASK) == OPEN_ANALYSE)
	{
		if (wpi->AnalyseW)
		{
			// В принципе, в Far2 была такая функция, так что если плаг ее экспортит - зовем
			h = wpi->OpenPluginW(OpenFrom_3_2(Info->OpenFrom), Info->Data);
			goto trap;
		}
		if (!wpi->pAnalyze || !wpi->OpenFilePluginW)
			goto trap;
		h = wpi->OpenFilePluginW(wpi->pAnalyze->FileName,
				(const unsigned char*)wpi->pAnalyze->Buffer,
				wpi->pAnalyze->BufferSize,
				OpMode_3_2(wpi->pAnalyze->OpMode));
		goto trap;
	}
	else if (wpi->pAnalyze)
	{
		free(wpi->pAnalyze);
		wpi->pAnalyze = NULL;
	}
	
	if ((Info->OpenFrom & OPEN_FROMMACRO_MASK))
	{
		if (wpi->Info.Reserved 
			&& (((Info->OpenFrom & OPEN_FROMMACRO_MASK) == OPEN_FROMMACRO)
				|| ((Info->OpenFrom & OPEN_FROMMACRO_MASK) == OPEN_FROMMACROSTRING))
		    && wpi->OpenPluginW)
		{
			h = wpi->OpenPluginW(Far2::OPEN_FROMMACRO, Info->Data);
		}
		goto trap;
	}

	if (wpi->OpenPluginW)
		h = wpi->OpenPluginW(Far2::OPEN_FROMMACRO, Info->Data);
trap:
	return h;
}

int    WINAPI AnalyseW(const AnalyseInfo *Info)
{
	if (!wpi)
		return FALSE;
	if (wpi->AnalyseW)
		return wpi->AnalyseW(Info);
	if (wpi->pAnalyze)
	{
		free(wpi->pAnalyze);
		wpi->pAnalyze = NULL;
	}
	if (!wpi->OpenFilePluginW)
		return FALSE;
	size_t nNewSize = sizeof(*wpi->pAnalyze) + Info->BufferSize
			+ ((Info->FileName ? lstrlen(Info->FileName) : 0)+1)*sizeof(wchar_t);
	wpi->pAnalyze = (AnalyseInfo*)malloc(nNewSize);
	if (!wpi->pAnalyze)
		return FALSE;
	LPBYTE ptr = (LPBYTE)wpi->pAnalyze;
	memmove(ptr, Info, sizeof(wpi->pAnalyze));
	ptr += sizeof(wpi->pAnalyze);
	if (Info->BufferSize && Info->Buffer)
	{
		memmove(ptr, Info->Buffer, Info->BufferSize);
		wpi->pAnalyze->Buffer = ptr;
		ptr += Info->BufferSize;
	}
	else
	{
		wpi->pAnalyze->Buffer = NULL;
	}
	if (Info->FileName)
	{
		lstrcpy((LPWSTR)ptr, Info->FileName);
		wpi->pAnalyze->FileName = (LPCWSTR)ptr;
	}
	else
		wpi->pAnalyze->FileName = NULL;
	
	HANDLE h = wpi->OpenFilePluginW(wpi->pAnalyze->FileName,
		(const unsigned char*)wpi->pAnalyze->Buffer,
		wpi->pAnalyze->BufferSize,
		OpMode_3_2(Info->OpMode));
	if (h && h != INVALID_HANDLE_VALUE && ((INT_PTR)h) != -2)
	{
		if (wpi->AnalyzeMode == 1)
		{
			//TODO: Не закрывать. только вопрос - когда его можно будет закрыть, если OpenW НЕ будет вызван?
		}
	
		//TODO: Хорошо бы оптимизнуть, и не открывать файл два раза подряд, но непонятно
		//TODO: в какой момент можно закрыть Far2 плагин, если до OpenW дело не дошло...
		if (wpi->ClosePluginW)
			wpi->ClosePluginW(h);
		return TRUE;
	}
	return 0;
}
void   WINAPI ClosePanelW(HANDLE hPanel)
{
	if (wpi && wpi->ClosePluginW)
		wpi->ClosePluginW(hPanel);
}
int    WINAPI CompareW(const CompareInfo *Info)
{
	int iRc = -2;
	if (wpi->CompareW)
	{
		Far2::PluginPanelItem Item1 = {{0}};
		Far2::PluginPanelItem Item2 = {{0}};
		PluginPanelItem_3_2(Info->Item1, &Item1);
		PluginPanelItem_3_2(Info->Item2, &Item2);
		iRc = wpi->CompareW(Info->hPanel, &Item1, &Item2, SortMode_3_2(Info->Mode));
	}
	return iRc;
}
int    WINAPI ConfigureW(const GUID* Guid)
{
	int iRc = 0;
	if (wpi && wpi->ConfigureW)
	{
		if (wpi->nPluginConfig > 0 && wpi->guids_PluginConfig)
		{
			for (int i = 0; i < wpi->nPluginConfig; i++)
			{
				if (memcmp(Guid, wpi->guids_PluginConfig+i, sizeof(GUID)) == 0)
				{
					iRc = wpi->ConfigureW(i);
					break;
				}
			}
		}
	}
	return iRc;
}
int    WINAPI DeleteFilesW(const DeleteFilesInfo *Info)
{
	int iRc = 0;
	if (wpi->DeleteFilesW)
	{
		Far2::PluginPanelItem *p2 = PluginPanelItems_3_2(Info->PanelItem, Info->ItemsNumber);
		if (p2)
		{
			iRc = wpi->DeleteFilesW(Info->hPanel, p2, Info->ItemsNumber, OpMode_3_2(Info->OpMode));
			free(p2);
		}
	}
	return iRc;
}
void   WINAPI ExitFARW(void)
{
	if (wpi && wpi->ExitFARW)
		wpi->ExitFARW();
}
void   WINAPI FreeVirtualFindDataW(const FreeFindDataInfo *Info)
{
	//TODO:
}
int    WINAPI GetFilesW(GetFilesInfo *Info)
{
	int iRc = 0;
	if (wpi->GetFilesW)
	{
		Far2::PluginPanelItem *p2 = PluginPanelItems_3_2(Info->PanelItem, Info->ItemsNumber);
		if (p2)
		{
			iRc = wpi->GetFilesW(Info->hPanel, p2, Info->ItemsNumber, Info->Move, &Info->DestPath, OpMode_3_2(Info->OpMode));
			free(p2);
		}
	}
	return iRc;
}
int    WINAPI GetFindDataW(GetFindDataInfo *Info)
{
	//GetFindDataW(HANDLE hPlugin,Far2::PluginPanelItem **pPanelItem,int *pItemsNumber,int OpMode);
	if (!wpi->GetFindDataW)
		return 0;
	Far2::PluginPanelItem *pPanelItem = NULL;
	int ItemsNumber = 0;
	int iRc = wpi->GetFindDataW(Info->hPanel, &pPanelItem, &ItemsNumber, OpMode_3_2(Info->OpMode));
	if (iRc && ItemsNumber > 0)
	{
		Info->PanelItem = PluginPanelItems_2_3(pPanelItem, ItemsNumber);
		Info->ItemsNumber = ItemsNumber;
		wpi->MapPanelItems[Info->PanelItem] = pPanelItem;
		//PluginPanelItem* p3 = Info->PanelItem;
		//Far2::PluginPanelItem* p2 = pPanelItem;
		//for (int i = 0; i < ItemsNumber; i++, p2++, p3++)
		//{
		//	p3->FileAttributes = p2->FindData.dwFileAttributes;
		//	p3->CreationTime = p2->FindData.ftCreationTime;
		//	p3->LastAccessTime = p2->FindData.ftLastAccessTime;
		//	p3->LastWriteTime = p2->FindData.ftLastWriteTime;
		//	p3->ChangeTime = p2->FindData.ftLastWriteTime;
		//	p3->FileSize = p2->FindData.nFileSize;
		//	p3->PackSize = p2->FindData.nPackSize;
		//	p3->FileName = p2->FindData.lpwszFileName;
		//	p3->AlternateFileName = p2->FindData.lpwszAlternateFileName;
		//	p3->Flags = PluginPanelItemFlags_3_2(p2->Flags);
		//	p3->NumberOfLinks = p2->NumberOfLinks;
		//	p3->Description = p2->Description;
		//	p3->Owner = p2->Owner;
		//	p3->CustomColumnData = p2->CustomColumnData;
		//	p3->CustomColumnNumber = p2->CustomColumnNumber;
		//	p3->UserData = p2->UserData;
		//	p3->CRC32 = p2->CRC32;
		//}
	}
	return iRc;
}
void   WINAPI FreeFindDataW(const FreeFindDataInfo *Info)
{
	if (Info->PanelItem)
	{
		if (wpi->FreeFindDataW)
		{
			Far2::PluginPanelItem *pPanelItem = wpi->MapPanelItems[Info->PanelItem];
			if (pPanelItem)
				wpi->FreeFindDataW(Info->hPanel, pPanelItem, Info->ItemsNumber);
		}
		free(Info->PanelItem);
		
		wpi->MapPanelItems.erase(Info->PanelItem);
		//std::map<PluginPanelItem*,Far2::PluginPanelItem*>::iterator iter;
		//for (iter = wpi->MapPanelItems.begin( ); iter!=wpi->MapPanelItems.end( ); iter++)
		//{
		//	if (iter->first == Info->PanelItem)
		//	{
		//		wpi->MapPanelItems.erase(iter);
		//		break;
		//	}
		//}
	}
}
void   WINAPI GetOpenPanelInfoW(OpenPanelInfo *Info)
{
	if (wpi->GetOpenPluginInfoW)
	{
		Far2::OpenPluginInfo ofi = {sizeof(Far2::OpenPluginInfo)};
		wpi->GetOpenPluginInfoW(Info->hPanel, &ofi);
		
		//memset(Info, 0, sizeof(*Info));
		//Info->StructSize = sizeof(*Info);
		//HANDLE hPanel;
		Info->Flags = OpenPanelInfoFlags_2_3(ofi.Flags);
		Info->HostFile = ofi.HostFile;
		Info->CurDir = ofi.CurDir;
		Info->Format = ofi.Format;
		Info->PanelTitle = ofi.PanelTitle;
		Info->InfoLines = InfoLines_2_3(ofi.InfoLines, ofi.InfoLinesNumber);
		Info->InfoLinesNumber = wpi->InfoLinesNumber;
		Info->DescrFiles = ofi.DescrFiles;
		Info->DescrFilesNumber = ofi.DescrFilesNumber;
		Info->PanelModesArray = PanelModes_2_3(ofi.PanelModesArray, ofi.PanelModesNumber);
		Info->PanelModesNumber = wpi->PanelModesNumber;
		Info->StartPanelMode = ofi.StartPanelMode;
		Info->StartSortMode = SortMode_2_3(ofi.StartSortMode);
		Info->StartSortOrder = ofi.StartSortOrder;
		Info->KeyBar = KeyBarTitles_2_3(ofi.KeyBar);
		Info->ShortcutData = ofi.ShortcutData;
		Info->FreeSize = 0;
	}
}
int    WINAPI GetVirtualFindDataW(GetVirtualFindDataInfo *Info)
{
	//TODO:
	return 0;
}
int    WINAPI MakeDirectoryW(MakeDirectoryInfo *Info)
{
	int iRc = 0;
	if (wpi->MakeDirectoryW)
		iRc = wpi->MakeDirectoryW(Info->hPanel, &Info->Name, OpMode_3_2(Info->OpMode));
	return iRc;
}
int    WINAPI ProcessDialogEventW(int Event,void *Param)
{
	//TODO:
	return 0;
}
int    WINAPI ProcessEditorEventW(int Event,void *Param)
{
	int iRc = 0;
	if (wpi->ProcessEditorEventW)
		iRc = wpi->ProcessEditorEventW(Event, Param);
	return iRc;
}
int    WINAPI ProcessEditorInputW(const INPUT_RECORD *Rec)
{
	//TODO:
	return 0;
}
int    WINAPI ProcessEventW(HANDLE hPanel,int Event,void *Param)
{
	int iRc = 0;
	if (wpi->ProcessEventW)
		iRc = wpi->ProcessEventW(hPanel, Event, Param);
	return iRc;
}
int    WINAPI ProcessHostFileW(const ProcessHostFileInfo *Info)
{
	int iRc = 0;
	if (wpi->ProcessHostFileW)
	{
		Far2::PluginPanelItem *p2 = PluginPanelItems_3_2(Info->PanelItem, Info->ItemsNumber);
		iRc = wpi->ProcessHostFileW(Info->hPanel, p2, Info->ItemsNumber, OpMode_3_2(Info->OpMode));
		if (p2)
			free(p2);
	}
	return iRc;
}
int    WINAPI ProcessKeyW(HANDLE hPanel,const INPUT_RECORD *Rec)
{
	//TODO:
	return 0;
}
int    WINAPI ProcessSynchroEventW(int Event,void *Param)
{
	int iRc = 0;
	if (wpi->ProcessSynchroEventW)
		iRc = wpi->ProcessSynchroEventW(Event, Param);
	return iRc;
}
int    WINAPI ProcessViewerEventW(int Event,void *Param)
{
	int iRc = 0;
	if (wpi->ProcessViewerEventW)
		iRc = wpi->ProcessViewerEventW(Event, Param);
	return iRc;
}
int    WINAPI PutFilesW(const PutFilesInfo *Info)
{
	int iRc = 0;
	if (wpi->PutFilesW)
	{
		Far2::PluginPanelItem *p2 = PluginPanelItems_3_2(Info->PanelItem, Info->ItemsNumber);
		if (p2)
		{
			iRc = wpi->PutFilesW(Info->hPanel, p2, Info->ItemsNumber, Info->Move, Info->SrcPath, OpMode_3_2(Info->OpMode));
			free(p2);
		}
	}
	return iRc;
}
int    WINAPI SetDirectoryW(const SetDirectoryInfo *Info)
{
	int iRc = 0;
	if (wpi->SetDirectoryW)
		iRc = wpi->SetDirectoryW(Info->hPanel, Info->Dir, OpMode_3_2(Info->OpMode));
	return iRc;
}
int    WINAPI SetFindListW(const SetFindListInfo *Info)
{
	//TODO:
	return 0;
}

//int EditCtrl(int Cmd, void* Parm)
//{
//	int iRc;
//	iRc = psi.EditorControl(-1, (EDITOR_CONTROL_COMMANDS)Cmd, 0, (INT_PTR)Parm);
//	return iRc;
//}




Far2Dialog::Far2Dialog(int X1, int Y1, int X2, int Y2,
    const wchar_t *HelpTopic, Far2::FarDialogItem *Items, UINT ItemsNumber,
    DWORD Flags, Far2::FARWINDOWPROC DlgProc, LONG_PTR Param,
    GUID PluginGuid, GUID DefGuid)
{
	hDlg3 = NULL; m_Items3 = NULL;
	m_PluginGuid = PluginGuid;
	
    m_X1 = X1; m_Y1 = Y1; m_X2 = X2; m_Y2 = Y2;
    m_HelpTopic = HelpTopic ? _wcsdup(HelpTopic) : NULL;
    
    m_ItemsNumber = ItemsNumber;
    m_Items2 = NULL;
    if (ItemsNumber > 0 && Items)
    {
    	m_Items2 = (Far2::FarDialogItem*)calloc(ItemsNumber, sizeof(*m_Items2));
    	memmove(m_Items2, Items, ItemsNumber*sizeof(*m_Items2));
    }
    m_Flags = Flags;
    m_DlgProc = DlgProc;
    m_Param = Param;
    // GUID
    m_GuidChecked = FALSE;
    memset(&m_Guid, 0, sizeof(m_Guid));
    m_DefGuid = DefGuid;
};

Far2Dialog::~Far2Dialog()
{
	// Far3 call
	FreeDlg();
	// Release memory
	if (m_HelpTopic)
	{
		free(m_HelpTopic);
		m_HelpTopic = NULL;
	}
	if (m_Items2)
	{
		free(m_Items2);
		m_Items2 = NULL;
	}
	if (m_Items3)
	{
		free(m_Items3);
		m_Items3 = NULL;
	}
};

void Far2Dialog::FreeDlg()
{
	if (hDlg3 != NULL)
	{
		psi3.DialogFree(hDlg3);
		hDlg3 = NULL;
	}
}

INT_PTR Far2Dialog::Far3DlgProc(HANDLE hDlg, int Msg, int Param1, INT_PTR Param2)
{
	Far2Dialog* p = wpi->MapDlg_3_2[hDlg];
	INT_PTR lRc = 0;
	if (p && p->m_DlgProc)
	{
		int Msg2 = FarMessage_3_2(Msg, Param2);
		lRc = p->m_DlgProc((HANDLE)p, Msg2, Param1, Param2);
	}
	else
	{
		lRc = psi3.DefDlgProc(hDlg, Msg, Param1, Param2);
	}
	return lRc;
}

int Far2Dialog::RunDlg()
{
	int iRc = -1;
	
	if (!m_GuidChecked)
	{
		/*
		-- пока не получается. Например, UCharMap зовет DM_GETDLGDATA, а диалога-то еще нет...
		if (m_DlgProc)
		{
			Far2::DialogInfo info = {sizeof(Far2::DialogInfo)};
			if (m_DlgProc((HANDLE)this, Far2::DM_GETDIALOGINFO, 0, (LONG_PTR)&info)
				&& memcmp(&info.Id, &m_Guid, sizeof(m_Guid)))
			{
				m_Guid = info.Id;
				m_GuidChecked = TRUE;
			}
		}
		*/
		if (!m_GuidChecked)
		{
			m_Guid = m_DefGuid;
			m_GuidChecked = TRUE;
		}
	}
	
	if (!m_Items3 && m_Items2 && m_ItemsNumber > 0)
	{
		m_Items3 = (FarDialogItem*)calloc(m_ItemsNumber, sizeof(*m_Items3));
		Far2::FarDialogItem *p2 = m_Items2;
		FarDialogItem *p3 = m_Items3;
		for (UINT i = 0; i < m_ItemsNumber; i++, p2++, p3++)
		{
			p3->Type = DialogItemTypes_2_3(p2->Type);
			p3->X1 = p2->X1;
			p3->Y1 = p2->Y1;
			p3->X2 = p2->X2;
			p3->Y2 = p2->Y2;

			p3->Flags = FarDialogItemFlags_2_3(p2->Flags);
			if (p2->DefaultButton)
				p3->Flags |= DIF_DEFAULTBUTTON;
			if (p2->Focus)
				p3->Flags |= DIF_FOCUS;

			p3->Data = p2->PtrData;
			p3->MaxLength = p2->MaxLen;

			if (p3->Type == DI_EDIT)
			{
				p3->History = p2->Param.History;
			}
			else if (p3->Type == DI_FIXEDIT)
			{
				p3->Mask = p2->Param.Mask;
			}
			else if (p3->Type == DI_COMBOBOX || p3->Type == DI_LISTBOX)
			{
				//TODO:
				_ASSERTE(p2->Param.ListItems == NULL);
				//p3->ListItems = p2->Param.ListItems;
			}
			else if (p3->Type == DI_USERCONTROL)
			{
				p3->VBuf = p2->Param.VBuf;
			}
			else
			{
				p3->Reserved = p2->Param.Reserved;
			}
		}
	}

	if (hDlg3 == NULL)
	{
		wpi->LastFar2Dlg = this;
		FARDIALOGFLAGS Flags3 = 0
			| ((m_Flags & Far2::FDLG_WARNING) ? FDLG_WARNING : 0)
			| ((m_Flags & Far2::FDLG_SMALLDIALOG) ? FDLG_SMALLDIALOG : 0)
			| ((m_Flags & Far2::FDLG_NODRAWSHADOW) ? FDLG_NODRAWSHADOW : 0)
			| ((m_Flags & Far2::FDLG_NODRAWPANEL) ? FDLG_NODRAWPANEL : 0);
		hDlg3 = psi3.DialogInit(&m_PluginGuid, &m_Guid,
			m_X1, m_Y1, m_X2, m_Y2, m_HelpTopic, m_Items3, m_ItemsNumber, 0,
			Flags3, Far3DlgProc, m_Param);
		if (hDlg3)
		{
			wpi->MapDlg_2_3[this] = hDlg3;
			wpi->MapDlg_3_2[hDlg3] = this;
		}
	}
	
	if (hDlg3 != NULL)
	{
		wpi->LastFar2Dlg = this;
		iRc = psi3.DialogRun(hDlg3);

		wpi->MapDlg_2_3.erase(this);
		//for (std::map<Far2Dialog*,HANDLE>::iterator i1 = wpi->MapDlg_2_3.begin();
		//	 i1 != wpi->MapDlg_2_3.end(); i++)
		//{
		//	if (i->first == this)
		//	{
		//		wpi->MapDlg_2_3.erase(i1);
		//		break;
		//	}
		//}
		wpi->MapDlg_3_2.erase(hDlg3);
		//for (std::map<HANDLE,Far2Dialog*>::iterator i2 = wpi->MapDlg_3_2.begin();
		//	 i2 != wpi->MapDlg_3_2.end(); i++)
		//{
		//	if (i->first == hDlg3)
		//	{
		//		wpi->MapDlg_3_2.erase(i2);
		//		break;
		//	}
		//}
	}
	
	wpi->LastFar2Dlg = NULL;
	return iRc;
}

