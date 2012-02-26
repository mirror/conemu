
/**************************************************************************
               Синхронизация нитей с основной нитью фара
**************************************************************************/

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

#include "PictureView.h"
#include "PVDManager.h"
#include "Image.h"
#include "PictureView_Lang.h"
#include "PictureView_Display.h"
//
#include "ConEmuSupport.h"
#ifdef _DEBUG
#include <shlwapi.h>
#endif
#include <Tlhelp32.h>

BOOL ExecuteInMainThread(DWORD nCmd, int nPeekMsg)
{	
	BOOL lbResult = FALSE;
	if (!g_Plugin.hSynchroDone)
		return FALSE;

	CFunctionLogger::FunctionLogger(L"ExecuteInMainThread(%i)", nCmd);

	TODO("Возможность отмены блокировки секции");
	EnterCriticalSection(&g_Plugin.csMainThread);

	_ASSERTE((g_Plugin.FlagsWork & nCmd) == 0);
	g_Plugin.FlagsWorkResult &= ~(nCmd);
	ResetEvent(g_Plugin.hSynchroDone);
	g_Plugin.FlagsWork |= nCmd;

	bool  lbBreak = false;
	DWORD nWait = -1;
	while (!lbBreak && (nWait = WaitForSingleObject(g_Plugin.hSynchroDone, 10)) != WAIT_OBJECT_0)
	{
		if ((g_Plugin.FlagsWork & nCmd) == 0) {
			// Событие может быть выставлено уже после отрабатывания WaitForSingleObject как TIMEOUT
			lbResult = (g_Plugin.FlagsWorkResult & nCmd) != 0;
			break;
		}
		//EscapePressed не катит. Нажатие Esc в субдиалогах настройки и картинка опять показана
		//if (EscapePressed()) g_Plugin.FlagsWork |= FW_TERMINATE;

		if (g_Plugin.FlagsWork & FW_TERMINATE) {
			g_Plugin.FlagsWork &= ~nCmd;
			break;
		}

		if (nPeekMsg)
		{
			MSG  Msg;
			while (PeekMessage(&Msg, NULL, 0, 0, PM_REMOVE))
			{
				// По идее, этого произойти не должно? Разве что идет закрытие фара?
				if (Msg.message == WM_QUIT || Msg.message == WM_DESTROY) {
					// вернуть обратно в очередь
					PostMessage(Msg.hwnd, Msg.message, Msg.wParam, Msg.lParam);
					// и выйти
					lbBreak = true; break;
				}

				__try {
					if (nPeekMsg == 1)
						DefWindowProc(Msg.hwnd, Msg.message, Msg.wParam, Msg.lParam);
					else
						DispatchMessage(&Msg);
				}__except(EXCEPTION_EXECUTE_HANDLER){
					CFunctionLogger::FunctionLogger(L"!!! Exception in ExecuteInMainThread.DefWindowProc(%i)",Msg.message);
				}

				// Если команда завершилась - прекращаем обработку (точнее пропуск) сообщений
				if (WaitForSingleObject(g_Plugin.hSynchroDone, 0) == WAIT_OBJECT_0) {
					lbBreak = true;
					lbResult = (g_Plugin.FlagsWorkResult & nCmd) != 0;
					break;
				}
			}
		}
	}

	if (nWait == WAIT_OBJECT_0)
	{
		lbResult = (g_Plugin.FlagsWorkResult & nCmd) != 0;
	}

	//_ASSERTE((g_Plugin.FlagsWork & nCmd) == 0);
	g_Plugin.FlagsWork &= ~nCmd;

	CFunctionLogger::FunctionLogger(L"~ExecuteInMainThread(%i)", nCmd);

	LeaveCriticalSection(&g_Plugin.csMainThread);

	return lbResult;
}

