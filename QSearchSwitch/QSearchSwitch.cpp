
// callplugin("91cf1dc3-9837-403d-8ac3-58c570a80ea4",123)

#include "version.h"
#include <TCHAR.H>

struct PluginStartupInfo psi;
struct FarStandardFunctions FSFW;
#ifdef _DEBUG
HMODULE ghInstance;
#endif

GUID guid_QSearchSwitch = { /* 9d5d070e-42f3-4a10-82e3-b3c74830705b */
    0x9d5d070e,
    0x42f3,
    0x4a10,
    {0x82, 0xe3, 0xb3, 0xc7, 0x48, 0x30, 0x70, 0x5b}
  };
  
#ifdef _DEBUG
BOOL APIENTRY DllMain( HANDLE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved )
{
	switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
			ghInstance = (HMODULE)hModule;
			break;
	}
	return TRUE;
}
#endif

int WINAPI GetMinFarVersionW(void)
{
	#define MAKEFARVERSION2(major,minor,build) ( ((major)<<8) | (minor) | ((build)<<16))
	return MAKEFARVERSION2(FARMANAGERVERSION_MAJOR,FARMANAGERVERSION_MINOR,FARMANAGERVERSION_BUILD);
}

void WINAPI GetGlobalInfoW(struct GlobalInfo *Info)
{
	Info->StructSize = sizeof(GlobalInfo);
	Info->MinFarVersion = FARMANAGERVERSION;

	// Build: YYMMDDX (YY - две цифры года, MM - месяц, DD - день, X - 0 и выше-номер подсборки)
	Info->Version = MAKEFARVERSION(MVV_1,MVV_2,MVV_3,MVV_4, VS_RELEASE);
	
	Info->Guid = guid_QSearchSwitch;
	Info->Title = L"QSearch switch";
	Info->Description = Info->Title;
	Info->Author = L"ConEmu.Maximus5@gmail.com";
}


BOOL gbEnabled = TRUE;

void WINAPI SetStartupInfoW(const PluginStartupInfo *aInfo)
{
	::psi = *aInfo;
	::FSFW = *aInfo->FSF;
	::psi.FSF = &::FSFW;

	FarSettingsCreate sc = {sizeof(FarSettingsCreate), guid_QSearchSwitch, INVALID_HANDLE_VALUE};
	if (psi.SettingsControl(INVALID_HANDLE_VALUE, SCTL_CREATE, 0, &sc) != 0)
	{
		FarSettingsItem fsi = {0};
		fsi.Name = L"Enabled";
		fsi.Type = FST_QWORD;
		if (psi.SettingsControl(sc.Handle, SCTL_GET, 0, &fsi) && (fsi.Type == FST_QWORD))
			gbEnabled = (fsi.Number != 0);
		psi.SettingsControl(sc.Handle, SCTL_FREE, 0, 0);
	}
}

void WINAPI GetPluginInfoW(struct PluginInfo *pi)
{
	pi->Flags = gbEnabled ? PF_PRELOAD : 0;
}

HANDLE WINAPI OpenW(const struct OpenInfo *Info)
{
	if ((Info->OpenFrom & OPEN_FROM_MASK) == OPEN_FROMMACRO)
	{
		OpenMacroInfo* p = (OpenMacroInfo*)Info->Data;
		enum QCommand { eNone = 0, eSwitch = 1, eEnable = 2, eDisable = 4, ePermanent = 0x10 };
		DWORD nCmd = eNone;
		if (p && (p->Count > 0) && p->Values && (p->Values[0].Type == FMVT_INTEGER))
		{
			if (p->Values[0].Integer & eSwitch)
				nCmd |= eSwitch;
			else if (p->Values[0].Integer & eEnable)
				nCmd |= eEnable;
			else if (p->Values[0].Integer & eEnable)
				nCmd |= eDisable;

			if (p->Values[0].Integer & ePermanent)
				nCmd |= ePermanent;
		}

		if (nCmd != eNone)
		{
			if (nCmd & eSwitch)
				gbEnabled = !gbEnabled;
			else if (nCmd & eEnable)
				gbEnabled = TRUE;
			else if (nCmd & eDisable)
				gbEnabled = FALSE;

			if (nCmd & ePermanent)
			{
				FarSettingsCreate sc = {sizeof(FarSettingsCreate), guid_QSearchSwitch, INVALID_HANDLE_VALUE};
				if (psi.SettingsControl(INVALID_HANDLE_VALUE, SCTL_CREATE, 0, &sc) != 0)
				{
					FarSettingsItem fsi = {0};
					fsi.Name = L"Enabled";
					fsi.Type = FST_QWORD;
					fsi.Number = gbEnabled ? 1 : 0;
					psi.SettingsControl(sc.Handle, SCTL_SET, 0, &fsi);
					psi.SettingsControl(sc.Handle, SCTL_FREE, 0, 0);
				}
			}

			return (HANDLE)TRUE;
		}
	}

	return NULL;
}

