module;
#include "stdafx.h"
#include "Globals.h"
#include "ctffunc.h"
#include <format>
#include <shellapi.h>
#include "resource.h"
// #include "test.h"
#ifdef TEST
#define WEASEL_ENABLE_LOGGING
#include "logging.h"
#endif // TEST
module WeaselTSF;
import LanguageBar;
import CandidateList;
import Compartment;
import WeaselUtility;

bool WeaselTSF::execute(std::wstring_view cmd, std::wstring_view args)
{
	return (int)ShellExecuteW(nullptr, NULL, cmd.data(), args.data(), NULL, SW_SHOWNORMAL) > 32;
}

bool WeaselTSF::explore(std::wstring_view path)
{
	return (int)ShellExecuteW(nullptr, L"open", L"explorer", path.data(), NULL, SW_SHOWNORMAL) > 32;
}

void WeaselTSF::_HandleLangBarMenuSelect(UINT wID)
{
	bool ret{};
	switch (wID)
	{
	case ID_WEASELTRAY_SETTINGS:
		ret = execute(std::format(LR"({}\WeaselDeployer.exe)", WeaselRootPath()));
		if (!ret)
			goto DEFAULT;
		break;

	case ID_WEASELTRAY_DICT_MANAGEMENT:
		ret = execute(std::format(LR"({}\WeaselDeployer.exe)", WeaselRootPath()), LR"(/dict)");
		if (!ret)
			goto DEFAULT;
		break;

	case ID_WEASELTRAY_USERCONFIG:
		ret = explore(WeaselUserDataPath());
		if (!ret)
			goto DEFAULT;
		break;

	case ID_WEASELTRAY_INSTALLDIR:
		ret = explore(WeaselRootPath());
		if (!ret)
			goto DEFAULT;
		break;

	case ID_WEASELTRAY_DAEMON_ENABLE:
	{
		Flip(WeaselFlag::DAEMON_ENABLE);
		UpdateGlobalCompartment();
#ifdef TEST
		LOG(INFO) << std::format("From WeaselTSF::_HandleLangBarMenuSelect. hr = {:#x}", (size_t)hr);
#endif // TEST
	}
	break;

	case ID_WEASELTRAY_HALF_SHAPE:
		ResetBit(WeaselFlag::FULL_SHAPE);
		goto DEFAULT;

	case ID_WEASELTRAY_FULL_SHAPE:
		SetBit(WeaselFlag::FULL_SHAPE);
		goto DEFAULT;

	case ID_STYLE_CARET_FOLLOWING:
		_bitset.flip(static_cast<int>(WeaselFlag::CARET_FOLLOWING));
		_cand->SetCaretFollowing(GetBit(WeaselFlag::CARET_FOLLOWING));
		break;

	case ID_STYLE_PRESERVED_KEY_SWITCH:
		Flip(WeaselFlag::PRESERVED_KEY_SWITCH);
		UpdateGlobalCompartment();
		if (GetBit(WeaselFlag::PRESERVED_KEY_SWITCH))
			_InitPreservedKey();
		else
			_UninitPreservedKey();
		break;

DEFAULT:
	default:
		m_client.TrayCommand(wID);
	}
}

HWND WeaselTSF::_GetFocusedContextWindow()
{
	HWND hwnd = NULL;
	ITfDocumentMgr* pDocMgr;
	if (_pThreadMgr->GetFocus(&pDocMgr) == S_OK && pDocMgr != NULL)
	{
		ITfContext* pContext;
		if (pDocMgr->GetTop(&pContext) == S_OK && pContext != NULL)
		{
			ITfContextView* pContextView;
			if (pContext->GetActiveView(&pContextView) == S_OK && pContextView != NULL)
			{
				pContextView->GetWnd(&hwnd);
				pContextView->Release();
			}
			pContext->Release();
		}
		pDocMgr->Release();
	}

	if (hwnd == NULL)
	{
		HWND hwndForeground = GetForegroundWindow();
		if (GetWindowThreadProcessId(hwndForeground, NULL) == GetCurrentThreadId())
			hwnd = hwndForeground;
	}

	return hwnd;
}

BOOL WeaselTSF::_InitLanguageBar()
{
	com_ptr<ITfLangBarItemMgr> pLangBarItemMgr;
	BOOL fRet = FALSE;

	if (_pThreadMgr->QueryInterface(&pLangBarItemMgr) != S_OK)
		return FALSE;

	if ((_pLangBarButton = new CLangBarItemButton(*this, GUID_LBI_INPUTMODE, _cand->style())) == NULL)
		return FALSE;

	if (pLangBarItemMgr->AddItem(_pLangBarButton) != S_OK)
	{
		_pLangBarButton = NULL;
		return FALSE;
	}

	_pLangBarButton->Show(TRUE);
	fRet = TRUE;

	return fRet;
}

