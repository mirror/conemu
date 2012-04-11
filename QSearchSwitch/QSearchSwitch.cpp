
// callplugin("91cf1dc3-9837-403d-8ac3-58c570a80ea4",123)

#include "plugin.hpp"
#include <TCHAR.H>

struct PluginStartupInfo psi;
struct FarStandardFunctions FSFW;
HMODULE ghInstance;

GUID guid_QSearchSwitch = { /* 9d5d070e-42f3-4a10-82e3-b3c74830705b */
    0x9d5d070e,
    0x42f3,
    0x4a10,
    {0x82, 0xe3, 0xb3, 0xc7, 0x48, 0x30, 0x70, 0x5b}
  };
  
BOOL APIENTRY DllMain( HANDLE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved )
{
	switch (ul_reason_for_call) {
		case DLL_PROCESS_ATTACH:
			ghInstance = (HMODULE)hModule;
			break;
	}
	return TRUE;
}

int WINAPI GetMinFarVersionW(void)
{
	#define MAKEFARVERSION2(major,minor,build) ( ((major)<<8) | (minor) | ((build)<<16))
	return MAKEFARVERSION2(FARMANAGERVERSION_MAJOR,FARMANAGERVERSION_MINOR,FARMANAGERVERSION_BUILD);
}

void WINAPI GetGlobalInfoW(struct GlobalInfo *Info)
{
	Info->StructSize = sizeof(GlobalInfo);
	Info->MinFarVersion = FARMANAGERVERSION;

	// Build: YYMMDDX (YY - две цифры года, MM - мес€ц, DD - день, X - 0 и выше-номер подсборки)
	Info->Version = MAKEFARVERSION(1,0,2,0, VS_RELEASE);
	
	Info->Guid = guid_QSearchSwitch;
	Info->Title = L"QSearch switch";
	Info->Description = Info->Title;
	Info->Author = L"ConEmu.Maximus5@gmail.com";
}


void WINAPI GetPluginInfoW(struct PluginInfo *pi)
{
	pi->Flags = PF_PRELOAD;
}

void WINAPI SetStartupInfoW(const PluginStartupInfo *aInfo)
{
	::psi = *aInfo;
	::FSFW = *aInfo->FSF;
	::psi.FSF = &::FSFW;
}

BOOL gbAltPressed = FALSE;
BOOL gbAltModified = FALSE;

int WINAPI ProcessConsoleInputW(struct ProcessConsoleInputInfo *Info)
{
	if (Info->Rec->EventType != KEY_EVENT)
	{
		return FALSE; // continue in Far
	}

	if (Info->Rec->Event.KeyEvent.wVirtualKeyCode == VK_MENU)
	{
		//BUGBUG: Both Alt's pressed?
		gbAltPressed = Info->Rec->Event.KeyEvent.bKeyDown;
		if (gbAltPressed)
			gbAltModified = FALSE;
	}

	if (Info->Rec->Event.KeyEvent.uChar.UnicodeChar < L' ')
	{
		return FALSE; // continue in Far
	}

	INT_PTR nArea = psi.MacroControl(&guid_QSearchSwitch, MCTL_GETAREA, 0, 0);
	// ѕерехват только в панел€х
	if (nArea != MACROAREA_SHELL)
		return FALSE; // continue in Far

	INT_PTR nCmdLen = psi.PanelControl(PANEL_ACTIVE, FCTL_GETCMDLINE, 0, NULL);
	//if (nCmdLen >= 2)
	//	return FALSE; // continue in Far
	
	DWORD nMods = (Info->Rec->Event.KeyEvent.dwControlKeyState & (SHIFT_PRESSED|LEFT_CTRL_PRESSED|RIGHT_CTRL_PRESSED|LEFT_ALT_PRESSED|RIGHT_ALT_PRESSED));
	INPUT_RECORD r = *Info->Rec;
	DWORD nCount = 1;
	BOOL bWritten = FALSE;

	switch (nMods)
	{
	case 0:
	case SHIFT_PRESSED:
		if (gbAltPressed)
			return FALSE; // continue in Far
		if (gbAltModified && (nCmdLen >= 2))
			return FALSE; // continue in Far
		r.Event.KeyEvent.dwControlKeyState |= LEFT_ALT_PRESSED;
		bWritten = WriteConsoleInput(GetStdHandle(STD_INPUT_HANDLE), &r, nCount, &nCount);
		gbAltModified = FALSE;
		return bWritten; // skip in Far on success
	case LEFT_ALT_PRESSED:
	case RIGHT_ALT_PRESSED:
	case (LEFT_ALT_PRESSED|SHIFT_PRESSED):
	case (RIGHT_ALT_PRESSED|SHIFT_PRESSED):
		if (!gbAltPressed)
			return FALSE; // continue in Far
		r.Event.KeyEvent.dwControlKeyState &= ~(LEFT_ALT_PRESSED|RIGHT_ALT_PRESSED);
		bWritten = WriteConsoleInput(GetStdHandle(STD_INPUT_HANDLE), &r, nCount, &nCount);
		gbAltModified = bWritten;
		return bWritten; // skip in Far on success
	}

	return FALSE; // continue in Far
}
