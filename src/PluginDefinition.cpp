//this file is part of notepad++
//Copyright (C)2003 Don HO <donho@altern.org>
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either
//version 2 of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#include "PluginDefinition.h"
#include "menuCmdID.h"
#include "SciMessager.h"
#include "BoyerMooreU.h"
#include "ZHConvertUtils.h"
#include <Shlwapi.h>
//
// The plugin data that Notepad++ needs
//
FuncItem funcItem[nbFunc];

//
// The data of Notepad++ that you can use in your plugin commands
//
NppData nppData;
CSciMessager m_sciMsgr;
HWND curScintilla;
NodeList *strList;

//
// Initialize your plugin data here
// It will be called while plugin loading   
void pluginInit(HANDLE /*hModule*/)
{
	size_t f;
  strList= ReadConvertTable("TradFix.txt",f); //讀取詞彙修正檔
}

//
// Here you can do the clean up, save the parameters (if any) for the next session
//
void pluginCleanUp()
{
	for (size_t i=strList->size()-1;i >0 ;i--)
		delete (*strList)[i];
	delete[] (*strList)[0]->simp;
	strList->clear();
}

//
// Initialization of your plugin commands
// You should fill your plugins commands here
void commandMenuInit()
{

	//--------------------------------------------//
	//-- STEP 3. CUSTOMIZE YOUR PLUGIN COMMANDS --//
	//--------------------------------------------//
	// with function :
	// setCommand(int index,                      // zero based number to indicate the order of command
	//            TCHAR *commandName,             // the command name that you want to see in plugin menu
	//            PFUNCPLUGINCMD functionPointer, // the symbol of function (function pointer) associated with this command. The body should be defined below. See Step 4.
	//            ShortcutKey *shortcut,          // optional. Define a shortcut to trigger this command
	//            bool check0nInit                // optional. Make this menu item be checked visually
	//            );
	//
	setCommand(0, TEXT("簡体->正体"), Simp2Trad, NULL, false);
	setCommand(1, TEXT("正体->簡体"), Trad2Simp, NULL, false);
	setCommand(2, TEXT("打開詞彙修正檔"), OpenTradFix, NULL, false);	
	setCommand(3, TEXT("關於..."), about, NULL, false);		// Get the current scintilla
	int which = -1;
	::SendMessage(nppData._nppHandle, NPPM_GETCURRENTSCINTILLA, 0, (LPARAM)&which);
	if (which == -1)
		return;
	curScintilla = (which == 0)?nppData._scintillaMainHandle:nppData._scintillaSecondHandle;
	m_sciMsgr.setSciWnd(curScintilla);
}

//
// Here you can do the clean up (especially for the shortcut)
//
void commandMenuCleanUp()
{
	// Don't forget to deallocate your shortcut here
}

//
// This function help you to initialize your plugin commands
//
bool setCommand(size_t index, TCHAR *cmdName, PFUNCPLUGINCMD pFunc, ShortcutKey *sk, bool check0nInit) 
{
	if (index >= nbFunc)
		return false;

	if (!pFunc)
		return false;

	lstrcpy(funcItem[index]._itemName, cmdName);
	funcItem[index]._pFunc = pFunc;
	funcItem[index]._init2Check = check0nInit;
	funcItem[index]._pShKey = sk;

	return true;
}

//----------------------------------------------//
//-- STEP 4. DEFINE YOUR ASSOCIATED FUNCTIONS --//
//----------------------------------------------//
int u2w(wchar_t* ws, int wLen, const char* us, int uLen)
{
	int len = MultiByteToWideChar(CP_UTF8, 0, us, uLen, ws, wLen);
	ws[len] = 0;
	return len;
}

int str_unsafe_len( const char* str )
{
	const char* str0 = str;
	while ( *str ) ++str;
	return (int) (str - str0);
}

int w2u(char* us, int uml, const wchar_t* ws, int wl)
{
	int len = WideCharToMultiByte(CP_UTF8, 0, ws, wl, us, uml, NULL, NULL);
	us[len] = 0;
	return len;
}