void WeaselTSF::_UninitLanguageBar()
{
	com_ptr<ITfLangBarItemMgr> pLangBarItemMgr;

	if (_pLangBarButton == NULL)
		return;

	if (_pThreadMgr->QueryInterface(&pLangBarItemMgr) == S_OK)
	{
		pLangBarItemMgr->RemoveItem(_pLangBarButton);
	}

	_pLangBarButton = NULL;
}

bool WeaselTSF::_UpdateLanguageBar()
{
	bool state{};
	if (!_pLangBarButton) return state;

	DWORD flags{};
	_pCompartmentConversion->_GetCompartmentDWORD(flags);
#ifdef TEST
	LOG(INFO) << std::format("From WeaselTSF::_UpdateLanguageBar. flags = {:#x}, ascii_mode = {}, full_shape = {}, ascii_punct = {}, _InitInputMethodState = {}", flags, stat.ascii_mode, stat.full_shape, stat.ascii_punct, !GetBit(4));
#endif // TEST

	if (!GetBit(WeaselFlag::INIT_INPUT_METHOD_STATE))
	{
		SetBit(WeaselFlag::INIT_INPUT_METHOD_STATE);
		SetBit(WeaselFlag::ASCII_MODE, _status.ascii_mode);
		SetBit(WeaselFlag::FULL_SHAPE, _status.full_shape);
		SetBit(WeaselFlag::ASCII_PUNCT, _status.ascii_punct);
		SetBit(WeaselFlag::SIMPLIFIED_TO_TRADITIONAL, _status.s2t);
		SetBit(WeaselFlag::PREDICTION, _status.prediction);
		_schema_id = _status.schema_id;

		if (_status.ascii_mode)
		{
			flags &= (~TF_CONVERSIONMODE_NATIVE);
		}
		else
		{
			flags |= TF_CONVERSIONMODE_NATIVE;
		}

		if (_status.full_shape)
		{
			flags |= TF_CONVERSIONMODE_FULLSHAPE;
		}
		else
		{
			flags &= (~TF_CONVERSIONMODE_FULLSHAPE);
		}

		if (_status.ascii_punct)
		{
			flags |= TF_CONVERSIONMODE_SYMBOL;
		}
		else
		{
			flags &= (~TF_CONVERSIONMODE_SYMBOL);
		}
	}
	else
	{
		if (GetBit(WeaselFlag::ASCII_MODE) != _status.ascii_mode)
		{
			SetBit(WeaselFlag::ASCII_MODE, _status.ascii_mode);
			if (_status.ascii_mode)
			{
				flags &= (~TF_CONVERSIONMODE_NATIVE);
			}
			else
			{
				flags |= TF_CONVERSIONMODE_NATIVE;
			}
			state = true;
		}
		else if (GetBit(WeaselFlag::FULL_SHAPE) != _status.full_shape)
		{
			SetBit(WeaselFlag::FULL_SHAPE, _status.full_shape);
			if (_status.full_shape)
			{
				flags |= TF_CONVERSIONMODE_FULLSHAPE;
			}
			else
			{
				flags &= (~TF_CONVERSIONMODE_FULLSHAPE);
			}
			state = true;
		}
		else if (GetBit(WeaselFlag::ASCII_PUNCT) != _status.ascii_punct)
		{
			SetBit(WeaselFlag::ASCII_PUNCT, _status.ascii_punct);
			if (_status.ascii_punct)
			{
				flags |= TF_CONVERSIONMODE_SYMBOL;
			}
			else
			{
				flags &= (~TF_CONVERSIONMODE_SYMBOL);
			}
			state = true;
		}
		else if (GetBit(WeaselFlag::SIMPLIFIED_TO_TRADITIONAL) != _status.s2t)
		{
			SetBit(WeaselFlag::SIMPLIFIED_TO_TRADITIONAL, _status.s2t);
			state = true;
		}
		else if (GetBit(WeaselFlag::PREDICTION) != _status.prediction)
		{
			SetBit(WeaselFlag::PREDICTION, _status.prediction);
			state = true;
		}
		else if (_schema_id != _status.schema_id)
		{
			_schema_id = _status.schema_id;
			state = true;
		}
	}

#ifdef TEST
	LOG(INFO) << std::format("From WeaselTSF::_UpdateLanguageBar. flags = {:#x}", flags);
#endif // TEST

	// _SetCompartmentDWORD(flags, GUID_COMPARTMENT_KEYBOARD_INPUTMODE_CONVERSION);
	_pCompartmentConversion->_SetCompartmentDWORD(flags);

	_pLangBarButton->UpdateWeaselStatus(_status);
	return state;
}

void WeaselTSF::_ShowLanguageBar(BOOL show)
{
	if (!_pLangBarButton) return;
	_pLangBarButton->Show(show);

}

void WeaselTSF::_EnableLanguageBar(BOOL enable)
{
	if (!_pLangBarButton) return;
	_pLangBarButton->SetLangbarStatus(TF_LBI_STATUS_DISABLED, !enable);
}