EFlagsMainProcess ProcessMainThread()
{
	EFlagsMainProcess nResult = FEM_EXIT_PLUGIN;

	DWORD dwWait = 0;
	//HANDLE hEvents[2] = {g_Plugin.hDisplayEvent, g_Plugin.hDisplayThread};
	//Wait ForSingleObject(g_Plugin.hDisplayEvent, INFINITE); -- низя. Если нить грохнулась - мы здесь повиснем

	_ASSERTE(GetCurrentThreadId() == gnMainThreadId);

	// Бесконечное ожидание заменить на цикл, в котором опрашивать консоль. А из Display - убрать ReadConsoleInput
	// Это нужно для того, чтобы FAR не вис при переключении в другие приложения во время отрисовки диалогов и хелпа
	// НО нужно учесть, что если работает хелп/диалог то консоль читает сам фар?

	while (!(g_Plugin.FlagsWork & FW_TERMINATE)
		&& (dwWait = WaitForSingleObject(g_Plugin.hDisplayThread, 10)) == WAIT_TIMEOUT)
	{
		MCHKHEAP;

		if (g_Plugin.FlagsWork & FW_SHOW_HELP)
		{
			// Показать Help в основной нити
			CFunctionLogger flog(L"FW_SHOW_HELP");
			wchar_t Title[0x40];
			lstrcpynW(Title, g_Plugin.pszPluginTitle, sizeofarray(Title) - 6);
			lstrcat(Title, L" - Far");
			// Раньше выполнялось в нити Display, и часто висла
			SetConsoleTitleW(Title);

			g_StartupInfo.ShowHelp(g_StartupInfo.ModuleName, _T("Controls"), 0);

			SetEvent(g_Plugin.hSynchroDone);
			g_Plugin.FlagsWorkResult |= FW_SHOW_HELP;
			g_Plugin.FlagsWork &= ~FW_SHOW_HELP;			

		}
		else if (g_Plugin.FlagsWork & (FW_SHOW_CONFIG | FW_SHOW_MODULES))
		{
			// Показать окно конфига в основной нити
			CFunctionLogger flog(L"FW_SHOW_CONFIG|FW_SHOW_MODULES");

			//TRUE, если настройки были изменены
			const bool reconfig = ConfigureW(0);

			if (reconfig)
				g_Plugin.FlagsDisplay |= FD_REQ_REFRESH;
			SetEvent(g_Plugin.hSynchroDone);
			g_Plugin.FlagsWorkResult |= (g_Plugin.FlagsWork & (FW_SHOW_CONFIG|FW_SHOW_MODULES));
			g_Plugin.FlagsWork &= ~(FW_SHOW_CONFIG|FW_SHOW_MODULES);

		}
		else if (g_Plugin.FlagsWork & (FW_MARK_FILE | FW_UNMARK_FILE))
		{
			CFunctionLogger flog(L"FW_MARK_FILE | FW_UNMARK_FILE");

			//g_Plugin.Image[0]->bSelected = (g_Plugin.FlagsWork & FW_MARK_FILE) == FW_MARK_FILE;
			g_Plugin.SelectionChanged = true; // чтобы при выходе из плагина можно было обновить панель, показав выделенные вверху
			//g_StartupInfo.PanelControl(INVALID_HANDLE_VALUE, FCTL_SETSELECTION, g_Plugin.Image[0]->PanelItemRaw(), g_Plugin.Image[0]->bSelected);
			//g_StartupInfo.PanelControl(INVALID_HANDLE_VALUE, FCTL_REDRAWPANEL, 0, 0);
			g_Panel.MarkUnmarkFile( ((g_Plugin.FlagsWork & FW_MARK_FILE) == FW_MARK_FILE)
				? CPicViewPanel::ema_Mark : CPicViewPanel::ema_Unmark);
			TitleRepaint();
			SetEvent(g_Plugin.hSynchroDone);
			g_Plugin.FlagsWorkResult |= (g_Plugin.FlagsWork & (FW_MARK_FILE | FW_UNMARK_FILE));
			g_Plugin.FlagsWork &= ~(FW_MARK_FILE | FW_UNMARK_FILE);

		}
		else if (g_Plugin.FlagsWork & (FW_TITLEREPAINT | FW_TITLEREPAINTD))
		{
			CFunctionLogger flog(L"FW_TITLEREPAINT | FW_TITLEREPAINTD");

			TitleRepaint((g_Plugin.FlagsWork & FW_TITLEREPAINTD)==FW_TITLEREPAINTD);
			SetEvent(g_Plugin.hSynchroDone);
			g_Plugin.FlagsWorkResult |= (g_Plugin.FlagsWork & (FW_TITLEREPAINT | FW_TITLEREPAINTD));
			g_Plugin.FlagsWork &= ~(FW_TITLEREPAINT | FW_TITLEREPAINTD);

		}
		else if (g_Plugin.FlagsWork & (FW_QVIEWREPAINT))
		{
			CFunctionLogger flog(L"FW_QVIEWREPAINT");

			QViewRepaint();
			SetEvent(g_Plugin.hSynchroDone);
			g_Plugin.FlagsWorkResult |= FW_QVIEWREPAINT;
			g_Plugin.FlagsWork &= ~FW_QVIEWREPAINT;

		}
		else if (g_Plugin.FlagsWork & (FW_RETRIEVE_FILE))
		{
			CFunctionLogger flog(L"FW_RETRIEVE_FILE");

			// Отдать управление в FAR чтобы извлечь файл из архива
			bool lbMacroOk = g_Panel.StartItemExtraction();

			SetEvent(g_Plugin.hSynchroDone);

			g_Plugin.FlagsWork &= ~FW_RETRIEVE_FILE;

			if (lbMacroOk)
			{
				nResult = FEM_RETRIEVE_FILE;
				g_Plugin.FlagsWork |= FW_IN_RETRIEVE;
				break;
			}
			else
			{
				_ASSERTE(lbMacroOk == true);
			}
		}

		if (g_Plugin.hWnd)
			ProcessConsoleInputs();
	}

	if (dwWait == WAIT_OBJECT_0 && !(g_Plugin.FlagsWork & FW_TERMINATE))
	{
		_ASSERTE(nResult != FEM_RETRIEVE_FILE);
		RequestTerminate(2);
	}

	CFunctionLogger::FunctionLogger(L"ProcessMainThread done");

	return nResult;
}