char* UnicodeToUTF8(const wchar_t* wStr, int wLen, int* L)
{
	if ( !wStr ) return NULL;
	if ( wLen < 0 )
		wLen = lstrlenW(wStr);
	if ( wLen > 0 )
	{
		int uLen = wLen*3;
		char* uStr = new char[uLen + 1];
		if ( uStr )
		{
			uLen = w2u(uStr, uLen, wStr, wLen);
			*L= uLen;
			return uStr;
		}
	}
	return NULL;
}

wchar_t* UTF8ToUnicode(const char* uStr, int uLen, int* L)
{
	if ( !uStr ) return NULL;
	if ( uLen < 0 )
		uLen = str_unsafe_len(uStr);
	if ( uLen > 0 )
	{
		int wLen = uLen;
		wchar_t* wStr = new wchar_t[wLen+ 1];
		if ( wStr )
		{
			wLen = u2w(wStr, wLen, uStr, uLen);
			*L= wLen;
			return wStr;
		}
	}
	return NULL;
}
//正体轉簡体
wchar_t* T2S(const wchar_t* T,int L)
{
	wchar_t* S = new wchar_t[L];
	LCMapString(GetUserDefaultLCID(), LCMAP_SIMPLIFIED_CHINESE, T, L, S, L);
	S[L]=0;
	return S;
}
//簡体轉正体
wchar_t* S2T(const wchar_t* S, int L)
{
	wchar_t* T = new wchar_t[L];
	LCMapString(GetUserDefaultLCID(), LCMAP_TRADITIONAL_CHINESE, S, L, T, L);
	T[L]=0;

	int len;
	wchar_t* pat;wchar_t* rep;
	for (size_t i=0;i < strList->size();i++)
	{
		pat=(*strList)[i]->simp;
		rep=(*strList)[i]->trad;
		len=(*strList)[i]->Len;
		replace(T, L, pat, len, rep);
	}
	return T;
}

void ZHConvert(ConvertPROC cvProc)	
{
	const int nTextLen = m_sciMsgr.getTextLength();
	int nSelLen=0, nLen = 0;
	char* buf = new char[nTextLen + 2];
	if ( buf )
	{
		nSelLen = m_sciMsgr.getSelectionStart()-m_sciMsgr.getSelectionEnd();
		if (nSelLen==0)
			nLen = m_sciMsgr.getText(nTextLen + 2, buf);
		else
			nLen = m_sciMsgr.getSelText(buf);
	}
	int L,M;
	const	wchar_t* T= UTF8ToUnicode(buf, nLen, &L);
	delete[] buf;
	const	wchar_t* S= cvProc(T,L);
	delete[] T;
	char* st = UnicodeToUTF8(S, L, &M);
	//::MessageBox(NULL, S, TEXT("轉換"), MB_OK);
	delete[] S;
	if (nSelLen==0)
		m_sciMsgr.setText(st);
	else
		m_sciMsgr.setSelText(st);
	delete[] st;
}

void Simp2Trad()
{
	ZHConvert(S2T);
}

void Trad2Simp()
{
	ZHConvert(T2S);
}

const wchar_t* STradFix= L"TradFix.txt";

void OpenTradFix()
{
	if (!::PathFileExists(STradFix))
	{
		wchar_t* msg= new wchar_t[100];
		wcscpy(msg,L"找不到詞彙修正檔 ");
		wcscat(msg,STradFix);
		::MessageBox(nppData._nppHandle, msg, L"修正檔不存在", MB_OK);
		delete[] msg;
		return;
	}
	::SendMessage(nppData._nppHandle, NPPM_DOOPEN, 0, (LPARAM)STradFix);
}

void about()
{ 
	wchar_t* msg = new wchar_t[100];
	wcscpy(msg, L"Version: 0.1\r\r");
	wcscat(msg, L"License: GPL\r\r");
	wcscat(msg, L"Author: Ken <pronet25@gmail.com>\r");
	::MessageBox(nppData._nppHandle, msg, L"中文轉換 Plugin", MB_OK);
	delete[] msg;
}