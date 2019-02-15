// BSD 2-Clause License
// 
// Copyright (c) 2019, Ambiesoft
// All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
// 
// * Redistributions of source code must retain the above copyright notice, this
//   list of conditions and the following disclaimer.
// 
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "stdafx.h"

#include "resource.h"
#include "thread.h"
#include "err.h"
#include "common.h"
// #include "systeminfo.h"


#include "../../lsMisc/IsFileExists.h"
#include "../../lsMisc/UrlEncode.h"
#include "../../lsMisc/UTF16toUTF8.h"
#include "../../lsMisc/IsFileOpen.h"
#include "../../lsMisc/showballoon.h"
#include "../../lsMisc/I18N.h"
#include "../../lsMisc/OpenCommon.h"

#include "C:\\Linkout\\CommonDLL\\TimedMessageBox.h"

using namespace Ambiesoft;

wstring myUrlEncode(wstring strIN)
{
	BYTE* pUtf8 = UTF16toUTF8(strIN.c_str());
	stlsoft::scoped_handle<void*> mapUtf8(pUtf8, free);

	char* pUtf8Encoded=NULL;
	// UrlEncode(pUtf8, strlen((char*)pUtf8), &pUtf8Encoded, TRUE);
	pUtf8Encoded= UrlEncode(pUtf8);
	stlsoft::scoped_handle<void*> mapUtf8Encoded(pUtf8Encoded, free);

	wchar_t* pLast = UTF8toUTF16((LPBYTE)pUtf8Encoded);
	stlsoft::scoped_handle<void*> mapLast(pLast, free);
	return pLast;
}

BOOL user32_ShutdownBlockReasonCreate( HWND hwnd, LPCWSTR lpcwstr )
{
  HMODULE hMod = LoadLibrary(_T("user32.dll"));
  if ( hMod == NULL ){
    return FALSE;
  }

  typedef BOOL (WINAPI *myFunc)( HWND, LPCWSTR );
  
  myFunc pFunc = (myFunc)GetProcAddress( hMod, "ShutdownBlockReasonCreate" );
  if ( pFunc == NULL ){
    return FALSE;
  }
  
  BOOL xxx = pFunc( hwnd, lpcwstr );

  FreeLibrary( hMod );

  return xxx; 
}

BOOL user32_ShutdownBlockReasonDestroy( HWND hwnd )
{
  HMODULE hMod = LoadLibrary(_T("user32.dll"));
  if ( hMod == NULL ){
    return FALSE;
  }

  typedef BOOL (WINAPI *myFunc)( HWND );
  
  myFunc pFunc = (myFunc)GetProcAddress( hMod, "ShutdownBlockReasonDestroy" );
  if ( pFunc == NULL ){
    return FALSE;
  }
  
  BOOL xxx = pFunc( hwnd );

  FreeLibrary( hMod );

  return xxx; 
}

