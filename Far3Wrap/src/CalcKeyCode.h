
//Extract from Far3

// LeftOnly = true - значит для совместимости, можно возвращать только левые модификаторы LCtrl/LAlt
DWORD CalcKeyCode(bool LeftOnly, INPUT_RECORD *rec,int RealKey,int *NotMacros,bool ProcessCtrlCode)
{
	/* from global Far3 variables */
	struct {
		int   UseVk_oem_x; 
		DWORD CASRule;
		int   ShiftsKeyRules;
	} Opt = {1, (DWORD)-1, 1}; //TODO: Options?
	struct FarKeyboardState {
		int AltPressed;
		int CtrlPressed;
		int ShiftPressed;
		int RightAltPressed;
		int RightCtrlPressed;
		int RightShiftPressed;
		DWORD MouseButtonState;
		DWORD PrevMouseButtonState;
		int PrevLButtonPressed;
		int PrevRButtonPressed;
		int PrevMButtonPressed;
		SHORT PrevMouseX;
		SHORT PrevMouseY;
		SHORT MouseX;
		SHORT MouseY;
		int PreMouseEventFlags;
		int MouseEventFlags;
		int ReturnAltValue;   // только что был ввод Alt-Цифира?
	} IntKeyState={0};
	//static BOOL IsKeyCASPressed=FALSE; // CtrlAltShift - нажато или нет?
	//static BOOL IsKeyRCASPressed=FALSE; // Right CtrlAltShift - нажато или нет?
	int WaitInMainLoop=FALSE; // мы крутимся в основном цикле?
	int WaitInFastFind=FALSE; // идет процесс быстрого поиска в панелях?
	/* global variables end */

	_SVS(CleverSysLog Clev(L"CalcKeyCode"));
	_SVS(SysLog(L"CalcKeyCode -> %s| RealKey=%d  *NotMacros=%d",_INPUT_RECORD_Dump(rec),RealKey,(NotMacros?*NotMacros:0)));
	UINT CtrlState=(rec->EventType==MOUSE_EVENT)?rec->Event.MouseEvent.dwControlKeyState:rec->Event.KeyEvent.dwControlKeyState;
	UINT ScanCode=rec->Event.KeyEvent.wVirtualScanCode;
	UINT KeyCode=rec->Event.KeyEvent.wVirtualKeyCode;
	WCHAR Char=rec->Event.KeyEvent.uChar.UnicodeChar;
	//// // _SVS(if(KeyCode == VK_DECIMAL || KeyCode == VK_DELETE) SysLog(L"CalcKeyCode -> CtrlState=%04X KeyCode=%s ScanCode=%08X AsciiChar=%02X IntKeyState.ShiftPressed=%d ShiftPressedLast=%d",CtrlState,_VK_KEY_ToName(KeyCode), ScanCode, Char.AsciiChar,IntKeyState.ShiftPressed,ShiftPressedLast));

	if (NotMacros)
		*NotMacros=CtrlState&0x80000000?TRUE:FALSE; //-V112

//  CtrlState&=~0x80000000;

	if (!(rec->EventType==KEY_EVENT || rec->EventType == FARMACRO_KEY_EVENT || rec->EventType == MOUSE_EVENT))
		return(KEY_NONE);

	if (!RealKey)
	{
		IntKeyState.CtrlPressed=(CtrlState & (LEFT_CTRL_PRESSED|RIGHT_CTRL_PRESSED));
		IntKeyState.AltPressed=(CtrlState & (LEFT_ALT_PRESSED|RIGHT_ALT_PRESSED));
		IntKeyState.ShiftPressed=(CtrlState & SHIFT_PRESSED);
		IntKeyState.RightCtrlPressed=(CtrlState & RIGHT_CTRL_PRESSED);
		IntKeyState.RightAltPressed=(CtrlState & RIGHT_ALT_PRESSED);
		IntKeyState.RightShiftPressed=(CtrlState & SHIFT_PRESSED);
	}

	if (LeftOnly)
	{
		// LeftOnly = true - значит для совместимости, можно возвращать только левые модификаторы LCtrl/LAlt
		IntKeyState.RightCtrlPressed=FALSE;
		IntKeyState.RightAltPressed=FALSE;
		IntKeyState.RightShiftPressed=FALSE;
	}

	DWORD Modif=0;
	if (IntKeyState.CtrlPressed) // && !IntKeyState.AltPressed && !IntKeyState.ShiftPressed)
		Modif|=(IntKeyState.RightCtrlPressed?KEY_RCTRL:KEY_CTRL);
	//else
	if (IntKeyState.AltPressed) // && !IntKeyState.CtrlPressed && !IntKeyState.ShiftPressed)
		Modif|=(IntKeyState.RightAltPressed?KEY_RALT:KEY_ALT);
	//else
	if (IntKeyState.ShiftPressed) // && !IntKeyState.CtrlPressed && !IntKeyState.AltPressed)
		Modif|=KEY_SHIFT;
	//else
	//	Modif=(IntKeyState.CtrlPressed?KEY_CTRL:0)|(IntKeyState.AltPressed?KEY_ALT:0)|(IntKeyState.ShiftPressed?KEY_SHIFT:0);
	DWORD ModifAlt=(IntKeyState.RightAltPressed?KEY_RALT:(IntKeyState.AltPressed?KEY_ALT:0));
	DWORD ModifCtrl=(IntKeyState.RightCtrlPressed?KEY_RCTRL:(IntKeyState.CtrlPressed?KEY_CTRL:0));

	if (rec->EventType==MOUSE_EVENT)
	{
		if (!(rec->Event.MouseEvent.dwEventFlags==MOUSE_WHEELED || rec->Event.MouseEvent.dwEventFlags==MOUSE_HWHEELED || rec->Event.MouseEvent.dwEventFlags==0))
		{
			return(KEY_NONE);
		}

		if (rec->Event.MouseEvent.dwEventFlags==MOUSE_WHEELED)
		{
			if (((short)(HIWORD(rec->Event.MouseEvent.dwButtonState))) > 0)
				return(Modif|KEY_MSWHEEL_UP);
			else if (((short)(HIWORD(rec->Event.MouseEvent.dwButtonState))) < 0)
				return(Modif|KEY_MSWHEEL_DOWN);
		}
		else if (rec->Event.MouseEvent.dwEventFlags==MOUSE_HWHEELED)
		{
			if (((short)(HIWORD(rec->Event.MouseEvent.dwButtonState))) > 0)
				return(Modif|KEY_MSWHEEL_RIGHT);
			else if (((short)(HIWORD(rec->Event.MouseEvent.dwButtonState))) < 0)
				return(Modif|KEY_MSWHEEL_LEFT);
		}
		else if (rec->Event.MouseEvent.dwEventFlags==0)
		{
			switch (rec->Event.MouseEvent.dwButtonState)
			{
			case FROM_LEFT_1ST_BUTTON_PRESSED:
				return(Modif|KEY_MSLCLICK);
			case RIGHTMOST_BUTTON_PRESSED:
				return(Modif|KEY_MSRCLICK);
			case FROM_LEFT_2ND_BUTTON_PRESSED:
				return(Modif|KEY_MSM1CLICK);
			case FROM_LEFT_3RD_BUTTON_PRESSED:
				return(Modif|KEY_MSM2CLICK);
			case FROM_LEFT_4TH_BUTTON_PRESSED:
				return(Modif|KEY_MSM3CLICK);
			}
		}

		return(KEY_NONE);
	}

	if (rec->Event.KeyEvent.wVirtualKeyCode >= 0xFF && RealKey)
	{
		//VK_?=0x00FF, Scan=0x0013 uChar=[U=' ' (0x0000): A=' ' (0x00)] Ctrl=0x00000120 (casac - EcNs)
		//VK_?=0x00FF, Scan=0x0014 uChar=[U=' ' (0x0000): A=' ' (0x00)] Ctrl=0x00000120 (casac - EcNs)
		//VK_?=0x00FF, Scan=0x0015 uChar=[U=' ' (0x0000): A=' ' (0x00)] Ctrl=0x00000120 (casac - EcNs)
		//VK_?=0x00FF, Scan=0x001A uChar=[U=' ' (0x0000): A=' ' (0x00)] Ctrl=0x00000120 (casac - EcNs)
		//VK_?=0x00FF, Scan=0x001B uChar=[U=' ' (0x0000): A=' ' (0x00)] Ctrl=0x00000120 (casac - EcNs)
		//VK_?=0x00FF, Scan=0x001E uChar=[U=' ' (0x0000): A=' ' (0x00)] Ctrl=0x00000120 (casac - EcNs)
		//VK_?=0x00FF, Scan=0x001F uChar=[U=' ' (0x0000): A=' ' (0x00)] Ctrl=0x00000120 (casac - EcNs)
		//VK_?=0x00FF, Scan=0x0023 uChar=[U=' ' (0x0000): A=' ' (0x00)] Ctrl=0x00000120 (casac - EcNs)
		if (!rec->Event.KeyEvent.bKeyDown && (CtrlState&(ENHANCED_KEY|NUMLOCK_ON)))
			return Modif|(KEY_VK_0xFF_BEGIN+ScanCode);

		return KEY_IDLE;
	}

#if 0
	static DWORD Time=0;

	if (!AltValue)
	{
		Time=GetTickCount();
	}

	if (!rec->Event.KeyEvent.bKeyDown)
	{
		KeyCodeForALT_LastPressed=0;

		if (KeyCode==VK_MENU && AltValue)
		{
			//FlushInputBuffer();//???
			INPUT_RECORD TempRec;
			size_t ReadCount;
			Console.ReadInput(&TempRec, 1, ReadCount);
			IntKeyState.ReturnAltValue=TRUE;
			//_SVS(SysLog(L"0 AltNumPad -> AltValue=0x%0X CtrlState=%X",AltValue,CtrlState));
			AltValue&=0xFFFF;
			/*
			О перетаскивании из проводника / вставке текста в консоль, на примере буквы 'ы':

			1. Нажимается Alt:
			bKeyDown=TRUE,  wRepeatCount=1, wVirtualKeyCode=VK_MENU,    UnicodeChar=0,    dwControlKeyState=LEFT_ALT_PRESSED

			2. Через numpad-клавиши вводится код символа в OEM, если он туда мапится, или 63 ('?'), если не мапится:
			bKeyDown=TRUE,  wRepeatCount=1, wVirtualKeyCode=VK_NUMPAD2, UnicodeChar=0,    dwControlKeyState=LEFT_ALT_PRESSED
			bKeyDown=FALSE, wRepeatCount=1, wVirtualKeyCode=VK_NUMPAD2, UnicodeChar=0,    dwControlKeyState=LEFT_ALT_PRESSED
			bKeyDown=TRUE,  wRepeatCount=1, wVirtualKeyCode=VK_NUMPAD3, UnicodeChar=0,    dwControlKeyState=LEFT_ALT_PRESSED
			bKeyDown=FALSE, wRepeatCount=1, wVirtualKeyCode=VK_NUMPAD3, UnicodeChar=0,    dwControlKeyState=LEFT_ALT_PRESSED
			bKeyDown=TRUE,  wRepeatCount=1, wVirtualKeyCode=VK_NUMPAD5, UnicodeChar=0,    dwControlKeyState=LEFT_ALT_PRESSED
			bKeyDown=FALSE, wRepeatCount=1, wVirtualKeyCode=VK_NUMPAD5, UnicodeChar=0,    dwControlKeyState=LEFT_ALT_PRESSED

			3. Отжимается Alt, при этом в uChar.UnicodeChar лежит исходный символ:
			bKeyDown=FALSE, wRepeatCount=1, wVirtualKeyCode=VK_MENU,    UnicodeChar=1099, dwControlKeyState=0

			Мораль сей басни такова: если rec->Event.KeyEvent.uChar.UnicodeChar не пуст - берём его, а не то, что во время удерживания Alt пришло.
			*/

			if (rec->Event.KeyEvent.uChar.UnicodeChar)
			{
				// BUGBUG: в Windows 7 Event.KeyEvent.uChar.UnicodeChar _всегда_ заполнен, но далеко не всегда тем, чем надо.
				// условно считаем, что если интервал между нажатиями не превышает 50 мс, то это сгенерированная при D&D или вставке комбинация,
				// иначе - ручной ввод.
				if (GetTickCount()-Time<50)
				{
					AltValue=rec->Event.KeyEvent.uChar.UnicodeChar;
				}
			}
			else
			{
				rec->Event.KeyEvent.uChar.UnicodeChar=static_cast<WCHAR>(AltValue);
			}

			//// // _SVS(SysLog(L"KeyCode==VK_MENU -> AltValue=%X (%c)",AltValue,AltValue));
			return(AltValue);
		}
		else
			return(KEY_NONE);
	}
#endif

	//BUGBUG: Что тут имелось в виду? Почему CtrlPressed вдруг сбрасывается?
/*
	if ((CtrlState & 9)==9)
	{
		if (Char)
			return Char;
		else
			IntKeyState.CtrlPressed=0;
	}
*/

#if 0
	if (KeyCode==VK_MENU)
		AltValue=0;
#endif

	if (Char && !IntKeyState.CtrlPressed && !IntKeyState.AltPressed)
	{
		if (KeyCode==VK_OEM_3 && !Opt.UseVk_oem_x)
			return(IntKeyState.ShiftPressed ? '~':'`');

		if (KeyCode==VK_OEM_7 && !Opt.UseVk_oem_x)
			return(IntKeyState.ShiftPressed ? '"':'\'');
	}

	if (Char<L' ' && (IntKeyState.CtrlPressed || IntKeyState.AltPressed))
	{
		switch (KeyCode)
		{
			case VK_OEM_COMMA:
				Char=L',';
				break;
			case VK_OEM_PERIOD:
				Char=L'.';
				break;
			case VK_OEM_4:

				if (!Opt.UseVk_oem_x)
					Char=L'[';

				break;
			case VK_OEM_5:

				//Char.AsciiChar=ScanCode==0x29?0x15:'\\'; //???
				if (!Opt.UseVk_oem_x)
					Char=L'\\';

				break;
			case VK_OEM_6:

				if (!Opt.UseVk_oem_x)
					Char=L']';

				break;
			case VK_OEM_7:

				if (!Opt.UseVk_oem_x)
					Char=L'\"';

				break;
		}
	}

#if 0
	/* $ 24.08.2000 SVS
	   "Персональные 100 грамм" :-)
	*/
	if (IntKeyState.CtrlPressed && IntKeyState.AltPressed && IntKeyState.ShiftPressed)
	{
		switch (KeyCode)
		{
			case VK_SHIFT:
			case VK_MENU:
			case VK_CONTROL:
			{
				if (IntKeyState.RightCtrlPressed && IntKeyState.RightAltPressed && IntKeyState.RightShiftPressed)
				{
					if ((Opt.CASRule&2))
						return (IsKeyRCASPressed?KEY_RCTRLALTSHIFTPRESS:KEY_RCTRLALTSHIFTRELEASE);
				}
				else if (Opt.CASRule&1)
					return (IsKeyCASPressed?KEY_CTRLALTSHIFTPRESS:KEY_CTRLALTSHIFTRELEASE);
			}
		}
	}

	if (IntKeyState.RightCtrlPressed && IntKeyState.RightAltPressed && IntKeyState.RightShiftPressed)
	{
		switch (KeyCode)
		{
			case VK_RSHIFT:
			case VK_RMENU:
			case VK_RCONTROL:

				if (Opt.CASRule&2)
					return (IsKeyRCASPressed?KEY_RCTRLALTSHIFTPRESS:KEY_RCTRLALTSHIFTRELEASE);

				break;
		}
	}
#endif

	if (KeyCode>=VK_F1 && KeyCode<=VK_F24)
//    return(Modif+KEY_F1+((KeyCode-VK_F1)<<8));
		return(Modif+KEY_F1+((KeyCode-VK_F1)));

	int NotShift=!IntKeyState.CtrlPressed && !IntKeyState.AltPressed && !IntKeyState.ShiftPressed;

#if 0
	if (IntKeyState.AltPressed && !IntKeyState.CtrlPressed && !IntKeyState.ShiftPressed)
	{
		if (!AltValue)
		{
			if (KeyCode==VK_INSERT || KeyCode==VK_NUMPAD0)
			{
				if (CtrlObject && CtrlObject->Macro.IsRecording())
				{
					_KEYMACRO(SysLog(L"[%d] CALL CtrlObject->Macro.ProcessKey(KEY_INS|KEY_ALT)",__LINE__));
					CtrlObject->Macro.ProcessKey(KEY_INS|KEY_ALT);
				}

				// макрос проигрывается и мы "сейчас" в состоянии выполнения функции waitkey? (Mantis#0000968: waitkey() пропускает AltIns)
				if (CtrlObject->Macro.IsExecuting() && CtrlObject->Macro.CheckWaitKeyFunc())
					return KEY_INS|KEY_ALT;

				RunGraber();
				return(KEY_NONE);
			}
		}

		//// // _SVS(SysLog(L"1 AltNumPad -> CalcKeyCode -> KeyCode=%s  ScanCode=0x%0X AltValue=0x%0X CtrlState=%X GetAsyncKeyState(VK_SHIFT)=%X",_VK_KEY_ToName(KeyCode),ScanCode,AltValue,CtrlState,GetAsyncKeyState(VK_SHIFT)));
		if (!(CtrlState & ENHANCED_KEY)
		        //(CtrlState&NUMLOCK_ON) && KeyCode >= VK_NUMPAD0 && KeyCode <= VK_NUMPAD9 ||
		        // !(CtrlState&NUMLOCK_ON) && KeyCode < VK_NUMPAD0
		   )
		{
			//// // _SVS(SysLog(L"2 AltNumPad -> CalcKeyCode -> KeyCode=%s  ScanCode=0x%0X AltValue=0x%0X CtrlState=%X GetAsyncKeyState(VK_SHIFT)=%X",_VK_KEY_ToName(KeyCode),ScanCode,AltValue,CtrlState,GetAsyncKeyState(VK_SHIFT)));
			static unsigned int ScanCodes[]={82,79,80,81,75,76,77,71,72,73};

			for (int I=0; I<int(ARRAYSIZE(ScanCodes)); I++)
			{
				if (ScanCodes[I]==ScanCode)
				{
					if (RealKey && (unsigned int)KeyCodeForALT_LastPressed != KeyCode)
					{
						AltValue=AltValue*10+I;
						KeyCodeForALT_LastPressed=KeyCode;
					}

//          _SVS(SysLog(L"AltNumPad -> AltValue=0x%0X CtrlState=%X",AltValue,CtrlState));
					if (AltValue)
						return(KEY_NONE);
				}
			}
		}
	}
#endif

	/*
	NumLock=Off
	  Down
	    CtrlState=0100 KeyCode=0028 ScanCode=00000050 AsciiChar=00         ENHANCED_KEY
	    CtrlState=0100 KeyCode=0028 ScanCode=00000050 AsciiChar=00
	  Num2
	    CtrlState=0000 KeyCode=0028 ScanCode=00000050 AsciiChar=00
	    CtrlState=0000 KeyCode=0028 ScanCode=00000050 AsciiChar=00

	  Ctrl-8
	    CtrlState=0008 KeyCode=0026 ScanCode=00000048 AsciiChar=00
	  Ctrl-Shift-8               ^^!!!
	    CtrlState=0018 KeyCode=0026 ScanCode=00000048 AsciiChar=00

	------------------------------------------------------------------------
	NumLock=On

	  Down
	    CtrlState=0120 KeyCode=0028 ScanCode=00000050 AsciiChar=00         ENHANCED_KEY
	    CtrlState=0120 KeyCode=0028 ScanCode=00000050 AsciiChar=00
	  Num2
	    CtrlState=0020 KeyCode=0062 ScanCode=00000050 AsciiChar=32
	    CtrlState=0020 KeyCode=0062 ScanCode=00000050 AsciiChar=32

	  Ctrl-8
	    CtrlState=0028 KeyCode=0068 ScanCode=00000048 AsciiChar=00
	  Ctrl-Shift-8               ^^!!!
	    CtrlState=0028 KeyCode=0026 ScanCode=00000048 AsciiChar=00
	*/

	/* ------------------------------------------------------------- */
	switch (KeyCode)
	{
		case VK_F24+1:
			return Modif|KEY_MSWHEEL_DOWN;
		case VK_F24+2:
			return Modif|KEY_MSWHEEL_UP;
		case VK_F24+3:
			return Modif|KEY_MSWHEEL_LEFT;
		case VK_F24+4:
			return Modif|KEY_MSWHEEL_RIGHT;

		case VK_INSERT:
		case VK_NUMPAD0:

			if (CtrlState&ENHANCED_KEY)
			{
				return(Modif|KEY_INS);
			}
			else if ((CtrlState&NUMLOCK_ON) && NotShift && KeyCode == VK_NUMPAD0)
				return '0';

			return Modif|KEY_NUMPAD0;
		case VK_DOWN:
		case VK_NUMPAD2:

			if (CtrlState&ENHANCED_KEY)
			{
				return(Modif|KEY_DOWN);
			}
			else if ((CtrlState&NUMLOCK_ON) && NotShift && KeyCode == VK_NUMPAD2)
				return '2';

			return Modif|KEY_NUMPAD2;
		case VK_LEFT:
		case VK_NUMPAD4:

			if (CtrlState&ENHANCED_KEY)
			{
				return(Modif|KEY_LEFT);
			}
			else if ((CtrlState&NUMLOCK_ON) && NotShift && KeyCode == VK_NUMPAD4)
				return '4';

			return Modif|KEY_NUMPAD4;
		case VK_RIGHT:
		case VK_NUMPAD6:

			if (CtrlState&ENHANCED_KEY)
			{
				return(Modif|KEY_RIGHT);
			}
			else if ((CtrlState&NUMLOCK_ON) && NotShift && KeyCode == VK_NUMPAD6)
				return '6';

			return Modif|KEY_NUMPAD6;
		case VK_UP:
		case VK_NUMPAD8:

			if (CtrlState&ENHANCED_KEY)
			{
				return(Modif|KEY_UP);
			}
			else if ((CtrlState&NUMLOCK_ON) && NotShift && KeyCode == VK_NUMPAD8)
				return '8';

			return Modif|KEY_NUMPAD8;
		case VK_END:
		case VK_NUMPAD1:

			if (CtrlState&ENHANCED_KEY)
			{
				return(Modif|KEY_END);
			}
			else if ((CtrlState&NUMLOCK_ON) && NotShift && KeyCode == VK_NUMPAD1)
				return '1';

			return Modif|KEY_NUMPAD1;
		case VK_HOME:
		case VK_NUMPAD7:

			if (CtrlState&ENHANCED_KEY)
			{
				return(Modif|KEY_HOME);
			}
			else if ((CtrlState&NUMLOCK_ON) && NotShift && KeyCode == VK_NUMPAD7)
				return '7';

			return Modif|KEY_NUMPAD7;
		case VK_NEXT:
		case VK_NUMPAD3:

			if (CtrlState&ENHANCED_KEY)
			{
				return(Modif|KEY_PGDN);
			}
			else if ((CtrlState&NUMLOCK_ON) && NotShift && KeyCode == VK_NUMPAD3)
				return '3';

			return Modif|KEY_NUMPAD3;
		case VK_PRIOR:
		case VK_NUMPAD9:

			if (CtrlState&ENHANCED_KEY)
			{
				return(Modif|KEY_PGUP);
			}
			else if ((CtrlState&NUMLOCK_ON) && NotShift && KeyCode == VK_NUMPAD9)
				return '9';

			return Modif|KEY_NUMPAD9;
		case VK_CLEAR:
		case VK_NUMPAD5:

			if (CtrlState&ENHANCED_KEY)
			{
				return(Modif|KEY_NUMPAD5);
			}
			else if ((CtrlState&NUMLOCK_ON) && NotShift && KeyCode == VK_NUMPAD5)
				return '5';

			return Modif|KEY_NUMPAD5;
		case VK_DELETE:
		case VK_DECIMAL:

			if (CtrlState&ENHANCED_KEY)
			{
				return (Modif|KEY_DEL);
			}
			else if ((CtrlState&NUMLOCK_ON) && NotShift && KeyCode == VK_DECIMAL)
				return KEY_DECIMAL;

			return Modif|KEY_NUMDEL;
	}

	switch (KeyCode)
	{
		case VK_RETURN:
			//  !!!!!!!!!!!!! - Если "!IntKeyState.ShiftPressed", то Shift-F4 Shift-Enter, не
			//                  отпуская Shift...
//_SVS(SysLog(L"IntKeyState.ShiftPressed=%d RealKey=%d !ShiftPressedLast=%d !IntKeyState.CtrlPressed=%d !IntKeyState.AltPressed=%d (%d)",IntKeyState.ShiftPressed,RealKey,ShiftPressedLast,IntKeyState.CtrlPressed,IntKeyState.AltPressed,(IntKeyState.ShiftPressed && RealKey && !ShiftPressedLast && !IntKeyState.CtrlPressed && !IntKeyState.AltPressed)));
#if 0

			if (IntKeyState.ShiftPressed && RealKey && !ShiftPressedLast && !IntKeyState.CtrlPressed && !IntKeyState.AltPressed && !LastShiftEnterPressed)
				return(KEY_ENTER);

			LastShiftEnterPressed=Modif&KEY_SHIFT?TRUE:FALSE;
			return(Modif|KEY_ENTER);
#else

	#if 0 // RealKey == FALSE
			if (IntKeyState.ShiftPressed && RealKey && !ShiftPressedLast && !IntKeyState.CtrlPressed && !IntKeyState.AltPressed && !LastShiftEnterPressed)
				return (CtrlState&ENHANCED_KEY)?KEY_NUMENTER:KEY_ENTER;

			LastShiftEnterPressed=Modif&KEY_SHIFT?TRUE:FALSE;
	#endif
			return Modif|((CtrlState&ENHANCED_KEY)?KEY_NUMENTER:KEY_ENTER);
#endif
		case VK_BROWSER_BACK:
			return Modif|KEY_BROWSER_BACK;
		case VK_BROWSER_FORWARD:
			return Modif|KEY_BROWSER_FORWARD;
		case VK_BROWSER_REFRESH:
			return Modif|KEY_BROWSER_REFRESH;
		case VK_BROWSER_STOP:
			return Modif|KEY_BROWSER_STOP;
		case VK_BROWSER_SEARCH:
			return Modif|KEY_BROWSER_SEARCH;
		case VK_BROWSER_FAVORITES:
			return Modif|KEY_BROWSER_FAVORITES;
		case VK_BROWSER_HOME:
			return Modif|KEY_BROWSER_HOME;
		case VK_VOLUME_MUTE:
			return Modif|KEY_VOLUME_MUTE;
		case VK_VOLUME_DOWN:
			return Modif|KEY_VOLUME_DOWN;
		case VK_VOLUME_UP:
			return Modif|KEY_VOLUME_UP;
		case VK_MEDIA_NEXT_TRACK:
			return Modif|KEY_MEDIA_NEXT_TRACK;
		case VK_MEDIA_PREV_TRACK:
			return Modif|KEY_MEDIA_PREV_TRACK;
		case VK_MEDIA_STOP:
			return Modif|KEY_MEDIA_STOP;
		case VK_MEDIA_PLAY_PAUSE:
			return Modif|KEY_MEDIA_PLAY_PAUSE;
		case VK_LAUNCH_MAIL:
			return Modif|KEY_LAUNCH_MAIL;
		case VK_LAUNCH_MEDIA_SELECT:
			return Modif|KEY_LAUNCH_MEDIA_SELECT;
		case VK_LAUNCH_APP1:
			return Modif|KEY_LAUNCH_APP1;
		case VK_LAUNCH_APP2:
			return Modif|KEY_LAUNCH_APP2;
		case VK_APPS:
			return(Modif|KEY_APPS);
		case VK_LWIN:
			return(Modif|KEY_LWIN);
		case VK_RWIN:
			return(Modif|KEY_RWIN);
		case VK_BACK:
			return(Modif|KEY_BS);
		case VK_SPACE:
			if (Char == L' ' || !Char)
				return(Modif|KEY_SPACE);
			return Char;
		case VK_TAB:
			return(Modif|KEY_TAB);
		case VK_ADD:
			return(Modif|KEY_ADD);
		case VK_SUBTRACT:
			return(Modif|KEY_SUBTRACT);
		case VK_ESCAPE:
			return(Modif|KEY_ESC);
	}

	switch (KeyCode)
	{
		case VK_CAPITAL:
			return(Modif|KEY_CAPSLOCK);
		case VK_NUMLOCK:
			return(Modif|KEY_NUMLOCK);
		case VK_SCROLL:
			return(Modif|KEY_SCROLLLOCK);
	}

	/* ------------------------------------------------------------- */
	if (IntKeyState.CtrlPressed && IntKeyState.AltPressed && IntKeyState.ShiftPressed)
	{

		_SVS(if (KeyCode!=VK_CONTROL && KeyCode!=VK_MENU) SysLog(L"CtrlAltShift -> |%s|%s|",_VK_KEY_ToName(KeyCode),_INPUT_RECORD_Dump(rec)));

		if (KeyCode>='A' && KeyCode<='Z')
			return((Modif)+KeyCode);

		if (Opt.ShiftsKeyRules) //???
			switch (KeyCode)
			{
				case VK_OEM_3:
					return(Modif+'`');
				case VK_OEM_MINUS:
					return(Modif+'-');
				case VK_OEM_PLUS:
					return(Modif+'=');
				case VK_OEM_5:
					return(Modif+KEY_BACKSLASH);
				case VK_OEM_6:
					return(Modif+KEY_BACKBRACKET);
				case VK_OEM_4:
					return(Modif+KEY_BRACKET);
				case VK_OEM_7:
					return(Modif+'\'');
				case VK_OEM_1:
					return(Modif+KEY_SEMICOLON);
				case VK_OEM_2:
					return(Modif+KEY_SLASH);
				case VK_OEM_PERIOD:
					return(Modif+KEY_DOT);
				case VK_OEM_COMMA:
					return(Modif+KEY_COMMA);
				case VK_OEM_102: // <> \|
 					return Modif+KEY_BACKSLASH;
			}

		switch (KeyCode)
		{
			case VK_DIVIDE:
				return(Modif|KEY_DIVIDE);
			case VK_MULTIPLY:
				return(Modif|KEY_MULTIPLY);
			case VK_CANCEL:
				return(Modif|KEY_PAUSE);
			case VK_SLEEP:
				return(Modif|KEY_STANDBY);
			case VK_SNAPSHOT:
				return(Modif|KEY_PRNTSCRN);
		}

		if (Char)
			return(Modif|Char);

		if (!RealKey && (KeyCode==VK_CONTROL || KeyCode==VK_MENU))
			return(KEY_NONE);

		if (KeyCode)
			return(Modif|KeyCode);
	}

	/* ------------------------------------------------------------- */
	if (IntKeyState.CtrlPressed && IntKeyState.AltPressed)
	{

		_SVS(if (KeyCode!=VK_CONTROL && KeyCode!=VK_MENU) SysLog(L"CtrlAlt -> |%s|%s|",_VK_KEY_ToName(KeyCode),_INPUT_RECORD_Dump(rec)));

		if (KeyCode>='A' && KeyCode<='Z')
			return(Modif+KeyCode);

		if (Opt.ShiftsKeyRules) //???
			switch (KeyCode)
			{
				case VK_OEM_3:
					return(Modif+'`');
				case VK_OEM_MINUS:
					return(Modif+'-');
				case VK_OEM_PLUS:
					return(Modif+'=');
				case VK_OEM_5:
					return(Modif+KEY_BACKSLASH);
				case VK_OEM_6:
					return(Modif+KEY_BACKBRACKET);
				case VK_OEM_4:
					return(Modif+KEY_BRACKET);
				case VK_OEM_7:
					return(Modif+'\'');
				case VK_OEM_1:
					return(Modif+KEY_SEMICOLON);
				case VK_OEM_2:
					return(Modif+KEY_SLASH);
				case VK_OEM_PERIOD:
					return(Modif+KEY_DOT);
				case VK_OEM_COMMA:
					return(Modif+KEY_COMMA);
				case VK_OEM_102: // <> \|
 					return Modif+KEY_BACKSLASH;
			}

		switch (KeyCode)
		{
			case VK_DIVIDE:
				return(Modif|KEY_DIVIDE);
			case VK_MULTIPLY:
				return(Modif|KEY_MULTIPLY);
				// KEY_EVENT_RECORD: Dn, 1, Vk="VK_CANCEL" [3/0x0003], Scan=0x0046 uChar=[U=' ' (0x0000): A=' ' (0x00)] Ctrl=0x0000014A (CAsac - EcnS)
				//case VK_PAUSE:
			case VK_CANCEL: // Ctrl-Alt-Pause

				if (!IntKeyState.ShiftPressed && (CtrlState&ENHANCED_KEY))
					return(Modif|KEY_PAUSE);

				return KEY_NONE;
			case VK_SLEEP:
				return(Modif|KEY_STANDBY);
			case VK_SNAPSHOT:
				return(Modif|KEY_PRNTSCRN);
		}

		if (Char)
			return(Modif|Char);

		if (!RealKey && (KeyCode==VK_CONTROL || KeyCode==VK_MENU))
			return(KEY_NONE);

		if (KeyCode)
			return(Modif+KeyCode);
	}

	/* ------------------------------------------------------------- */
	if (IntKeyState.AltPressed && IntKeyState.ShiftPressed)
	{

		_SVS(if (KeyCode!=VK_MENU && KeyCode!=VK_SHIFT) SysLog(L"AltShift -> |%s|%s|",_VK_KEY_ToName(KeyCode),_INPUT_RECORD_Dump(rec)));

		if (KeyCode>='0' && KeyCode<='9')
		{
#if 0
			//TODO: Разобраться, что можно возвращать
			if (WaitInFastFind > 0 &&
			        CtrlObject->Macro.GetCurRecord(nullptr,nullptr) < MACROMODE_RECORDING &&
			        CtrlObject->Macro.GetIndex(KEY_ALTSHIFT0+KeyCode-'0',-1) == -1)
			{
				return KEY_ALT|KEY_SHIFT|Char; //BUGBUG: А тут можно Modif| использовать?
			}
			else
#endif
			{
				/*
				return(KEY_ALTSHIFT0+KeyCode-'0');
				*/
				return(Modif+KeyCode); //BUGBUG: А тут можно Modif| использовать?
			}
		}

		if (!WaitInMainLoop && KeyCode>='A' && KeyCode<='Z')
			return(Modif+KeyCode);

		if (Opt.ShiftsKeyRules) //???
			switch (KeyCode)
			{
				case VK_OEM_3:
					return(Modif+'`');
				case VK_OEM_MINUS:
					return(Modif+'_');
				case VK_OEM_PLUS:
					return(Modif+'=');
				case VK_OEM_5:
					return(Modif+KEY_BACKSLASH);
				case VK_OEM_6:
					return(Modif+KEY_BACKBRACKET);
				case VK_OEM_4:
					return(Modif+KEY_BRACKET);
				case VK_OEM_7:
					return(Modif+'\'');
				case VK_OEM_1:
					return(Modif+KEY_SEMICOLON);
				case VK_OEM_2:
					//if(WaitInFastFind)
					//  return(Modif+'?');
					//else
					return(Modif+KEY_SLASH);
				case VK_OEM_PERIOD:
					return(Modif+KEY_DOT);
				case VK_OEM_COMMA:
					return(Modif+KEY_COMMA);
				case VK_OEM_102: // <> \|
 					return Modif+KEY_BACKSLASH;
			}

		switch (KeyCode)
		{
			case VK_DIVIDE:
				//if(WaitInFastFind)
				//  return(Modif+'/');
				//else
				return(Modif|KEY_DIVIDE);
			case VK_MULTIPLY:
				//if(WaitInFastFind)
				//{
				//  return(Modif+'*');
				//}
				//else
				return(Modif|KEY_MULTIPLY);
			case VK_PAUSE:
				return(Modif|KEY_PAUSE);
			case VK_SLEEP:
				return(Modif|KEY_STANDBY);
			case VK_SNAPSHOT:
				return(Modif|KEY_PRNTSCRN);
		}

		if (Char)
			return(Modif|Char);

		if (!RealKey && (KeyCode==VK_MENU || KeyCode==VK_SHIFT))
			return(KEY_NONE);

		if (KeyCode)
			return(Modif+KeyCode);
	}

	/* ------------------------------------------------------------- */
	if (IntKeyState.CtrlPressed && IntKeyState.ShiftPressed)
	{

		_SVS(if (KeyCode!=VK_CONTROL && KeyCode!=VK_SHIFT) SysLog(L"CtrlShift -> |%s|%s|",_VK_KEY_ToName(KeyCode),_INPUT_RECORD_Dump(rec)));

		if (KeyCode>='0' && KeyCode<='9')
			return(Modif|KeyCode);

		if (KeyCode>='A' && KeyCode<='Z')
			return(Modif|KeyCode);

		switch (KeyCode)
		{
			case VK_OEM_PERIOD:
				return(Modif|KEY_DOT);
			case VK_OEM_4:
				return(Modif|KEY_BRACKET);
			case VK_OEM_6:
				return(Modif|KEY_BACKBRACKET);
			case VK_OEM_2:
				return(Modif|KEY_SLASH);
			case VK_OEM_5:
				return(Modif|KEY_BACKSLASH);
			case VK_DIVIDE:
				return(Modif|KEY_DIVIDE);
			case VK_MULTIPLY:
				return(Modif|KEY_MULTIPLY);
			case VK_SLEEP:
				return(Modif|KEY_STANDBY);
			case VK_SNAPSHOT:
				return(Modif|KEY_PRNTSCRN);
		}

		if (Opt.ShiftsKeyRules) //???
			switch (KeyCode)
			{
				case VK_OEM_3:
					return(Modif+'`');
				case VK_OEM_MINUS:
					return(Modif+'-');
				case VK_OEM_PLUS:
					return(Modif+'=');
				case VK_OEM_7:
					return(Modif+'\'');
				case VK_OEM_1:
					return(Modif+KEY_SEMICOLON);
				case VK_OEM_COMMA:
					return(Modif+KEY_COMMA);
				case VK_OEM_102: // <> \|
 					return(Modif+KEY_BACKSLASH);
			}

		if (Char)
			return(Modif|Char);

		if (!RealKey && (KeyCode==VK_CONTROL || KeyCode==VK_SHIFT))
			return(KEY_NONE);

		if (KeyCode)
			return(Modif+KeyCode);
	}

	/* ------------------------------------------------------------- */
	if ((CtrlState & RIGHT_CTRL_PRESSED)==RIGHT_CTRL_PRESSED)
	{
		if (KeyCode>='0' && KeyCode<='9')
			return(KEY_RCTRL0+KeyCode-'0');
	}

	/* ------------------------------------------------------------- */
	if (!IntKeyState.CtrlPressed && !IntKeyState.AltPressed && !IntKeyState.ShiftPressed)
	{
		switch (KeyCode)
		{
			case VK_DIVIDE:
				return(KEY_DIVIDE);
			case VK_CANCEL:
#if 0
				CtrlObject->Macro.SendDropProcess();
#endif
				return(KEY_BREAK);
			case VK_MULTIPLY:
				return(KEY_MULTIPLY);
			case VK_PAUSE:
				return(KEY_PAUSE);
			case VK_SLEEP:
				return KEY_STANDBY;
			case VK_SNAPSHOT:
				return KEY_PRNTSCRN;
		}
	}
	else if (KeyCode == VK_CANCEL && IntKeyState.CtrlPressed && !IntKeyState.AltPressed && !IntKeyState.ShiftPressed)
	{
#if 0
		CtrlObject->Macro.SendDropProcess();
#endif
		return(ModifCtrl|KEY_BREAK);
	}

	/* ------------------------------------------------------------- */
	if (IntKeyState.CtrlPressed)
	{

		_SVS(if (KeyCode!=VK_CONTROL) SysLog(L"Ctrl -> |%s|%s|",_VK_KEY_ToName(KeyCode),_INPUT_RECORD_Dump(rec)));

		if (KeyCode>='0' && KeyCode<='9')
			return(ModifCtrl+KeyCode);

		if (KeyCode>='A' && KeyCode<='Z')
			return(ModifCtrl+KeyCode);

		switch (KeyCode)
		{
			case VK_OEM_COMMA:
				return(ModifCtrl|KEY_COMMA);
			case VK_OEM_PERIOD:
				return(KEY_CTRLDOT); //BUGBUG: Не стал ModifCtrl ставить, чтобы не слетела вдруг обработка Ctrl.
			case VK_OEM_2:
				return(ModifCtrl|KEY_SLASH);
			case VK_OEM_4:
				return(ModifCtrl|KEY_BRACKET);
			case VK_OEM_5:
				return(KEY_CTRLBACKSLASH); //BUGBUG: Не стал ModifCtrl ставить, чтобы не слетела вдруг обработка Ctrl\ ...
			case VK_OEM_6:
				return(ModifCtrl|KEY_BACKBRACKET);
			case VK_OEM_7:
				return(ModifCtrl+'\''); // KEY_QUOTE
			case VK_MULTIPLY:
				return(ModifCtrl|KEY_MULTIPLY);
			case VK_DIVIDE:
				return(ModifCtrl|KEY_DIVIDE);
			case VK_PAUSE:
				if (CtrlState&ENHANCED_KEY)
					return ModifCtrl|KEY_NUMLOCK;
#if 0
				CtrlObject->Macro.SendDropProcess();
#endif
				return KEY_BREAK;
			case VK_SLEEP:
				return ModifCtrl|KEY_STANDBY;
			case VK_SNAPSHOT:
				return ModifCtrl|KEY_PRNTSCRN;
			case VK_OEM_102: // <> \|
 				return ModifCtrl|KEY_BACKSLASH;
		}

		if (Opt.ShiftsKeyRules) //???
			switch (KeyCode)
			{
				case VK_OEM_3:
					return(ModifCtrl+'`');
				case VK_OEM_MINUS:
					return(ModifCtrl+'-');
				case VK_OEM_PLUS:
					return(ModifCtrl+'=');
				case VK_OEM_1:
					return(ModifCtrl+KEY_SEMICOLON);
			}

		if (KeyCode)
		{
			if (ProcessCtrlCode)
			{
				if (KeyCode == VK_CONTROL)
					return (IntKeyState.CtrlPressed && !IntKeyState.RightCtrlPressed)?KEY_CTRL:(IntKeyState.RightCtrlPressed?KEY_RCTRL:KEY_CTRL);
				else if (KeyCode == VK_RCONTROL)
					return KEY_RCTRL;
			}

			if (!RealKey && KeyCode==VK_CONTROL)
				return KEY_NONE;

			return(ModifCtrl+KeyCode);
		}
		
		if (Char)
			return (ModifCtrl|Char);
	}

	/* ------------------------------------------------------------- */
	if (IntKeyState.AltPressed)
	{

		_SVS(if (KeyCode!=VK_MENU) SysLog(L"Alt -> |%s|%s|",_VK_KEY_ToName(KeyCode),_INPUT_RECORD_Dump(rec)));

		if (Opt.ShiftsKeyRules) //???
			switch (KeyCode)
			{
				case VK_OEM_3:
					return(ModifAlt+'`');
				case VK_OEM_MINUS:
					//if(WaitInFastFind)
					//  return(KEY_ALT+KEY_SHIFT+'_');
					//else
					return(ModifAlt+'-');
				case VK_OEM_PLUS:
					return(ModifAlt+'=');
				case VK_OEM_5:
					return(ModifAlt+KEY_BACKSLASH);
				case VK_OEM_6:
					return(ModifAlt+KEY_BACKBRACKET);
				case VK_OEM_4:
					return(ModifAlt+KEY_BRACKET);
				case VK_OEM_7:
					return(ModifAlt+'\'');
				case VK_OEM_1:
					return(ModifAlt+KEY_SEMICOLON);
				case VK_OEM_2:
					return(ModifAlt+KEY_SLASH);
				case VK_OEM_102: // <> \|
 					return ModifAlt+KEY_BACKSLASH;
			}

		switch (KeyCode)
		{
			case VK_OEM_COMMA:
				return(ModifAlt|KEY_COMMA);
			case VK_OEM_PERIOD:
				return(ModifAlt|KEY_DOT);
			case VK_DIVIDE:
				//if(WaitInFastFind)
				//  return(KEY_ALT+KEY_SHIFT+'/');
				//else
				return(ModifAlt|KEY_DIVIDE);
			case VK_MULTIPLY:
//        if(WaitInFastFind)
//          return(KEY_ALT+KEY_SHIFT+'*');
//        else
				return(ModifAlt|KEY_MULTIPLY);
			case VK_PAUSE:
				return(ModifAlt+KEY_PAUSE);
			case VK_SLEEP:
				return ModifAlt|KEY_STANDBY;
			case VK_SNAPSHOT:
				return ModifAlt|KEY_PRNTSCRN;
		}

		if (Char)
		{
			if (!Opt.ShiftsKeyRules || WaitInFastFind > 0)
				return ModifAlt|Upper(Char);
			else if (WaitInMainLoop)
				return ModifAlt|Char;
		}

		if (ProcessCtrlCode)
		{
			if (KeyCode == VK_MENU)
				return (IntKeyState.AltPressed && !IntKeyState.RightAltPressed)?KEY_ALT:(IntKeyState.RightAltPressed?KEY_RALT:KEY_ALT);
			else if (KeyCode == VK_RMENU)
				return KEY_RALT;
		}

		if (!RealKey && KeyCode==VK_MENU)
			return KEY_NONE;

		return(ModifAlt+KeyCode);
	}

	if (IntKeyState.ShiftPressed)
	{

		_SVS(if (KeyCode!=VK_SHIFT) SysLog(L"Shift -> |%s|%s|",_VK_KEY_ToName(KeyCode),_INPUT_RECORD_Dump(rec)));

		switch (KeyCode)
		{

			case VK_OEM_MINUS:
				return '_';
				//return KEY_SHIFT|'-';
			case VK_OEM_PLUS:
				return '+';
				//return KEY_SHIFT|'+';
			case VK_DIVIDE:
				return KEY_SHIFT|KEY_DIVIDE;
			case VK_MULTIPLY:
				return KEY_SHIFT|KEY_MULTIPLY;
			case VK_PAUSE:
				return KEY_SHIFT|KEY_PAUSE;
			case VK_SLEEP:
				return KEY_SHIFT|KEY_STANDBY;
			case VK_SNAPSHOT:
				return KEY_SHIFT|KEY_PRNTSCRN;
		}

		if (ProcessCtrlCode)
		{
			if (KeyCode == VK_SHIFT)
				return KEY_SHIFT;
			else if (KeyCode == VK_RSHIFT)
				return KEY_RSHIFT;
		}

	}

	_ASSERTE(!IntKeyState.CtrlPressed && !IntKeyState.AltPressed);

	return Char?Char:KEY_NONE;
}
