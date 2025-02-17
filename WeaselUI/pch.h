// pch.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

//#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers

#include <atlbase.h>
#include <atlwin.h>

#include <wtl/atlapp.h>
#include <wtl/atlframe.h>
#include <wtl/atlgdi.h>
#include <wtl/atlmisc.h>

#include <atlcomcli.h> 

#include <string>
#include <vector>
#include <utility>
#include <array>

#include <format>
#include <regex>
#include <span>
#include <algorithm>
#include <map>
#define GDIPVER 0x0110

#include <d2d1.h>
#include <dwrite_2.h>
#include <wrl/client.h>
#include <atlcomcli.h> 
#include <ShellScalingApi.h>

#if defined _M_IX86
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif