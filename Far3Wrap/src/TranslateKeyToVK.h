
#pragma once

int TranslateKeyToVK(int Key,int &VirtKey,int &ControlState,INPUT_RECORD *Rec)
{
	/* ----------------------------------------------------------------- */
	static struct TTable_KeyToVK
	{
		int Key;
		int VK;
	} Table_KeyToVK[]=
	{
		//   {KEY_PGUP,          VK_PRIOR},
		//   {KEY_PGDN,          VK_NEXT},
		//   {KEY_END,           VK_END},
		//   {KEY_HOME,          VK_HOME},
		//   {KEY_LEFT,          VK_LEFT},
		//   {KEY_UP,            VK_UP},
		//   {KEY_RIGHT,         VK_RIGHT},
		//   {KEY_DOWN,          VK_DOWN},
		//   {KEY_INS,           VK_INSERT},
		//   {KEY_DEL,           VK_DELETE},
		//   {KEY_LWIN,          VK_LWIN},
		//   {KEY_RWIN,          VK_RWIN},
		//   {KEY_APPS,          VK_APPS},
		//   {KEY_MULTIPLY,      VK_MULTIPLY},
		//   {KEY_ADD,           VK_ADD},
		//   {KEY_SUBTRACT,      VK_SUBTRACT},
		//   {KEY_DIVIDE,        VK_DIVIDE},
		//   {KEY_F1,            VK_F1},
		//   {KEY_F2,            VK_F2},
		//   {KEY_F3,            VK_F3},
		//   {KEY_F4,            VK_F4},
		//   {KEY_F5,            VK_F5},
		//   {KEY_F6,            VK_F6},
		//   {KEY_F7,            VK_F7},
		//   {KEY_F8,            VK_F8},
		//   {KEY_F9,            VK_F9},
		//   {KEY_F10,           VK_F10},
		//   {KEY_F11,           VK_F11},
		//   {KEY_F12,           VK_F12},
		{KEY_BREAK,         VK_CANCEL},
		{KEY_BS,            VK_BACK},
		{KEY_TAB,           VK_TAB},
		{KEY_ENTER,         VK_RETURN},
		{KEY_NUMENTER,      VK_RETURN}, //????
		{KEY_ESC,           VK_ESCAPE},
		{KEY_SPACE,         VK_SPACE},
		{KEY_NUMPAD5,       VK_CLEAR},
	};

 	WORD EventType=KEY_EVENT;

 	DWORD FKey  =Key&KEY_END_SKEY;
 	DWORD FShift=Key&KEY_CTRLMASK;

	VirtKey=0;

  	ControlState=(FShift&KEY_SHIFT?PKF_SHIFT:0)|
  	             (FShift&KEY_ALT?PKF_ALT:0)|
 	             (FShift&KEY_RALT?PKF_RALT:0)|
 	             (FShift&KEY_RCTRL?PKF_RCONTROL:0)|
  	             (FShift&KEY_CTRL?PKF_CONTROL:0);

	bool KeyInTable=false;
	size_t i;
	for (i=0; i < ARRAYSIZE(Table_KeyToVK); i++)
	{
		if (FKey==(DWORD)Table_KeyToVK[i].Key)
		{
			VirtKey=Table_KeyToVK[i].VK;
			KeyInTable=true;
			break;
		}
	}

	if (!KeyInTable)
	{
 		// TODO: KEY_ALTDIGIT
 		if ((FKey>=L'0' && FKey<=L'9') || (FKey>=L'A' && FKey<=L'Z'))
			VirtKey=FKey;
 		//else if (FKey > KEY_VK_0xFF_BEGIN && FKey < KEY_VK_0xFF_END)
 		//	VirtKey=FKey-KEY_FKEY_BEGIN;
		else if (FKey > KEY_FKEY_BEGIN && FKey < KEY_END_FKEY)
			VirtKey=FKey-KEY_FKEY_BEGIN;
		else if (FKey && FKey < WCHAR_MAX)
		{
			VirtKey=VkKeyScan(static_cast<WCHAR>(FKey));
			if (HIBYTE(VirtKey))
			{
				VirtKey&=0xFF;
				FShift|=
					    (HIBYTE(VirtKey)&1?KEY_SHIFT:0)|
					    (HIBYTE(VirtKey)&2?KEY_CTRL:0)|
					    (HIBYTE(VirtKey)&4?KEY_ALT:0);
			  	ControlState=(FShift&KEY_SHIFT?PKF_SHIFT:0)|
  	        		     (FShift&KEY_ALT?PKF_ALT:0)|
		 	             (FShift&KEY_RALT?PKF_RALT:0)|
 	    		         (FShift&KEY_RCTRL?PKF_RCONTROL:0)|
  	            		 (FShift&KEY_CTRL?PKF_CONTROL:0);
			}

		}
		else if (!FKey)
		{
			DWORD ExtKey[]={KEY_SHIFT,VK_SHIFT,KEY_CTRL,VK_CONTROL,KEY_ALT,VK_MENU,KEY_RSHIFT,VK_RSHIFT,KEY_RCTRL,VK_RCONTROL,KEY_RALT,VK_RMENU};
			for (i=0; i < ARRAYSIZE(ExtKey); i+=2)
				if(FShift == ExtKey[i])
				{
					VirtKey=ExtKey[i+1];
					break;
				}
		}
		else
 		{
  			VirtKey=FKey;
 			switch (FKey)
 			{
 				case KEY_MSWHEEL_UP:
 				case KEY_MSWHEEL_DOWN:
 				case KEY_MSWHEEL_LEFT:
 				case KEY_MSWHEEL_RIGHT:
 				case KEY_MSLCLICK:
 				case KEY_MSRCLICK:
 				case KEY_MSM1CLICK:
 				case KEY_MSM2CLICK:
 				case KEY_MSM3CLICK:
 					EventType=MOUSE_EVENT;
 					break;
 			}
 		}
	}

	/* TODO:
		KEY_CTRLALTSHIFTPRESS
		KEY_CTRLALTSHIFTRELEASE
		KEY_RCTRLALTSHIFTPRESS
		KEY_RCTRLALTSHIFTRELEASE
	*/


	if (Rec)
	{
		Rec->EventType=EventType;

		switch (EventType)
		{
			case KEY_EVENT:
			{
				if (VirtKey)
				{
					Rec->Event.KeyEvent.bKeyDown=1;
					Rec->Event.KeyEvent.wRepeatCount=1;
					Rec->Event.KeyEvent.wVirtualKeyCode=VirtKey;
					Rec->Event.KeyEvent.wVirtualScanCode = MapVirtualKey(Rec->Event.KeyEvent.wVirtualKeyCode,MAPVK_VK_TO_VSC);
					Rec->Event.KeyEvent.uChar.UnicodeChar=(WORD)(FKey > WCHAR_MAX?0:FKey);

					// здесь подход к Shift-клавишам другой, нежели для ControlState
					Rec->Event.KeyEvent.dwControlKeyState=
					    (FShift&KEY_SHIFT?SHIFT_PRESSED:0)|
					    (FShift&KEY_ALT?LEFT_ALT_PRESSED:0)|
					    (FShift&KEY_CTRL?LEFT_CTRL_PRESSED:0)|
					    (FShift&KEY_RALT?RIGHT_ALT_PRESSED:0)|
					    (FShift&KEY_RCTRL?RIGHT_CTRL_PRESSED:0);

					DWORD ExtKey[]={KEY_PGUP,KEY_PGDN,KEY_END,KEY_HOME,KEY_LEFT,KEY_UP,KEY_RIGHT,KEY_DOWN,KEY_INS,KEY_DEL,KEY_NUMENTER};
					for (i=0; i < ARRAYSIZE(ExtKey); i++)
						if(FKey == ExtKey[i])
						{
							Rec->Event.KeyEvent.dwControlKeyState|=ENHANCED_KEY;
							break;
						}
				}
				break;
			}

			case MOUSE_EVENT:
			{
				DWORD ButtonState=0;
				DWORD EventFlags=0;

				switch (FKey)
				{
					case KEY_MSWHEEL_UP:
						ButtonState=MAKELONG(0,120);
						EventFlags|=MOUSE_WHEELED;
						break;
					case KEY_MSWHEEL_DOWN:
						ButtonState=MAKELONG(0,(WORD)(short)-120);
						EventFlags|=MOUSE_WHEELED;
						break;
					case KEY_MSWHEEL_RIGHT:
						ButtonState=MAKELONG(0,120);
						EventFlags|=MOUSE_HWHEELED;
						break;
					case KEY_MSWHEEL_LEFT:
						ButtonState=MAKELONG(0,(WORD)(short)-120);
						EventFlags|=MOUSE_HWHEELED;
						break;

					case KEY_MSLCLICK:
						ButtonState=FROM_LEFT_1ST_BUTTON_PRESSED;
						break;
					case KEY_MSRCLICK:
						ButtonState=RIGHTMOST_BUTTON_PRESSED;
						break;
					case KEY_MSM1CLICK:
						ButtonState=FROM_LEFT_2ND_BUTTON_PRESSED;
						break;
					case KEY_MSM2CLICK:
						ButtonState=FROM_LEFT_3RD_BUTTON_PRESSED;
						break;
					case KEY_MSM3CLICK:
						ButtonState=FROM_LEFT_4TH_BUTTON_PRESSED;
						break;
				}

				Rec->Event.MouseEvent.dwButtonState=ButtonState;
				Rec->Event.MouseEvent.dwEventFlags=EventFlags;
				Rec->Event.MouseEvent.dwControlKeyState=
					    (FShift&KEY_SHIFT?SHIFT_PRESSED:0)|
					    (FShift&KEY_ALT?LEFT_ALT_PRESSED:0)|
					    (FShift&KEY_CTRL?LEFT_CTRL_PRESSED:0)|
					    (FShift&KEY_RALT?RIGHT_ALT_PRESSED:0)|
					    (FShift&KEY_RCTRL?RIGHT_CTRL_PRESSED:0);
				//TODO: MouseX/Y
				Rec->Event.MouseEvent.dwMousePosition.X=0; //IntKeyState.MouseX;
				Rec->Event.MouseEvent.dwMousePosition.Y=0; //IntKeyState.MouseY;
				break;
			}
			case WINDOW_BUFFER_SIZE_EVENT:
				break;
			case MENU_EVENT:
				break;
			case FOCUS_EVENT:
				break;
		}
	}

	return VirtKey;
}