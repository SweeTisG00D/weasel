﻿// WeaselSetup.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "WeaselSetup.h"
#include "InstallOptionsDialog.h"
#include <format>
import  RimeWithWeasel;
import WeaselIPC;

CAppModule _Module;

static int Run(LPTSTR lpCmdLine);
void ResetServer();

int APIENTRY _tWinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPTSTR    lpCmdLine,
	int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);

	HRESULT hRes = ::CoInitialize(NULL);
	// If you are running on NT 4.0 or higher you can use the following call instead to
	// make the EXE free threaded. This means that calls come in on a random RPC thread.
	//HRESULT hRes = ::CoInitializeEx(NULL, COINIT_MULTITHREADED);
	ATLASSERT(SUCCEEDED(hRes));

	// this resolves ATL window thunking problem when Microsoft Layer for Unicode (MSLU) is used
	::DefWindowProc(NULL, 0, 0, 0L);

	AtlInitCommonControls(ICC_BAR_CLASSES);	// add flags to support other controls

	hRes = _Module.Init(NULL, hInstance);
	ATLASSERT(SUCCEEDED(hRes));

	int ret = Run(lpCmdLine);

	_Module.Term();
	::CoUninitialize();

	return ret;
}

int install(bool hant, bool silent);
int uninstall(bool silent, bool ime = false);
bool has_installed(bool& ime);

static int CustomInstall(bool installing)
{
	bool hant = false;
	bool silent = false;
	std::wstring user_dir;
	bool installed{};

	const WCHAR KEY[] = L"Software\\Rime\\Weasel";
	HKEY hKey;
	LSTATUS ret = RegOpenKey(HKEY_CURRENT_USER, KEY, &hKey);
	if (ret == ERROR_SUCCESS)
	{
		WCHAR value[MAX_PATH];
		DWORD len = sizeof(value);
		DWORD type = 0;
		DWORD data = 0;
		ret = RegQueryValueEx(hKey, L"RimeUserDir", NULL, &type, (LPBYTE)value, &len);
		if (ret == ERROR_SUCCESS && type == REG_SZ)
		{
			user_dir = value;
		}
		len = sizeof(data);
		ret = RegQueryValueEx(hKey, L"Hant", NULL, &type, (LPBYTE)&data, &len);
		if (ret == ERROR_SUCCESS && type == REG_DWORD)
		{
			hant = (data != 0);
			if (installing)
				silent = true;
		}
		RegCloseKey(hKey);
	}

	if (!silent)
	{
		InstallOptionsDialog dlg;
		dlg.installed = installed = has_installed(dlg.ime);
		dlg.hant = hant;
		dlg.user_dir = user_dir;
		if (IDOK != dlg.DoModal())
		{
			if (!installing)
				return 1;  // aborted by user
		}
		else
		{
			hant = dlg.hant;
			user_dir = dlg.user_dir;
		}
	}
	if (!installed && 0 != install(hant, silent))
		return 1;

	ret = RegCreateKeyEx(HKEY_CURRENT_USER, KEY,
		0, NULL, 0, KEY_ALL_ACCESS, 0, &hKey, NULL);
	if (FAILED(HRESULT_FROM_WIN32(ret)))
	{
		MessageBox(NULL, KEY, L"安装失败", MB_ICONERROR | MB_OK);
		return 1;
	}

	ret = RegSetValueEx(hKey, L"RimeUserDir", 0, REG_SZ,
		(const BYTE*)user_dir.c_str(),
		(user_dir.length() + 1) * sizeof(WCHAR));
	if (FAILED(HRESULT_FROM_WIN32(ret)))
	{
		MessageBox(NULL, L"无法写入 RimeUserDir", L"安装失败", MB_ICONERROR | MB_OK);
		return 1;
	}

	if (installed)
	{
		ResetServer();
		return 0;
	}

	DWORD data = hant ? 1 : 0;
	ret = RegSetValueEx(hKey, L"Hant", 0, REG_DWORD, (const BYTE*)&data, sizeof(DWORD));
	if (FAILED(HRESULT_FROM_WIN32(ret)))
	{
		MessageBox(NULL, L"无法写入 Hant", L"安装失败", MB_ICONERROR | MB_OK);
		return 1;
	}	

	return 0;
}

static int Run(LPTSTR lpCmdLine)
{
	const bool silent = true;
	bool uninstalling = !wcscmp(L"/u", lpCmdLine);
	bool ime;
	if (uninstalling)
	{
		has_installed(ime);
		return uninstall(silent, ime);
	}		
	bool hans = !wcscmp(L"/s", lpCmdLine);
	if (hans)
		return install(false, silent);
	bool hant = !wcscmp(L"/t", lpCmdLine);
	if (hant)
		return install(true, silent);
	bool installing = !wcscmp(L"/i", lpCmdLine);
	return CustomInstall(installing);
}

void ResetServer()
{
	weasel::Client client;
	if (client.Connect())
	{
		client.ShutdownServer();
	}

	if ((size_t)ShellExecute(nullptr, L"Open", L"WeaselServer.exe", nullptr, L".\\", SW_SHOWDEFAULT) < 33)
	{
		MessageBox(nullptr, std::format(L"L错误代码：{}", GetLastError()).data(), L"算法服务启动失败", MB_OK);
	}
}