// EntryPoint
//		Это общая точка входа для плагина. ПЕРВАЯ картинка серии!
// OpenFrom
//		0 - НЕявный вызов из панелей (перехват "входа в файл" по Enter/CtrlPgDn БЕЗ использования ассоциаций)
//		OPEN_BY_USER - ЯВНЫЙ вызов из панелей. Это может быть префикс или F11. Возможно, из архива!
//		OPEN_VIEWER - пойман VE_READ, сюда можно попадать только для первой картинки серии
//		(OPEN_VIEWER|OPEN_BY_USER) - F11 из вьювера
//		OPEN_EDITOR - пойман EE_READ, сюда можно попадать только для первой картинки серии
//		(OPEN_EDITOR|OPEN_BY_USER) - F11 из редактора
// asFile
//		Однозначтно заполняется только если (OpenFrom == 0)
//		Иначе - может быть пуст. Функция сама должна получить имя файла.
EFlagsResult EntryPoint(int OpenFrom, LPCWSTR asFile)
{
	WARNING("закрывать вызовы в TRY {} CATCH {}");

	EFlagsResult result = FE_PROCEEDED;
	EFlagsMainProcess r = FEM_EXIT_PLUGIN;
	DecodeParams parms;
	BOOL lbNeedCache;
	CImage *pImage = NULL;

	_ASSERTE(g_Plugin.hWnd == NULL);

	g_Plugin.InitPlugin(); // InitHooks() уже вызван, но InitPlugin его на всякий случай позовет

	g_Plugin.SaveTitle();

	// Для первой картинки серии - установить флаг FW_FIRST_CALL, но не FW_FORCE_DECODE,
	// т.к. сюда мы можем попасть и при перехвате вьювера/редактора, а там FW_FORCE_DECODE не нужен
	//g_Plugin.FlagsWork |= FW_FIRST_CALL; --> eRenderFirstImage
	//if ((OpenFrom & OPEN_BY_USER))
	//	g_Plugin.FlagsWork |= FW_FORCE_DECODE;
	// Сброс недопустимых флагов, могли остаться от предыдущего запуска
	g_Plugin.FlagsWork &= ~FW_TERMINATE;

	parms.Flags = ((OpenFrom & OPEN_BY_USER) ? eRenderForceDecoder : 0) | eRenderFirstImage;

	// Может быть случайно мусор остался?
	g_Panel.CheckAllClosed();
	g_Panel.FreeAllItems();

	// Если открытие НЕ явное - то сначала проверим, поддерживается ли это расширение
	if (!(OpenFrom & OPEN_BY_USER))
	{
		//_ASSERTE((g_Plugin.FlagsWork & FW_FORCE_DECODE) == FW_FORCE_DECODE);
		if (!CPVDManager::IsSupportedExtension(asFile))
		{
			result = FE_UNKNOWN_EXTENSION;
			goto wrap;
		}
	}

	// Инициализация списка файлов
	if (!g_Panel.Initialize(OpenFrom, asFile))
	{
		result = FE_OTHER_ERROR;
		g_Plugin.FlagsWork |= FW_TERMINATE;
		goto wrap;
	}


	// Проверка, может мы работаем в ConEmu?
	_ASSERTE(g_Plugin.hConEmuCtrlPressed==NULL && g_Plugin.hConEmuShiftPressed==NULL);
	CheckConEmu();

	// Проверка валидности родительского окна
	if (!g_Plugin.hConEmuWnd && (!IsWindow(g_Plugin.hFarWnd) || !IsWindowVisible(g_Plugin.hFarWnd)))
	{
		g_StartupInfo.Message(PluginNumberMsg, FMSG_ALLINONE|FMSG_MB_OK,
			NULL,
			(const wchar_t * const *)GetMsg(MIConEmuPluginWarning),
			0, 0);
		result = FE_SKIP_ERROR;
		g_Plugin.FlagsWork |= FW_TERMINATE;
		goto wrap;
	}

	MCHKHEAP;

	// Палитра для преобразования CMYK -> RGB
	g_Plugin.InitCMYK(FALSE); // Дождаться его завершения

	if (g_Plugin.hDisplayThread)
	{
		// Проверим, жив ли поток дисплея
		if (WaitForSingleObject(g_Plugin.hDisplayThread, 0) == WAIT_OBJECT_0)
		{
			CloseHandle(g_Plugin.hDisplayThread); g_Plugin.hDisplayThread = NULL;
		}
		else
		{
			_ASSERTE(g_Plugin.hDisplayThread == NULL);
		}
	}
	if (!g_Plugin.hDisplayThread)
	{
		// Нужно создать
		if (!g_Display.CreateDisplayThread())
		{
			result = FE_OTHER_ERROR;
			goto wrap;
		}
	}

	MCHKHEAP;

	parms.Priority = eCurrentImageCurrentPage;
	parms.nRawIndex = g_Panel.GetActiveRawIdx();
	// RequestDecodedImage - помещает запрос в очередь декодирования
	// GetDecodedImage - дожидается результата декодирования
	pImage = g_Manager.GetDecodedImage(&parms);
	if (!pImage)
	{
		result = FE_OPEN_FAILED;
		RequestTerminate(); // на всякий случай
		goto wrap;
	}

	MCHKHEAP;

	if ((g_Plugin.FlagsWork & FW_TERMINATE) == FW_TERMINATE)
	{
		result = FE_OTHER_ERROR;
		goto wrap;
	}

	MCHKHEAP;

	if (!g_Plugin.hWnd || !IsWindow(g_Plugin.hWnd))
	{
		_ASSERTE(g_Plugin.hWnd && IsWindow(g_Plugin.hWnd));
		result = FE_OTHER_ERROR;
		goto wrap;
	}
	else
	{
		// Если картинку удалось декодировать - окно отрисовщика нужно показать
		if (!IsWindowVisible(g_Plugin.hWnd))
		{
			// Async, т.к. оно создается в другом потоке
			ShowWindowAsync(g_Plugin.hWnd, SW_SHOWNORMAL);
		}
	}
	
	// Сразу поставить в очередь на декодирование следующий фрейм (Priority = eCurrentImageNextPage)!
	if ((pImage->Info.nPages > 1) && ((pImage->Info.nPage + 1) < pImage->Info.nPages))
	{
		WARNING("Поставить в очередь на декодирование следующий фрейм!");
		DecodeParams next;
		next.Priority = eCurrentImageNextPage;
		next.nPage = pImage->Info.nPage+1; // следующий фрейм
		next.nRawIndex = parms.nRawIndex;
		
		// RequestDecodedImage - помещает запрос в очередь декодирования,
		// GetDecodedImage - дожидается результата декодирования
		g_Manager.RequestDecodedImage(&next);
	}
	
	// 
	lbNeedCache = (g_Panel.IsRealNames()) ? g_Plugin.bCachingRP : g_Plugin.bCachingVP;
	if (lbNeedCache)
	{
		DecodeParams next;
		next.Priority = eDecodeNextImage; // приоритет низкий, не текущее изображение
		next.nPage = 0; // первый фрейм
		
		TODO("Заменить на eRenderRelativeIndex & iDirection & nRawIndex");
		
		// RequestDecodedImage - помещает запрос в очередь декодирования,
		// GetDecodedImage - дожидается результата декодирования

		// Следующий файл
		int nNext = next.nRawIndex = g_Panel.GetNextItemRawIdx(1);
		if ((next.nRawIndex != -1) && (next.nRawIndex != parms.nRawIndex))
			g_Manager.RequestDecodedImage(&next);

		// Предыдущий файл
		next.nRawIndex = g_Panel.GetNextItemRawIdx(-1);
		if ((next.nRawIndex != -1) && (next.nRawIndex != parms.nRawIndex) && (next.nRawIndex != nNext))
			g_Manager.RequestDecodedImage(&next);
	}

	MCHKHEAP;

	r = ProcessMainThread();

	if (r == FEM_RETRIEVE_FILE)
	{
		WARNING("нужно запустить на выполнение макрос?");
		_ASSERTE(r != FEM_RETRIEVE_FILE);
		result = FE_RETRIEVE_FILE;
	}
	else
	{
		result = FE_PROCEEDED;
	}

wrap:
	if (result != FE_RETRIEVE_FILE)
	{

		if (!(g_Plugin.FlagsWork & FW_TERMINATE))
		{
			_ASSERTE(g_Plugin.FlagsWork & FW_TERMINATE);
			g_Plugin.FlagsWork |= FW_TERMINATE;
		}
	}

	if ((g_Plugin.FlagsWork & FW_TERMINATE))
	{
		OnFinalTerminate(result);
	}

	return result;
}

