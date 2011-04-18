// BlockEditor.cpp : Defines the entry point for the DLL application.
//

#include <windows.h>
#include <TCHAR.h>
#include "BlockEditorRes.h"

#define _FAR_NO_NAMELESS_UNIONS

#ifdef _UNICODE
	#if FAR_UNICODE>=1906
		#include "common/far3/pluginW3.hpp"
	#else
		#include "common/unicode/pluginW.hpp"
	#endif
#else
	#include "common/ascii/pluginA.hpp"
#endif

#include <malloc.h>
#include <TChar.h>

#ifdef MDEBUG
//#include "/VCProject/MLib/MLibDef.h"
#endif


#ifndef MDEBUG
#define MCHKHEAP
#endif

#define InvalidOp()

HMODULE ghInstance=NULL;

#if defined(__GNUC__)
extern "C"{
  BOOL WINAPI DllMain( HANDLE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved );
  void WINAPI SetStartupInfoW(struct PluginStartupInfo *Info);
  void WINAPI GetPluginInfoW( struct PluginInfo *Info );
#if FAR_UNICODE>=1906
  void WINAPI GetGlobalInfoW(struct GlobalInfo *Info);
#else
  int  WINAPI GetMinFarVersionW ();
#endif
  HANDLE WINAPI OpenPluginW(int OpenFrom,INT_PTR Item);
};
#endif


BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
                     )
{
    if (ghInstance==NULL)
        ghInstance = (HMODULE)hModule;
    return TRUE;
}


DWORD gdwFarVersion = MAKEFARVERSION(1,70,70);
PluginStartupInfo psi;

void WINAPI SetStartupInfoW(struct PluginStartupInfo *Info)
{
	memset(&psi, 0, sizeof(psi));
	memmove(&psi, Info, Info->StructSize);
  
	#if FAR_UNICODE>=1906
		error
	#else
	gdwFarVersion = (DWORD)psi.AdvControl(psi.ModuleNumber, ACTL_GETFARVERSION, NULL);
	#endif
}

#if FAR_UNICODE>=1906

#else
int WINAPI GetMinFarVersionW()
{
  #ifdef _UNICODE
  return MAKEFARVERSION(2,0,1017);
  #else
  return MAKEFARVERSION(1,70,146);
  #endif
}
#endif



void WINAPI GetPluginInfoW( struct PluginInfo *Info )
{
    memset(Info, 0, sizeof(PluginInfo));

    MCHKHEAP;

    static TCHAR *szMenu[1];
#ifdef _UNICODE
    int nLen = lstrlenA(szMsgBlockEditorPlugin)+1;
    MCHKHEAP;
    szMenu[0]=(WCHAR*)malloc(nLen*2);
    MultiByteToWideChar(0,0,szMsgBlockEditorPlugin,nLen,szMenu[0],nLen);
    MCHKHEAP;
#else
    szMenu[0]=szMsgBlockEditorPlugin;
#endif
    MCHKHEAP;

    Info->Flags = PF_DISABLEPANELS | PF_EDITOR;
    Info->PluginMenuStrings = szMenu;
    Info->PluginMenuStringsNumber = 1;
#ifdef _UNICODE
	Info->Reserved = 1296198763; //'MBlk';
#endif
}

enum EWorkMode
{
	ewmUndefined = 0,
	ewmFirst = 1,
	ewmTabulateRight = 1,
	ewmTabulateLeft = 2,
	ewmCommentFirst = 3,
	ewmCommentAuto = ewmCommentFirst,
	ewmUncommentAuto = 4,
	ewmCommentBlock = 5,
	ewmCommentStream = 6,
	ewmCommentLast = ewmCommentStream,
	ewmLastValidCall = ewmCommentLast,
	// Use Internally
	ewmUncommentBlock = 7,
	ewmUncommentStream = 8,
	ewmLastInternal = ewmUncommentStream
};

