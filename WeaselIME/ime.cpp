﻿#include "stdafx.h"
#include "WeaselIME.h"

#include "test.h"
#ifdef TEST
#define WEASEL_ENABLE_LOGGING
#include "logging.h"
namespace fs = std::filesystem;
#endif // TEST

#pragma warning(disable: 4996)

static BOOL g_is_winlogon = FALSE;

//
// IME export functions
//

BOOL WINAPI ImeInquire(IMEINFO* lpIMEInfo, LPWSTR lpszUIClass, DWORD dwSystemInfoFlags)
{
	if (!lpIMEInfo || !lpszUIClass)
		return FALSE;

	if (dwSystemInfoFlags & IME_SYSINFO_WINLOGON)
	{
		// disable input method in winlogon.exe
		g_is_winlogon = TRUE;
	}

	wcscpy(lpszUIClass, WeaselIME::GetUIClassName());

	lpIMEInfo->dwPrivateDataSize = 0;
	lpIMEInfo->fdwProperty = IME_PROP_UNICODE | IME_PROP_SPECIAL_UI;
	lpIMEInfo->fdwConversionCaps = IME_CMODE_FULLSHAPE | IME_CMODE_NATIVE;
	lpIMEInfo->fdwSentenceCaps = IME_SMODE_NONE;
	lpIMEInfo->fdwUICaps = UI_CAP_2700;
	lpIMEInfo->fdwSCSCaps = 0;
	lpIMEInfo->fdwSelectCaps = SELECT_CAP_CONVERSION;

	return TRUE;
}

BOOL WINAPI ImeConfigure(HKL hKL, HWND hWnd, DWORD dwMode, LPVOID lpData)
{
	if (g_is_winlogon) return TRUE;

	// TODO:
	MessageBox(hWnd, L"本品无设定界面 :)", L"输入法设定", MB_OK);
	return TRUE;
}

DWORD WINAPI ImeConversionList(HIMC hIMC, LPCTSTR lpSource, LPCANDIDATELIST lpCandList, DWORD dwBufLen, UINT uFlag)
{
#ifdef TEST
	LOG(INFO) << std::format("ImeConversionList.");
#endif // TEST
	//
	return TRUE;
}

BOOL WINAPI ImeDestroy(UINT uForce)
{
#ifdef TEST
	LOG(INFO) << std::format("ImeDestroy. uForce = 0x{:X}", (unsigned)uForce);
#endif // TEST
	//
	return TRUE;
}

LRESULT WINAPI ImeEscape(HIMC hIMC, UINT uSubFunc, LPVOID lpData)
{
	//
	return TRUE;
}

BOOL WINAPI ImeProcessKey(HIMC hIMC, UINT vKey, LPARAM lKeyData, const LPBYTE lpbKeyState)
{
	if (g_is_winlogon) return FALSE;

	BOOL accepted = FALSE;
	std::shared_ptr<WeaselIME> p = WeaselIME::GetInstance(hIMC);
    if (!p)
        return FALSE;
	accepted = p->ProcessKeyEvent(vKey, lKeyData, lpbKeyState);
	return accepted;
}

BOOL WINAPI ImeSelect(HIMC hIMC, BOOL fSelect)
{
	if (g_is_winlogon) return TRUE;

	std::shared_ptr<WeaselIME> p = WeaselIME::GetInstance(hIMC);
    if (!p)
        return FALSE;
	HRESULT hr = p->OnIMESelect(fSelect);
	if (FAILED(hr))
		return FALSE;

	return TRUE;
}

BOOL WINAPI ImeSetActiveContext(HIMC hIMC, BOOL fFocus)
{
	if (g_is_winlogon) return TRUE;

	if (hIMC)
	{	
		std::shared_ptr<WeaselIME> p = WeaselIME::GetInstance(hIMC);
        if (!p)
            return FALSE;
		HRESULT hr = p->OnIMEFocus(fFocus);
        if (FAILED(hr))
			return FALSE;
	}

	return TRUE;
}

UINT WINAPI ImeToAsciiEx (UINT uVKey, UINT uScanCode, CONST LPBYTE lpbKeyState, LPDWORD lpdwTransKey, UINT fuState, HIMC hIMC)
{
	//
	return 0;
}

BOOL WINAPI NotifyIME(HIMC hIMC, DWORD dwAction, DWORD dwIndex, DWORD dwValue)
{
#ifdef TEST
	LOG(INFO) << std::format("NotifyIME. dwAction = 0x{:X}", (unsigned)dwAction);
#endif // TEST
	switch (dwAction)
	{
	case IMN_CLOSESTATUSWINDOW:
		break;

	case IMN_CHANGECANDIDATE:
#ifdef TEST
		LOG(INFO) << std::format("NotifyIME. dwAction = IMN_CHANGECANDIDATE");
#endif // TEST
		break;

	case IMN_CLOSECANDIDATE:
		break;	

	case IMN_OPENCANDIDATE:
		break;
	}
	IMN_CHANGECANDIDATE;
	IMN_CLOSECANDIDATE;
	IMN_CLOSESTATUSWINDOW;
	IMN_GUIDELINE;
	IMN_OPENCANDIDATE;
	IMN_OPENSTATUSWINDOW;
	IMN_SETCANDIDATEPOS;
	IMN_SETCOMPOSITIONFONT;
	IMN_SETCOMPOSITIONWINDOW;
	IMN_SETCONVERSIONMODE;
	IMN_SETOPENSTATUS;
	IMN_SETSENTENCEMODE;
	IMN_SETSTATUSWINDOWPOS;
	//
	return TRUE;
}
BOOL WINAPI ImeRegisterWord(LPCTSTR lpRead, DWORD dw, LPCTSTR lpStr)
{
	//
	return FALSE;
}

BOOL WINAPI ImeUnregisterWord(LPCTSTR lpRead, DWORD dw, LPCTSTR lpStr)
{
	//
	return FALSE;
}

UINT WINAPI ImeGetRegisterWordStyle(UINT nItem, LPSTYLEBUF lp)
{
	//
	return 0;
}

UINT WINAPI ImeEnumRegisterWord(REGISTERWORDENUMPROC lpfn, LPCTSTR lpRead, DWORD dw, LPCTSTR lpStr, LPVOID lpData)
{
	//
	return 0;
}

BOOL WINAPI ImeSetCompositionString(HIMC hIMC, DWORD dwIndex, LPCVOID lpComp, DWORD dwComp, LPCVOID lpRead, DWORD dwRead)
{
	//
	return FALSE;
}