BOOL doQueryEndSession(HWND hWnd)
{
	HMODULE hModule = LoadLibrary(_T("TimedMessageBox.dll"));
	if(!hModule)
		return TRUE;

	FNTimedMessageBox func=NULL;
	func = (FNTimedMessageBox)GetProcAddress(hModule, "fnTimedMessageBox");
	if(!func)
		return TRUE;

	if(IDOK != func(hWnd, 300, APP_NAME, NS("message")))
		return FALSE;
	return TRUE; 
}
LRESULT CALLBACK WndProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	static UINT sTaskBarCreated;
	if(nMsg >= 0xC000 && nMsg == sTaskBarCreated)
	{
		tstring traytip = NS("watching");
		traytip += _T(" ");
		traytip += g_workfile.c_str();

		

		if(!AddTrayIcon(
			g_hWnd, 
			WM_APP_TRAY_NOTIFY, 
			g_hTrayIcon, 
			traytip.c_str()))
		{
			errExit(NS("could not register tray icon."),GetLastError());
		}
		return 0;
	}

	static bool btimerprocessing;
	switch(nMsg)
	{
		case WM_CREATE:
		{
			// user32_ShutdownBlockReasonCreate(hWnd,L"FJOD");
			sTaskBarCreated = RegisterWindowMessage(_T("TaskbarCreated"));

//			EnableDebugPriv();
			SetTimer(hWnd, 1, 5000,NULL);
		}
		break;
		
		case WM_TIMER:
		{
			if(btimerprocessing)
				break;
			btimerprocessing = true;
			if(g_hKanshiApp)
			{
				DWORD dw =WaitForSingleObject(g_hKanshiApp, 1);
				if(dw == WAIT_OBJECT_0)
				{// app is gone
					g_hKanshiApp = NULL;
					if(IsFileOpen(g_workfile.c_str()))
					{
						wstring title=L"kocchiwork";
						wstring text = NS("App was closed but file still opens, changed mode to monitor file");

/**
						wchar_t file[MAX_PATH];
						GetModuleFileName(NULL, file, MAX_PATH);
						*wcsrchr(file, L'\\') = 0;
						lstrcat(file, L"\\showballoon.exe");

						HICON hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON_MAIN));
						showballoon(g_hWnd,title ,text, g_hTrayIcon,WM_APP_TRAY_NOTIFY,TRUE);
**/


						if(!PopupTrayIcon(
							g_hWnd, 
							WM_APP_TRAY_NOTIFY, 
							g_hTrayIcon, 
							text.c_str()))
						{
							errExit(NS("could not register tray icon."),GetLastError());
						}




						//wstring param;
						//param += L"/title:";
						//param += myUrlEncode(title);
						//param += L" /icon:kocchiwork.exe";
						//param += _T(" \"") + myUrlEncode(message) + _T("\"");						

						//ShellExecute(NULL,
						//	NULL,
						//	file,
						//	param.c_str(),
						//	NULL,
						//	SW_SHOW);
						
						//SetForegroundWindow(hWnd);
						//MessageBoxA(hWnd,
						 	//"sss",
							//"sss",
							//0);
						// KillTimer(hWnd,1);
					}
					else
					{ // no one opens file
						int mret = IDNO;
						if( (time(NULL)-g_starttime) < 10)
						{
							SetForegroundWindow(hWnd);
							mret = MessageBox(hWnd,
								NS("App closed. Do you want to continue monitor file?"),
								L"kocchiwork",
								MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON2);
						}

						if(mret==IDYES)
						{//use file monitor
							KillTimer(hWnd,1);
						}
						else
						{
							//quit
							KillTimer(hWnd,1);
							doPostQuitMessage(0);
						}
					}
				}
			}
			else
			{// no kanshiapp
				static int count;				
				if(!IsFileOpen(g_workfile.c_str()))
				{
					count++;
					if(count > 1)
					{
						ReturnFileAndQuit(hWnd);
						KillTimer(hWnd,1);
						doPostQuitMessage(0);
					}
				}
				else
				{
					count=0;
				}
			}
			btimerprocessing = false;
		}
		break;
		
		case WM_APP_TRAY_NOTIFY:
		switch(lParam)
		{
			case WM_LBUTTONUP:
			{
				SetForegroundWindow(hWnd);
			}
			break;

			case WM_RBUTTONUP:
			{
				POINT apos;
				HMENU hSubMenu = CreatePopupMenu();
#ifdef _DEBUG
				AppendMenu(hSubMenu, MF_BYCOMMAND, IDC_START, L"TEST");
				AppendMenu(hSubMenu, MF_SEPARATOR, 0, NULL);
#endif
				AppendMenu(hSubMenu, MF_BYCOMMAND, IDC_ABOUT, (L"&About..."));
				AppendMenu(hSubMenu, MF_BYCOMMAND, IDC_SWHOWHELP, (L"&Help..."));
				AppendMenu(hSubMenu, MF_SEPARATOR, 0, NULL);

				AppendMenu(hSubMenu, MF_BYCOMMAND, IDC_REOPENFILE, (L"&Reopen"));
				AppendMenu(hSubMenu, MF_BYCOMMAND, IDC_OPENWITHEXPLORER, (L"&Open Local File Location"));
				AppendMenu(hSubMenu, MF_BYCOMMAND, IDC_OPENREMOTE_WITHEXPLOER, (L"Open Re&mote File Location"));
				AppendMenu(hSubMenu, MF_SEPARATOR, 0, NULL);
				
				AppendMenu(hSubMenu, MF_BYCOMMAND, IDC_QUIT, (L"&Exit"));

				Ambiesoft::i18nChangeMenuText(hSubMenu);

				SetForegroundWindow(hWnd);
				GetCursorPos((LPPOINT)&apos);

				TrackPopupMenu(hSubMenu,
					TPM_TOPALIGN | TPM_RIGHTBUTTON | TPM_LEFTBUTTON,
					apos.x, apos.y, 0, hWnd, NULL);

				DestroyMenu(hSubMenu);
			}
			break;

		}
		break;
	
		case WM_APP_LACHANGED:
		{
			if(!g_hKanshiApp)
				ReturnFileAndQuit(hWnd);
		}
		break;
	
		case WM_APP_APPKANHIDONE:
		{
			doPostQuitMessage(0);
		}
		break;
	
		case WM_QUERYENDSESSION:
		{
			g_bQuerying = TRUE;
			user32_ShutdownBlockReasonCreate(hWnd, L"kocchiwork");
			BOOL ret = doQueryEndSession(hWnd);
			user32_ShutdownBlockReasonDestroy(hWnd);
			g_bQuerying = FALSE;
			if(g_bQuittedWhileQuerying)
			{
				PostQuitMessage(0);
			}
			return ret;
		}
		break;

		case WM_COMMAND:
		switch(LOWORD(wParam))
		{
			case IDC_ABOUT:
			{
				tstring message;
				message += APP_NAME L" " APP_VERSION;
				message += _T("\r\n\r\n");
				message += _T("Watching\t");
				message += g_workfile;
				message += _T("\r\n");
				message += _T("Remote\t");
				message += g_remotefile;
				MessageBox(hWnd, 
					message.c_str(),
					APP_NAME, 
					MB_ICONINFORMATION);
			}
			break;
	
			case IDC_SWHOWHELP:
			{
				if (_wcsicmp(Ambiesoft::i18nGetCurrentLang(), L"jpn") == 0)
				{
					OpenCommon(hWnd, L"https://github.com/ambiesoft/kocchiwork/blob/master/README-JP.md");
				}
				else
				{ 
					OpenCommon(hWnd, L"https://github.com/ambiesoft/kocchiwork/blob/master/README.md");
				}
			}
			break;

			case IDC_QUIT:
			{
				DestroyWindow(hWnd);
				doPostQuitMessage(0);
			}
			break;

			case IDC_REOPENFILE:
			{
				SHELLEXECUTEINFO sei = {0};
				sei.cbSize = sizeof(sei);
				sei.lpVerb = NULL;
				sei.lpFile = g_workfile.c_str();
				sei.nShow = SW_SHOW;
				tstring dir = GetDirFromPath(g_workfile.c_str());
				sei.lpDirectory = dir.c_str();
				if(!ShellExecuteEx(&sei)) 
				{
					MessageBox(hWnd,
						NS("Failed to ShellExecuteEx"),
						APP_NAME,
						MB_ICONERROR);
				}
			}
			break;

			case IDC_OPENWITHEXPLORER:
			{
				OpenFolder(hWnd, g_workfile.c_str());
			}
			break;
			
			case IDC_OPENREMOTE_WITHEXPLOER:
			{
				OpenFolder(hWnd, g_remotefile.c_str());
			}
			break;
		}
		break;

	default:
		return DefWindowProc(hWnd, nMsg, wParam, lParam);
	}

	return 0;
}