void OnFinalTerminate(EFlagsResult result)
{
	RequestTerminate(2);
	_ASSERTE((g_Plugin.FlagsWork & FW_TERMINATE) == FW_TERMINATE);

	g_Plugin.RestoreTitle();

	g_Display.OnTerminate();
	g_Manager.OnTerminate();
	g_Panel.OnTerminate();


	switch (result)
	{
		case FE_PROCEEDED:
			// Если (g_Plugin.FlagsWork & FW_PLUGIN_CALL) то в FAR мог быть послан Ctrl-PgDn
			CFunctionLogger::FunctionLogger(L"OnFinalTerminate.FE_PROCEEDED");
			//hPlugin = (HANDLE)-2; // Чтобы FAR не пытался открыть файл другим плагином
			break;
		case FE_SKIP_ERROR:
			CFunctionLogger::FunctionLogger(L"OnFinalTerminate.FE_SKIP_ERROR");
			break; // Не открыли, но ошибку показывать не нужно
		case FE_UNKNOWN_EXTENSION:
			// Неизвестное расширение
			//if (g_Plugin.FlagsWork & FW_FIRST_CALL) {
			//	g_StartupInfo.Message(PluginNumberMsg, FMSG_ALLINONE|FMSG_MB_OK,
			//		NULL, (const wchar_t * const *)GetMsg(MIOpenUnknownExtension), 0, 0);
			//}
			CFunctionLogger::FunctionLogger(L"OnFinalTerminate.FE_UNKNOWN_EXTENSION");
			break;
		case FE_OPEN_FAILED:
			// Ошибка открытия файла
			CFunctionLogger::FunctionLogger(L"OnFinalTerminate.FE_OPEN_FAILED");
			//if (g_Plugin.FlagsWork & FW_FIRST_CALL)
			{
				g_StartupInfo.Message(PluginNumberMsg, FMSG_ALLINONE|FMSG_MB_OK,
					NULL, (const wchar_t * const *)GetMsg(MIOpenFailedMsg), 0, 0);
			}
			break;
		case FE_NO_PANEL_ITEM:
			// Больше нет элементов на панели?
			CFunctionLogger::FunctionLogger(L"OnFinalTerminate.FE_NO_PANEL_ITEM");
			break;
		case FE_VIEWER_ERROR:
			// Ошибка в ProcessViewerEventW
			CFunctionLogger::FunctionLogger(L"OnFinalTerminate.FE_VIEWER_ERROR");
			break;
	}
}
