// WeaselDeployer.cpp : Defines the entry point for the application.
//
#include "stdafx.h"
#include "WeaselDeployer.h"
import Config;
import WeaselUtility;

#pragma data_seg("Shared")
HWND g_hwnd{ nullptr };
#pragma data_seg()

#pragma comment(linker, "/SECTION:Shared,RWS")

#define WM_NOTIFY_INSTANCE (WM_USER + 100)

CAppModule _Module;

static int Run(LPTSTR lpCmdLine);

void NotifyOtherInstance();

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

	CreateDirectory(WeaselUserDataPath().c_str(), NULL);

	int ret = 0;
	HANDLE hMutex = CreateMutex(NULL, TRUE, L"WeaselDeployerExclusiveMutex");
	if (!hMutex)
	{
		ret = 1;
	}
	else if (GetLastError() == ERROR_ALREADY_EXISTS)
	{
		NotifyOtherInstance();
		ret = 1;
	}
	else
	{
		ret = Run(lpCmdLine);
	}
	
	if (hMutex)
	{
		CloseHandle(hMutex);
	}
	_Module.Term();
	::CoUninitialize();

	return ret;
}

static int Run(LPTSTR lpCmdLine)
{
	Configurator configurator;
	configurator.Initialize();
	bool deployment_scheduled = !wcscmp(L"/deploy", lpCmdLine);
	if (deployment_scheduled)
	{
		return configurator.UpdateWorkspace();
	}

	bool dict_management = !wcscmp(L"/dict", lpCmdLine);
	if (dict_management)
	{
		return configurator.DictManagement();
	}

	bool sync_user_dict = !wcscmp(L"/sync", lpCmdLine);
	if (sync_user_dict)
	{
		return configurator.SyncUserData();
	}

	bool installing = !wcscmp(L"/install", lpCmdLine);
	return configurator.Run(installing);
}

void NotifyOtherInstance()
{
	::PostMessage(g_hwnd, WM_NOTIFY_INSTANCE, ::GetCurrentProcessId(), 0);
	::ShowWindow(g_hwnd, SW_NORMAL);
	::SetForegroundWindow(g_hwnd);
}