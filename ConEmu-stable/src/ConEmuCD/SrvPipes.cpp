
/*
Copyright (c) 2009-2012 Maximus5
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

#include "ConEmuC.h"
#include "Queue.h"

#define DEBUGSTRINPUTPIPE(s) //DEBUGSTR(s) // ConEmuC: Recieved key... / ConEmuC: Recieved input

// Forwards
DWORD WINAPI GetDataThread(LPVOID lpvParam);


// Helpers
bool InputServerStart()
{
	if (!gpSrv || !*gpSrv->szInputname)
	{
		_ASSERTE(gpSrv!=NULL && *gpSrv->szInputname);
		return false;
	}

	// Запустить нить обработки событий (клавиатура, мышь, и пр.)
	gpSrv->hInputPipeThread = CreateThread(NULL, 0, InputPipeThread, NULL, 0, &gpSrv->dwInputPipeThreadId);

	if (gpSrv->hInputPipeThread == NULL)
	{
		return false;
	}
	
	SetThreadPriority(gpSrv->hInputPipeThread, THREAD_PRIORITY_ABOVE_NORMAL);
	
	return true;
}

bool CmdServerStart()
{
	if (!gpSrv || !*gpSrv->szPipename)
	{
		_ASSERTE(gpSrv!=NULL && *gpSrv->szPipename);
		return false;
	}

	// Запустить нить обработки команд
	gpSrv->hServerThread = CreateThread(NULL, 0, ServerThread, NULL, 0, &gpSrv->dwServerThreadId);

	if (gpSrv->hServerThread == NULL)
	{
		return false;
	}
	
	return true;
}

bool DataServerStart()
{
	if (!gpSrv || !*gpSrv->szGetDataPipe)
	{
		_ASSERTE(gpSrv!=NULL && *gpSrv->szGetDataPipe);
		return false;
	}

	// Запустить нить обработки событий (клавиатура, мышь, и пр.)
	gpSrv->hGetDataPipeThread = CreateThread(NULL, 0, GetDataThread, NULL, 0, &gpSrv->dwGetDataPipeThreadId);

	if (gpSrv->hGetDataPipeThread == NULL)
	{
		return false;
	}

	SetThreadPriority(gpSrv->hGetDataPipeThread, THREAD_PRIORITY_ABOVE_NORMAL);
	
	return true;
}


// Bodies
DWORD WINAPI GetDataThread(LPVOID lpvParam)
{
	BOOL fConnected, fSuccess;
	DWORD dwErr = 0;

	while (!gbQuit)
	{
		MCHKHEAP;
		gpSrv->hGetDataPipe = CreateNamedPipe(
		                       gpSrv->szGetDataPipe,        // pipe name
		                       PIPE_ACCESS_DUPLEX,       // goes from client to server only
		                       PIPE_TYPE_MESSAGE |       // message type pipe
		                       PIPE_READMODE_MESSAGE |   // message-read mode
		                       PIPE_WAIT,                // blocking mode
		                       PIPE_UNLIMITED_INSTANCES, // max. instances
		                       DATAPIPEBUFSIZE,          // output buffer size
		                       PIPEBUFSIZE,              // input buffer size
		                       0,                        // client time-out
		                       gpLocalSecurity);          // default security attribute

		if (gpSrv->hGetDataPipe == INVALID_HANDLE_VALUE)
		{
			dwErr = GetLastError();
			_ASSERTE(gpSrv->hGetDataPipe != INVALID_HANDLE_VALUE);
			_printf("CreatePipe failed, ErrCode=0x%08X\n", dwErr);
			Sleep(50);
			//return 99;
			continue;
		}

		// Wait for the client to connect; if it succeeds,
		// the function returns a nonzero value. If the function
		// returns zero, GetLastError returns ERROR_PIPE_CONNECTED.
		fConnected = ConnectNamedPipe(gpSrv->hGetDataPipe, NULL) ?
		             TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);
		MCHKHEAP;

		if (fConnected)
		{
			//TODO:
			DWORD cbBytesRead = 0, cbBytesWritten = 0, cbWrite = 0;
			CESERVER_REQ_HDR Command = {0};

			while(!gbQuit
			        && (fSuccess = ReadFile(gpSrv->hGetDataPipe, &Command, sizeof(Command), &cbBytesRead, NULL)) != FALSE)         // not overlapped I/O
			{
				// предусмотреть возможность завершения нити
				if (gbQuit)
					break;

				if (Command.nVersion != CESERVER_REQ_VER)
				{
					_ASSERTE(Command.nVersion == CESERVER_REQ_VER);
					break; // переоткрыть пайп!
				}

				if (Command.nCmd != CECMD_CONSOLEDATA)
				{
					_ASSERTE(Command.nCmd == CECMD_CONSOLEDATA);
					break; // переоткрыть пайп!
				}

				if (gpSrv->pConsole->bDataChanged == FALSE)
				{
					cbWrite = sizeof(gpSrv->pConsole->info);
					ExecutePrepareCmd(&(gpSrv->pConsole->info.cmd), Command.nCmd, cbWrite);
					gpSrv->pConsole->info.nDataShift = 0;
					gpSrv->pConsole->info.nDataCount = 0;
					fSuccess = WriteFile(gpSrv->hGetDataPipe, &(gpSrv->pConsole->info), cbWrite, &cbBytesWritten, NULL);
				}
				else //if (Command.nCmd == CECMD_CONSOLEDATA)
				{
					_ASSERTE(Command.nCmd == CECMD_CONSOLEDATA);
					gpSrv->pConsole->bDataChanged = FALSE;
					cbWrite = gpSrv->pConsole->info.crWindow.X * gpSrv->pConsole->info.crWindow.Y;

					// Такого быть не должно, ReadConsoleData корректирует возможный размер
					if ((int)cbWrite > (gpSrv->pConsole->info.crMaxSize.X * gpSrv->pConsole->info.crMaxSize.Y))
					{
						_ASSERTE((int)cbWrite <= (gpSrv->pConsole->info.crMaxSize.X * gpSrv->pConsole->info.crMaxSize.Y));
						cbWrite = (gpSrv->pConsole->info.crMaxSize.X * gpSrv->pConsole->info.crMaxSize.Y);
					}

					gpSrv->pConsole->info.nDataShift = (DWORD)(((LPBYTE)gpSrv->pConsole->data) - ((LPBYTE)&(gpSrv->pConsole->info)));
					gpSrv->pConsole->info.nDataCount = cbWrite;
					DWORD nHdrSize = sizeof(gpSrv->pConsole->info);
					cbWrite = cbWrite * sizeof(CHAR_INFO) + nHdrSize ;
					ExecutePrepareCmd(&(gpSrv->pConsole->info.cmd), Command.nCmd, cbWrite);
					fSuccess = WriteFile(gpSrv->hGetDataPipe, &(gpSrv->pConsole->info), cbWrite, &cbBytesWritten, NULL);
				}

				// Next query
			}

			// Выход из пайпа (ошибки чтения и пр.). Пайп будет пересоздан, если не gbQuit
			SafeCloseHandle(gpSrv->hInputPipe);
		}
		else
			// The client could not connect, so close the pipe. Пайп будет пересоздан, если не gbQuit
			SafeCloseHandle(gpSrv->hInputPipe);
	}

	MCHKHEAP;
	return 1;
}