HANDLE WINAPI OpenPluginW(int OpenFrom,INT_PTR Item)
{
    
    EditorGetString egs;
    EditorInfo ei = {0};
    EditorSetString ess;
    EditorSelect es;
	int nMode = ewmUndefined;

    MCHKHEAP;

    psi.EditorControl(ECTL_GETINFO,&ei);
    //if (ei.BlockType == BTYPE_NONE)
    //    return INVALID_HANDLE_VALUE;
    es.BlockType = ei.BlockType;

    //TODO: Если выделения нет, и хотят Tab/ShiftTab можно переслать в ФАР
    //ACTL_POSTKEYSEQUENCE TAB/требуемое количество BS


    MCHKHEAP;
    //CStaticMenu lmMenu(_ATOT(szMsgBlockEditorPlugin));

    /*lmMenu.AddMenuItem ( _T("&1 Tabulate right") );
    lmMenu.AddMenuItem ( _T("&2 Tabulate left") );
    lmMenu.AddMenuItem ( _T("&3 Comment") );
    lmMenu.AddMenuItem ( _T("&4 UnComment") );*/
    //psi.Menu(

#ifdef _UNICODE
	if (((OpenFrom & OPEN_FROMMACRO) == OPEN_FROMMACRO) && (Item >= ewmFirst && Item <= ewmLastValidCall))
	{
		nMode = (int)Item;
	}
#endif


	if (nMode == ewmUndefined)
	{
		FarMenuItem pItems[] =
		{
			{_T("&1 Tabulate right"),0,0,0},
			{_T("&2 Tabulate left"),0,0,0},
			{_T("&3 Comment auto"),0,0,0},
			{_T("&4 UnComment"),0,0,0},
			{_T("&5 Comment block"),0,0,0},
			{_T("&6 Comment stream"),0,0,0}
		};

		MCHKHEAP;

		//int nMode = lmMenu.DoModal();
		nMode = psi.Menu ( psi.ModuleNumber,
			-1, -1, 0, FMENU_WRAPMODE,
			_T(szMsgBlockEditorPlugin),NULL,NULL,
			0, NULL, pItems, ARRAYSIZE(pItems) );
		if (nMode < 0)
			return INVALID_HANDLE_VALUE;
		nMode++;
	}
    MCHKHEAP;

    if (nMode < ewmFirst || nMode > ewmLastValidCall)
        return INVALID_HANDLE_VALUE;

	#ifdef _UNICODE
	EditorUndoRedo eur = {EUR_BEGIN};
	psi.EditorControl(ECTL_UNDOREDO,&eur);

	int nLen = psi.EditorControl(ECTL_GETFILENAME,NULL);
	wchar_t *pszFileName = (wchar_t*)calloc(nLen+1,2);
	psi.EditorControl(ECTL_GETFILENAME,pszFileName);
	#else
	LPCSTR pszFileName = ei.FileName;
	#endif

	BOOL lbSkipNonSpace = TRUE;
    LPCTSTR psComment = _T("//"), psCommentBegin = NULL, psCommentEnd = NULL;
	int nCommentBeginLen = 0, nCommentEndLen = 0;
	TCHAR szComment[100] = {0}, szCommentBegin[100] = {0}, szCommentEnd[100] = {0}, szTemp[16] = {0};
    if ((nMode >= ewmCommentFirst && nMode <= ewmCommentLast) && pszFileName)
	{
    	// CommentFromBegin
    	HKEY hk = 0;
		TCHAR szKey[MAX_PATH];
		lstrcpy(szKey, psi.RootKey); 
		if (lstrlen(szKey) < (MAX_PATH - 15))
		{
			lstrcat(szKey, _T("\\MBlockEditor"));

			//[HKEY_CURRENT_USER\Software\Far2\Plugins\MBlockEditor]
			//;; value "off" means 'comments are inserted at first non-space symbol'
			//;; value "on"  means '... inserted at the line beginning'
			//"CommentFromBegin"="off"
			if (RegOpenKeyEx(HKEY_CURRENT_USER, szKey, 0, KEY_READ, &hk) == 0)
			{
				DWORD dwSize;
				if (RegQueryValueEx(hk, _T("CommentFromBegin"), NULL, NULL, (LPBYTE)szTemp, &(dwSize=sizeof(szTemp))) == 0)
				{
					if (szTemp[0] && dwSize < sizeof(szTemp))
					{
						lbSkipNonSpace = lstrcmpi(szTemp, _T("on")) != 0;
					}
				}
				RegCloseKey(hk); hk = NULL;
			}
		}
    
        LPCTSTR psExt = _tcsrchr(pszFileName, _T('.'));
        if (psExt)
		{
			if (lstrlen(szKey) < (MAX_PATH - 2 - lstrlen(psExt)))
			{
				lstrcat(szKey, _T("\\"));
				lstrcat(szKey, psExt);
				
				if (RegOpenKeyEx(HKEY_CURRENT_USER, szKey, 0, KEY_READ, &hk) == 0)
				{
					DWORD dwSize, dwSize2;
					// May be overrided for each extension
					if (RegQueryValueEx(hk, _T("CommentFromBegin"), NULL, NULL, (LPBYTE)szTemp, &(dwSize=sizeof(szTemp))) == 0)
					{
						if (szTemp[0] && dwSize < sizeof(szTemp))
						{
							lbSkipNonSpace = lstrcmpi(szTemp, _T("on")) != 0;
						}
					}
					// Настройка "блочного" комментирования (на каждой строке)
					if (RegQueryValueEx(hk, _T("Comment"), NULL, NULL, (LPBYTE)szComment, &(dwSize=sizeof(szComment))) == 0)
					{
						// Чтобы можно было запретить "блочный" комментарий (для html например)
						// нужно указать "Comment"=""
						if (/*szComment[0] &&*/ dwSize < sizeof(szComment))
						{
							psComment = szComment;
							// Имеет приоритет над встроенным в программу
							psExt = NULL;
						}
					}
					// Комментирование может быть и "потоковое" (начало и конец выделения)
					if (!RegQueryValueEx(hk, _T("CommentBegin"), NULL, NULL, (LPBYTE)szCommentBegin, &(dwSize=sizeof(szCommentBegin))) &&
						!RegQueryValueEx(hk, _T("CommentEnd"), NULL, NULL, (LPBYTE)szCommentEnd, &(dwSize2=sizeof(szCommentEnd))))
					{
						if (szCommentBegin[0] && dwSize < sizeof(szCommentBegin)
							&& szCommentEnd[0] && dwSize2 < sizeof(szCommentEnd))
						{
							psCommentBegin = szCommentBegin;
							psCommentEnd = szCommentEnd;
							// Имеет приоритет над встроенным в программу
							psExt = NULL;
						}
					}
					RegCloseKey(hk); hk = NULL;
				}
			}
		}			

		if (psExt)
		{
            if (_tcsicmp(psExt, _T(".bat"))==0 || _tcsicmp(psExt, _T(".cmd"))==0)
			{
                psComment = _T("rem ");
            }
			else if (_tcsicmp(psExt, _T(".sql"))==0)
			{
                psComment = _T("--");
            }
			else if (_tcsicmp(psExt, _T(".bas"))==0 || _tcsicmp(psExt, _T(".vbs"))==0)
			{
                psComment = _T("'");
			}
			else if (_tcsicmp(psExt, _T(".cpp"))==0 || _tcsicmp(psExt, _T(".c"))==0 || _tcsicmp(psExt, _T(".cxx"))==0 ||
				_tcsicmp(psExt, _T(".hpp"))==0 || _tcsicmp(psExt, _T(".h"))==0 || _tcsicmp(psExt, _T(".hxx"))==0)
			{
				psCommentBegin = _T("/*");
				psCommentEnd = _T("*/");
			}
			else if (_tcsicmp(psExt, _T(".htm"))==0 || _tcsicmp(psExt, _T(".html"))==0 || _tcsicmp(psExt, _T(".xml"))==0)
			{
				psCommentBegin = _T("<!--");
				psCommentEnd = _T("-->");
				psComment = _T("");
			}
        }
    }


	#ifdef _UNICODE
	if (pszFileName)
	{
		free(pszFileName); pszFileName = NULL;
	}
	#endif


    if (ei.TabSize<1) ei.TabSize=1;
    BOOL lbExpandTabs = (ei.Options & (EOPT_EXPANDALLTABS|EOPT_EXPANDONLYNEWTABS))!=0;

    MCHKHEAP;

    int nStartLine = (ei.BlockType == BTYPE_NONE) ? ei.CurLine : ei.BlockStartLine;
    int nEndLine = nStartLine;
    //int nEmptyLine = -1;
    int Y1 = nStartLine, Y2 = -1;
    int X1 = 0, X2 = -1;
	bool lbCurLineShifted = false;
    int nInsertSlash = -1;
    int nMaxStrLen = 0;
	bool lbStartChanged = true, lbEndChanged = true; // измнены ли первая и последняя строки выделения?

    MCHKHEAP;

	// Пробежаться по выделению, определить максимальную длину строки
    for (egs.StringNumber = nStartLine;
         egs.StringNumber < ei.TotalLines;
         egs.StringNumber++)
    {
        psi.EditorControl(ECTL_GETSTRING,&egs);

        MCHKHEAP;

        if (egs.StringNumber==nStartLine)
		{
            X1 = egs.SelStart;
        }

        if (egs.SelEnd!=-1 && X2==-1)
		{
            X2 = egs.SelEnd;
            Y2 = nEndLine;
        }

        MCHKHEAP;

        if (egs.StringLength>nMaxStrLen)
            nMaxStrLen = egs.StringLength;

        if ((ei.BlockType != BTYPE_NONE) && (egs.SelStart==-1))
		{
            break;
        }
		else if ((ei.BlockType == BTYPE_NONE) || 
            (egs.SelStart>0) || (egs.SelStart==0 && (egs.SelEnd==-1 || egs.SelEnd>egs.SelStart))) 
        {
            MCHKHEAP;
            nEndLine = egs.StringNumber;
            //if (nEmptyLine!=-1) nEmptyLine=-1;
            if (nMode == ewmCommentBlock || nMode == ewmCommentAuto)
			{
                int nPos = 0, nIdx=0; BOOL lbSpace=FALSE;
                while ((lbSpace=(egs.StringText[nIdx]==_T(' '))) || egs.StringText[nIdx]==_T('\t'))
				{
                    nIdx++;
                    if (lbSpace)
                        nPos++;
                    else
                        nPos += ei.TabSize;
                }
                if (nPos || (nInsertSlash==-1))
				{
                    if (nInsertSlash==-1)
                        nInsertSlash = nPos;
                    else if (nInsertSlash>nPos)
                        nInsertSlash = nPos;
                }
            }
            MCHKHEAP;

            if (ei.BlockType == BTYPE_NONE)
                break;

        }
		else
		{
            //nEmptyLine = egs.StringNumber;
            break;
        }
    }
    //if (Y2==-1)
    Y2 = nEndLine;
    if (X2==0) Y2++;

	// Скорректировать X1/X2 если они выходят ЗА пределы строки
	if (X1 > 0)
	{
		egs.StringNumber = Y1;
		if (psi.EditorControl(ECTL_GETSTRING,&egs) && X1 > egs.StringLength)
			X1 = egs.StringLength;
	}
	if (X2 > 0)
	{
		egs.StringNumber = Y2;
		if (psi.EditorControl(ECTL_GETSTRING,&egs) && X2 > egs.StringLength)
			X2 = egs.StringLength;
	}

    MCHKHEAP;

	if (nMode == ewmCommentAuto)
	{
		if (psCommentBegin && *psCommentBegin
			&& psCommentEnd && *psCommentEnd 
			&& ((X1 >= 0 && X2 >= 0) && (X1 || X2)))
		{
			// Если есть из чего выбирать - проверить, а не захватывает ли X2 всю строку?
			if (!X1 && X2 && psComment && *psComment)
			{
				egs.StringNumber = Y2;
		        if (psi.EditorControl(ECTL_GETSTRING,&egs)
					&& egs.StringLength == X2)
				{
					// Раз захватывает всю строку целиком - выбираем "блочное" комментирование
					nMode = ewmCommentBlock;
				}
			}
			if (nMode == ewmCommentAuto)
				nMode = ewmCommentStream;
		}
		else if ((nMode == ewmCommentAuto) && (ei.BlockType == BTYPE_NONE)
			&& (!psComment || !*psComment))
		{
			egs.StringNumber = ei.CurLine;
			if (psi.EditorControl(ECTL_GETSTRING,&egs) && egs.StringLength)
			{
				X1 = 0;
				X2 = egs.StringLength;
				if (lbSkipNonSpace)
				{
					while (X1 < X2 && X1 < ei.CurPos &&
						(egs.StringText[X1] == _T(' ') || egs.StringText[X1] == _T('\t')))
						X1++;
				}
				Y1 = Y2 = ei.CurLine;
				nMode = ewmCommentStream;
				ei.BlockType = BTYPE_STREAM;
			}
		}
		
		if (nMode != ewmCommentStream)
			nMode = ewmCommentBlock;
	}
	if (nMode == ewmCommentStream)
	{
		if (!psCommentBegin || !psCommentEnd)
		{
			nMode = ewmCommentBlock;
		}
		else
		{
			nCommentBeginLen = lstrlen(psCommentBegin);
			nCommentEndLen = lstrlen(psCommentEnd);
		}
	}
	if (nMode == ewmCommentBlock)
	{
		nCommentBeginLen = nCommentEndLen = lstrlen(psComment);
	}
	if ((nMode == ewmCommentBlock) && (nMode == ewmCommentStream))
	{
		if (nCommentBeginLen || nCommentEndLen)
			return INVALID_HANDLE_VALUE;
	}


    nMaxStrLen += 10+ei.TabSize; // с запасом на символы комментирования
	if (psCommentBegin && psCommentEnd)
		nMaxStrLen += lstrlen(psCommentBegin) + lstrlen(psCommentEnd);
	if (psComment)
		nMaxStrLen += lstrlen(psComment);

    MCHKHEAP;

    if (ei.BlockType == BTYPE_NONE)
        X1 = ei.CurPos;


	// Поехали
    TCHAR* lsText = (TCHAR*)calloc(nMaxStrLen,sizeof(TCHAR));
	bool lbFirstUncommented = false;
    MCHKHEAP;
    for (egs.StringNumber = nStartLine;
         egs.StringNumber <= nEndLine;
         egs.StringNumber++)
    {
        MCHKHEAP;
        psi.EditorControl(ECTL_GETSTRING,&egs);
        
        if (nMode == ewmTabulateRight)
		{
            MCHKHEAP;
            if (*egs.StringText==0) continue;
            if (lbExpandTabs)
			{
                //lsText.Fill(_T(' '), ei.TabSize);
                for(int i=0; i<ei.TabSize; i++) lsText[i]=_T(' ');
                lsText[ei.TabSize]=0;
            }
			else
			{
                //lsText = _T("\t");
                lsText[0]=_T('\t'); lsText[1]=0;
            }
            MCHKHEAP;
            //lsText += egs.StringText;
            lstrcat(lsText, egs.StringText);
            egs.StringText = lsText;

			if (egs.StringNumber == ei.CurLine)
				lbCurLineShifted = true;

        }
		else if (nMode == ewmTabulateLeft)
		{
            if (*egs.StringText==0) continue;
            MCHKHEAP;
            if (*egs.StringText==_T('\t'))
			{
                egs.StringText++;
                if (egs.StringNumber==nStartLine)
                    if (X1) X1--;
                if (egs.StringNumber==nEndLine)
                    if (X2) X2--;
                MCHKHEAP;
				if (egs.StringNumber == ei.CurLine)
					lbCurLineShifted = true;
            }
			else if (*egs.StringText==_T(' '))
			{
                int nSpaces = 0;
                while (nSpaces<ei.TabSize && *egs.StringText==_T(' '))
				{
                    egs.StringText++; nSpaces++;
                }
                MCHKHEAP;
                if (egs.StringNumber==nStartLine)
                    X1 = (X1>nSpaces) ? (X1-nSpaces) : 0;
                if (egs.StringNumber==nEndLine)
                    X2 = (X2>nSpaces) ? (X2-nSpaces) : 0;
				if (egs.StringNumber == ei.CurLine)
					lbCurLineShifted = true;
            }
            //lstrcpy(lsText, egs.StringText);

        }
		else if (nMode == ewmCommentBlock || nMode == ewmCommentStream)
		{
            MCHKHEAP;
            int nPos = 0, nIdx = 0; BOOL lbSpace=FALSE;
            while (lbSkipNonSpace
            	   && ((lbSpace=(egs.StringText[nIdx]==_T(' '))) || egs.StringText[nIdx]==_T('\t')))
            {
                if (nPos >= nInsertSlash)
					break;
                nIdx++;
                if (lbSpace)
                    nPos++;
                else
                    nPos += ei.TabSize;
            }

            MCHKHEAP;

			if (nMode == ewmCommentBlock ||
				(nMode == ewmCommentStream
				 && ((egs.StringNumber == nStartLine) || (egs.StringNumber == nEndLine))))
			{
				LPCTSTR psCommCurr = NULL;
				if (nMode == ewmCommentBlock)
					psCommCurr = psComment;
				else
					psCommCurr = (egs.StringNumber == nStartLine) ? psCommentBegin : psCommentEnd;

				//// nIdx - пропуск пробельных символов
				//if (egs.StringText[nIdx])
				//{
					// Если НЕ конец строки
					//lsText = egs.StringText;
					//lsText.Insert(nIdx,_T("//"));

				lsText[0] = 0;
				MCHKHEAP;
				if (nMode == ewmCommentBlock)
				{
					//if ((nMode == ewmCommentBlock) || (egs.StringNumber == nStartLine))
					//{
					if (nIdx) // сначала скопировать пробельные символы
						memcpy(lsText, egs.StringText, nIdx*sizeof(TCHAR));
					MCHKHEAP;
					lstrcpy(lsText+nIdx, psCommCurr);
					MCHKHEAP;
					lstrcat(lsText, egs.StringText+nIdx);
					//}
					//else
					//{
					//	lstrcpy(lsText, egs.StringText);
					//	lstrcat(lsText, psCommCurr);
					//}
				}
				else if (nMode == ewmCommentStream)
				{
					size_t n = 0;
					if (egs.StringNumber == nStartLine)
					{
						if (X1)
						{
							memcpy(lsText+n, egs.StringText, X1*sizeof(TCHAR));
							n += X1;
						}
						memcpy(lsText+n, psCommentBegin, nCommentBeginLen*sizeof(TCHAR));
						n += nCommentBeginLen;
						lsText[n] = 0;
					}
					if (egs.StringNumber == nEndLine)
					{
						if (egs.StringNumber == nStartLine)
						{
							if (X2 <= X1)
							{
								InvalidOp();
								break;
							}
							memcpy(lsText+n, egs.StringText+X1, (X2 - X1)*sizeof(TCHAR));
							n += (X2 - X1);
						}
						else
						{
							if ((Y2 == nEndLine) && (X2 > 0))
							{
								memcpy(lsText+n, egs.StringText, X2*sizeof(TCHAR));
								n += X2;
							}
							else
							{
								lstrcpy(lsText+n, egs.StringText);
								n += lstrlen(egs.StringText);
							}
						}
						memcpy(lsText+n, psCommentEnd, nCommentEndLen*sizeof(TCHAR));
						n += nCommentEndLen;
						lsText[n] = 0;
						if ((Y2 == nEndLine) && egs.StringText[X2])
							lstrcpy(lsText+n, egs.StringText+X2);
					}
					else
					{
						if (egs.StringNumber == nStartLine)
							lstrcpy(lsText+n, egs.StringText+X1);
						else
							lstrcpy(lsText+n, egs.StringText);
					}
					
				}
				MCHKHEAP;
				//}
				//else
				//{
				//	//nPos = lsText.Find(_T("//"));
				//	//if(nPos==-1) nPos=0; else nPos+=2;
				//	//lsText.SetAt(nPos,0);

				//	TCHAR* psComm = _tcsstr(lsText, psCommCurr);
				//	if (psComm==NULL) psComm=lsText; else psComm+=lstrlen(psCommCurr);
				//	MCHKHEAP;
				//	*psComm =0;
				//	MCHKHEAP;
				//}
				egs.StringText = lsText;

				// Коррекция будущего выделения
				if (egs.StringNumber == nStartLine && nMode != ewmCommentStream)
				{
					if ((X1 > nIdx) && (X1 || Y1==Y2))
						X1 += nCommentBeginLen;
				}
				if (egs.StringNumber == nEndLine) // БЕЗ else - может быть однострочное выделение
				{
					if (X2 || Y1==Y2)
						X2 += nCommentEndLen;
					if (nMode == ewmCommentStream && Y1==Y2)
						X2 += nCommentBeginLen;
				}
				if (egs.StringNumber == ei.CurLine)
					lbCurLineShifted = true;
			}
        }
		else if (nMode == ewmUncommentAuto || nMode == ewmUncommentBlock || nMode == ewmUncommentStream)
		{
			// Убрать комментарий
            if (*egs.StringText==0)
			{
				if (egs.StringNumber == nStartLine)
					lbStartChanged = false;
				else if (egs.StringNumber == nEndLine)
					lbEndChanged = false;
				continue;
			}
            //lsText = egs.StringText;
            //int nSlash = lsText.Find(_T("//"));
            //if (nSlash>=0) lsText.Delete(nSlash,2);

			if (nMode == ewmUncommentAuto)
			{
				if (psComment && *psComment && !psCommentBegin)
					nMode = ewmUncommentBlock;
				else if (psCommentBegin && psCommentEnd && !(psComment && *psComment))
					nMode = ewmUncommentStream;
				else
				{
					// Нужно опеределить, какой комментарий используется в блоке :(
					LPCTSTR pszTest = egs.StringText;
					if (egs.StringNumber == nStartLine && X1 >= 0)
						pszTest += X1;
					while (*pszTest == _T(' ') || *pszTest == _T('\t'))
						pszTest++;
					if (_tcsncmp(pszTest, psCommentBegin, lstrlen(psCommentBegin)) == 0)
						nMode = ewmUncommentStream;
					else if ((pszTest - egs.StringText) >= lstrlen(psCommentBegin)
						&& _tcsncmp(pszTest-lstrlen(psCommentBegin), psCommentBegin, lstrlen(psCommentBegin)) == 0)
						nMode = ewmUncommentStream;
					else
						nMode = ewmUncommentBlock;
				}
				if (nMode == ewmUncommentStream)
				{
					nCommentBeginLen = lstrlen(psCommentBegin);
					nCommentEndLen = lstrlen(psCommentEnd);
				}
				else
				{
					nCommentBeginLen = nCommentEndLen = lstrlen(psComment);
				}
			}

			if (nMode == ewmUncommentBlock ||
				(nMode == ewmUncommentStream
				 && (!lbFirstUncommented || (egs.StringNumber == nEndLine))))
			{
				MCHKHEAP;
				LPCTSTR psComm = NULL, psCommCurr = NULL;
				if (nMode == ewmUncommentBlock)
				{
					psCommCurr = psComment;
				}
				else
				{
					psCommCurr = lbFirstUncommented ? psCommentEnd : psCommentBegin;

					if (egs.StringNumber == nStartLine)
					{
						LPCTSTR pszTest = egs.StringText;
						if (egs.StringNumber == nStartLine && X1 >= 0)
							pszTest += X1;
						while (*pszTest == _T(' ') || *pszTest == _T('\t'))
							pszTest++;
						if (_tcsncmp(pszTest, psCommentBegin, nCommentBeginLen) == 0)
							psComm = pszTest;
						else if ((pszTest - egs.StringText) >= nCommentBeginLen
							&& _tcsncmp(pszTest-nCommentBeginLen, psCommentBegin, nCommentBeginLen) == 0)
							psComm = pszTest-nCommentBeginLen;
					}
				}
				
				if (!psComm)
					psComm = _tcsstr(egs.StringText, psCommCurr);

				if (!psComm)
				{
					if (egs.StringNumber == nStartLine)
						lbStartChanged = false;
					if (egs.StringNumber == nEndLine)
						lbEndChanged = false;
				}
				else
				{
					MCHKHEAP;
					if (nMode == ewmUncommentBlock)
					{
						// Коррекция будущего выделения
						if (egs.StringNumber == nStartLine)
						{
							if ((X1 > (psComm - egs.StringText)) && (X1 || Y1==Y2))
								X1 -= lstrlen(psCommCurr);
						}
						if (egs.StringNumber == nEndLine) // БЕЗ else - может быть однострочное выделение
						{
							if (X2 || Y1==Y2)
								X2 -= lstrlen(psCommCurr);
						}

						if (psComm > egs.StringText)
						{
							// Комментарий НЕ с начала строки
							memcpy(lsText, egs.StringText, (psComm-egs.StringText)*sizeof(TCHAR));
							lstrcpy(lsText+(psComm-egs.StringText), psComm+lstrlen(psCommCurr));
							egs.StringText = lsText;
						}
						else
						{
							// Комментарий с начала строки - просто "отбросить" кусок (передвинуть указатель)
							egs.StringText += lstrlen(psCommCurr);
						}
					}
					else
					{
						size_t n = 0;
						LPCTSTR pszNextPart = egs.StringText;
						if (!lbFirstUncommented && (psComm >= egs.StringText))
						{
							if (psComm > egs.StringText)
							{
								memcpy(lsText+n, egs.StringText, (psComm-egs.StringText)*sizeof(TCHAR));
								n += (psComm-egs.StringText);
							}
							pszNextPart = psComm + lstrlen(psCommCurr);
						}
						if (egs.StringNumber == nEndLine)
						{
							if (lbFirstUncommented)
								psComm = egs.StringText;
							else
								psComm += nCommentBeginLen;
							// нужно отбросить и закрывающий коментатор
							LPCTSTR pszEndComm = egs.StringText + X2;
							if (_tcsncmp(pszEndComm, psCommentEnd, nCommentEndLen) == 0)
							{
								// OK, выделение НЕ включало закрывающий коментатор
							}
							else if ((pszEndComm - psComm) >= nCommentEndLen
								&& _tcsncmp(pszEndComm-nCommentEndLen, psCommentEnd, nCommentEndLen) == 0)
							{
								// выделение включало закрывающий коментатор
								pszEndComm -= nCommentEndLen;
							}
							else
							{
								// Просто найти в строке закрывающий комментатор
								if (!lbFirstUncommented && psComm)
								{
									// Если в этой строке убирали "открывающий комментатор" - искать нужно после него
									pszEndComm = _tcsstr(psComm+nCommentBeginLen, psCommentEnd);
								}
								else
								{
									pszEndComm = _tcsstr(egs.StringText, psCommentEnd);
								}
							}
							if (pszEndComm)
							{
								memcpy(lsText+n, psComm, (pszEndComm-psComm)*sizeof(TCHAR));
								n += (pszEndComm-psComm);
								lstrcpy(lsText+n, pszEndComm+nCommentEndLen);
							}
							else
							{
								// Просто докопировать остаток (чтобы не потерять ничего)
								lstrcpy(lsText+n, pszNextPart);
							}
						}
						else
						{
							lstrcpy(lsText+n, pszNextPart);
						}

						// Коррекция будущего выделения
						if (egs.StringNumber == nStartLine)
						{
							if ((X1 > (psComm - egs.StringText)) && (X1 || Y1==Y2))
								X1 -= nCommentBeginLen;
						}
						if (egs.StringNumber == nEndLine) // БЕЗ else - может быть однострочное выделение
						{
							if (X2 || Y1==Y2)
								X2 -= nCommentEndLen;
							if (Y1 == Y2)
								X2 -= nCommentBeginLen;
						}

						egs.StringText = lsText;
					}
					MCHKHEAP;

					if (egs.StringNumber == ei.CurLine)
						lbCurLineShifted = true;
					if (!lbFirstUncommented)
						lbFirstUncommented = true;
				}
			}
        }


        MCHKHEAP;
		ess.StringLength = lstrlen(egs.StringText);
		ess.StringText = (TCHAR*)egs.StringText;
		ess.StringNumber = egs.StringNumber;
		ess.StringEOL = (TCHAR*)egs.StringEOL;
		psi.EditorControl(ECTL_SETSTRING,&ess);
        MCHKHEAP;
    }

	if (X1<0)
	{
		//_ASSERTE(X1>=0);
		X1 = 0;
	}
	if (X2<0)
	{
		//_ASSERTE(X2>=0);
		X2 = 0;
	}

	if (nMode == ewmTabulateRight)
	{
		int nSize = lbExpandTabs ? ei.TabSize : 1;
		if (X1 || Y1==Y2) X1 += nSize;
		if (X2 || Y1==Y2) X2 += nSize;
	}
	else if (nMode == ewmTabulateLeft)
	{
		int nSize = lbExpandTabs ? ei.TabSize : 1;
		// X1&X2 корректируются выше, при обработке ewmTabulateLeft
	}
	//else if (nMode == ewmComment)
	//{
	//	int nCmtLen = lstrlen(psComment);
	//	//if (X1 || Y1==Y2) X1 += nCmtLen;
	//	if (X2 || Y1==Y2) X2 += nCmtLen;
	//}
	//else if (nMode == ewmUncomment)
	//{
	//	int nCmtLen = lstrlen(psComment);
	//	//if (lbStartChanged && (X1 || Y1==Y2))
	//	//	X1 = (X1>nCmtLen) ? (X1-nCmtLen) : 0;
	//	if (lbEndChanged && (X2 || Y1==Y2))
	//		X2 = (X2>nCmtLen) ? (X2-nCmtLen) : 0;
	//}

    MCHKHEAP;
	
	if (nMode <= ewmLastInternal && lbCurLineShifted)
	{
		EditorSetPosition esp;
		esp.CurLine = -1;
		esp.CurPos = esp.CurTabPos = -1;
		esp.TopScreenLine = -1;
		esp.LeftPos = -1;
		switch (nMode)
		{
		case ewmTabulateRight:
			esp.CurTabPos = ei.CurTabPos+ei.TabSize;
			break;
		case ewmTabulateLeft:
			esp.CurTabPos = max(0,ei.CurTabPos-ei.TabSize);
			break;
		case ewmCommentBlock:
			//WARNING: для потоковых комментариев - доработать
			esp.CurPos = ei.CurPos+lstrlen(psComment);
			break;
		case ewmCommentStream:
			//WARNING: для потоковых комментариев - доработать
			//esp.CurPos = ei.CurPos+lstrlen(psCommentBegin);
			if ((Y1 == Y2) && (X2 == (ei.CurPos+nCommentBeginLen+nCommentEndLen)))
				esp.CurPos = ei.CurPos + nCommentBeginLen + nCommentEndLen;
			else if (Y1 == ei.CurLine)
				esp.CurPos = ei.CurPos + nCommentBeginLen;
			else if (Y2 == ei.CurLine)
				esp.CurPos = ei.CurPos + nCommentEndLen;
			else
				esp.CurPos = ei.CurPos;
			break;
		case ewmUncommentBlock:
			//WARNING: для потоковых комментариев - доработать
			esp.CurPos = max(0,ei.CurPos-lstrlen(psComment));
			break;
		case ewmUncommentStream:
			//WARNING: для потоковых комментариев - доработать
			//esp.CurPos = max(0,ei.CurPos-lstrlen(psCommentBegin));
			if ((Y1 == Y2) && (ei.CurPos >= (X2+nCommentBeginLen+nCommentEndLen)))
				esp.CurPos = ei.CurPos - nCommentBeginLen - nCommentEndLen;
			else if (Y1 == ei.CurLine)
			{
				if (ei.CurPos >= X1)
					esp.CurPos = ei.CurPos - nCommentBeginLen;
				else
					esp.CurPos = ei.CurPos;
			}
			else if (Y2 == ei.CurLine)
				esp.CurPos = ei.CurPos - nCommentEndLen;
			else
				esp.CurPos = ei.CurPos;
			if (esp.CurPos < 0)
				esp.CurPos = 0;
			break;
		default:
			esp.CurTabPos = ei.CurTabPos;
		}

		if (esp.CurPos > 0)
		{
			egs.StringNumber = ei.CurLine;
			if (psi.EditorControl(ECTL_GETSTRING,&egs) && esp.CurPos > egs.StringLength)
				esp.CurPos = egs.StringLength;
		}

		esp.Overtype=-1;
		
		//TODO: сдвиг курсора при обычной табуляции
		psi.EditorControl(ECTL_SETPOSITION,&esp);
	}

	if (nMode <= ewmLastInternal && es.BlockType != BTYPE_NONE)
    {
		if (es.BlockType==BTYPE_STREAM)
		{
			es.BlockStartLine = min(Y2,Y1);
			es.BlockStartPos = (Y1 == Y2) ? (min(X1,X2)) : ((Y1 < Y2) ? X1 : X2);

			// небольшая коррекция, если позиции равны
			if(X1 == X2)
				es.BlockStartPos += (Y1 < Y2) ? 1: -1;

			es.BlockHeight = max(Y1,Y2)-min(Y1,Y2)+1;

			if (Y1 < Y2)
				es.BlockWidth = X2-X1/*+1*/;
			else if (Y1 > Y2)
				es.BlockWidth = X1-X2+1;
			else if (Y1 == Y2)
				es.BlockWidth = max(X1,X2)-min(X1,X2);

			if(X1 == X2)
			{
				if(Y1 < Y2)
					es.BlockStartPos--;
				else
					es.BlockStartPos++;
			}
		}
		else
		{
			//#ifndef _UNICODE
			// В FAR 1.7x build 2479 возникала проблема если
			// TAB не заменяется проблами и идет выделение верт.блока
			// gdwFarVersion == 0x09af014b
			if (!lbExpandTabs)
			{
				EditorConvertPos ecp;
				ecp.StringNumber = Y1; ecp.SrcPos = ecp.DestPos = X1;
				if (psi.EditorControl(ECTL_REALTOTAB, &ecp))
					X1 = ecp.DestPos;
				ecp.SrcPos = ecp.DestPos = X2;
				if (psi.EditorControl(ECTL_REALTOTAB, &ecp))
					X2 = ecp.DestPos;
			}
			//#endif

			es.BlockStartLine=min(Y2,Y1);
			es.BlockStartPos=(Y1==Y2) ? (min(X1,X2)) : (Y1 < Y2?X1:X2);
			
			// небольшая коррекция, если позиции равны
			if(X1 == X2)
				es.BlockStartPos+=(Y1 < Y2?1:-1);
			
			es.BlockHeight=max(Y1,Y2)-min(Y1,Y2)+1;
			
			if(Y1 < Y2)
				es.BlockWidth=X2-X1/*+1*/;
			else if(Y1 > Y2)
				es.BlockWidth=X1-X2+1;
			else if (Y1 == Y2)
				es.BlockWidth=max(X1,X2)-min(X1,X2);
			
			if(X1 == X2)
			{
				if(Y1 < Y2)
					es.BlockStartPos--;
				else
					es.BlockStartPos++;
			}
		}

        MCHKHEAP;
        psi.EditorControl(ECTL_SELECT,(void*)&es);
    }

	#ifdef _UNICODE
	eur.Command = EUR_END;
	psi.EditorControl(ECTL_UNDOREDO,&eur);
	#endif

    MCHKHEAP;

    return INVALID_HANDLE_VALUE;
}