BOOL gbAltPressed = FALSE;
BOOL gbAltModified = FALSE;

int WINAPI ProcessConsoleInputW(struct ProcessConsoleInputInfo *Info)
{
	if (!gbEnabled || (Info->Rec.EventType != KEY_EVENT))
	{
		return FALSE; // continue in Far
	}

	if (Info->Rec.Event.KeyEvent.wVirtualKeyCode == VK_MENU)
	{
		//BUGBUG: Both Alt's pressed?
		gbAltPressed = Info->Rec.Event.KeyEvent.bKeyDown;
		if (gbAltPressed)
			gbAltModified = FALSE;
	}

	if (Info->Rec.Event.KeyEvent.uChar.UnicodeChar < L' ')
	{
		return FALSE; // continue in Far
	}

	INT_PTR nArea = psi.MacroControl(&guid_QSearchSwitch, MCTL_GETAREA, 0, 0);
	// Перехват только в панелях
	if (nArea != MACROAREA_SHELL && nArea != MACROAREA_SHELLAUTOCOMPLETION)
		return FALSE; // continue in Far

	INT_PTR nCmdLen = psi.PanelControl(PANEL_ACTIVE, FCTL_GETCMDLINE, 0, NULL);
	//if (nCmdLen >= 2)
	//	return FALSE; // continue in Far
	
	DWORD nMods = (Info->Rec.Event.KeyEvent.dwControlKeyState & (SHIFT_PRESSED|LEFT_CTRL_PRESSED|RIGHT_CTRL_PRESSED|LEFT_ALT_PRESSED|RIGHT_ALT_PRESSED));
	//INPUT_RECORD r = *Info->Rec;
	//DWORD nCount = 1;
	//BOOL bWritten = FALSE;

	switch (nMods)
	{
	case 0:
	case SHIFT_PRESSED:
		if (gbAltPressed)
			return FALSE; // continue in Far
		if (gbAltModified && (nCmdLen >= 2))
			return FALSE; // continue in Far
		Info->Rec.Event.KeyEvent.dwControlKeyState |= LEFT_ALT_PRESSED;
		//bWritten = WriteConsoleInput(GetStdHandle(STD_INPUT_HANDLE), &r, nCount, &nCount);
		gbAltModified = FALSE;
		//return bWritten; // skip in Far on success
		return 2; // modified!
	case LEFT_ALT_PRESSED:
	case RIGHT_ALT_PRESSED:
	case (LEFT_ALT_PRESSED|SHIFT_PRESSED):
	case (RIGHT_ALT_PRESSED|SHIFT_PRESSED):
		// Если это нажатие пришло от нас - пропускаем в фар
		if (!gbAltPressed)
			return FALSE; // continue in Far
		Info->Rec.Event.KeyEvent.dwControlKeyState &= ~(LEFT_ALT_PRESSED|RIGHT_ALT_PRESSED);
		//bWritten = WriteConsoleInput(GetStdHandle(STD_INPUT_HANDLE), &r, nCount, &nCount);
		gbAltModified = TRUE;
		//return bWritten; // skip in Far on success
		return 2; // modified!
	}

	return FALSE; // continue in Far
}
