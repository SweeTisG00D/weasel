#include "pch.h"
#include <WeaselUI.h>
#include "WeaselPanel.h"
#include "test.h"
#ifdef TEST
#define WEASEL_ENABLE_LOGGING
#include "logging.h"
#else
#include "logging.h"
#endif // TEST

using namespace weasel;

struct UI::Data
{
	Context ctx;
	Context octx;
	Status status;
	UIStyle style;
	UIStyle ostyle;

	PDWR pDWR;
	std::function<void(int* const, int* const, bool* const)> UICallback;
	std::function<void(const RECT&)> SetRectCallback;
	bool caretFollowing{ true };
	bool shown{ false };
};

class UI::UIImpl
{
public:
	WeaselPanel panel;

	UIImpl(weasel::UI& ui)
		: panel(ui), m_ui{ ui }
	{
	}
	~UIImpl()
	{
	}
	void Refresh()
	{
		if (!panel.IsWindow()) return;
		if (timer)
		{
			Hide();
			panel.KillTimer(AUTOHIDE_TIMER);
			timer = 0;
		}
		panel.Refresh();
	}

	void Show();
	void Hide();
	void ShowWithTimeout(DWORD millisec);

	bool GetIsReposition();

	static VOID CALLBACK OnTimer(
		_In_  HWND hwnd,
		_In_  UINT uMsg,
		_In_  UINT_PTR idEvent,
		_In_  DWORD dwTime
	);

public:
	static const int AUTOHIDE_TIMER = 20121220;
	static UINT_PTR timer;
	
private:
	UI& m_ui;
};

UINT_PTR UI::UIImpl::timer = 0;

void UI::UIImpl::Show()
{
#ifdef TEST
	LOG(INFO) << std::format("From UIImpl::Show. panel.IsWindow() = {}", panel.IsWindow());
#endif // TEST
	if (!panel.IsWindow()) return;	
	panel.ShowWindow(SW_SHOWNA);
	NotifyWinEvent(EVENT_OBJECT_IME_SHOW, panel.m_hWnd, OBJID_CLIENT, CHILDID_SELF);
#ifdef TEST
	LOG(INFO) << std::format("From UIImpl::Show. timer = {}", timer);
#endif // TEST
	m_ui.m_data->shown = true;
	if (timer)
	{
		panel.KillTimer(AUTOHIDE_TIMER);
		timer = 0;
	}
}

void UI::UIImpl::Hide()
{
	if (!panel.IsWindow()) return;	
	panel.ShowWindow(SW_HIDE);
	NotifyWinEvent(EVENT_OBJECT_IME_HIDE, panel.m_hWnd, OBJID_CLIENT, CHILDID_SELF);
	m_ui.m_data->shown = false;
	if (timer)
	{
		panel.KillTimer(AUTOHIDE_TIMER);
		timer = 0;
	}
}

void UI::UIImpl::ShowWithTimeout(DWORD millisec)
{
	if (!panel.IsWindow()) return;
	DLOG(INFO) << "ShowWithTimeout: " << millisec;
	panel.ShowWindow(SW_SHOWNA);
	m_ui.m_data->shown = true;
	panel.SetTimer(AUTOHIDE_TIMER, millisec, &UIImpl::OnTimer);
	timer = UINT_PTR(this);
}
bool UI::UIImpl::GetIsReposition()
{
	return panel.GetIsReposition();
}

VOID CALLBACK UI::UIImpl::OnTimer(
  _In_  HWND hwnd,
  _In_  UINT uMsg,
  _In_  UINT_PTR idEvent,
  _In_  DWORD dwTime
)
{
	DLOG(INFO) << "OnTimer:";
	KillTimer(hwnd, idEvent);
	UIImpl* self = (UIImpl*)timer;
	timer = 0;
	if (self)
	{
		self->Hide();
		self->m_ui.m_data->shown = false;
	}
}

UI::UI()
{
	m_data = std::make_unique<Data>();
}

UI::~UI()
{
	if (pimpl_)
		Destroy(true);
	m_data = nullptr;
}

bool UI::Create(HWND parent)
{
#ifdef TEST
	LOG(INFO) << std::format("From UI::Create.");
#endif // TEST
	if (!pimpl_)
	{
		pimpl_ = std::make_unique<UIImpl>(*this);
	}
#ifdef TEST
	LOG(INFO) << std::format("From UI::Create. make_unique<UIImpl>(*this)");
#endif // TEST
	pimpl_->panel.Create(parent, 0, 0, WS_POPUP, WS_EX_TOOLWINDOW | WS_EX_TOPMOST | WS_EX_NOACTIVATE | WS_EX_TRANSPARENT, 0U, 0);
#ifdef TEST
	LOG(INFO) << std::format("From UI::Create. done.");
#endif // TEST
	return true;
}

// for ending of composition
void UI::Destroy(bool full)
{
	if (pimpl_)
	{
		// destroy panel
		if (pimpl_->panel.IsWindow())
		{
			pimpl_->panel.DestroyWindow();
#ifdef TEST
			LOG(INFO) << std::format("From UI::Destroy. pimpl_->panel.DestroyWindow()");
#endif // TEST
		}
		if (full)
		{
			pimpl_ = nullptr;
			m_data->pDWR.reset();
		}
	}
}

void UI::Show()
{
#ifdef TEST
	LOG(INFO) << std::format("From UI::Show.");
#endif // TEST
	if (pimpl_)
	{
		pimpl_->Show();
	}
}

void UI::Hide()
{
	if (pimpl_)
	{
		pimpl_->Hide();
	}
}

void UI::ShowWithTimeout(DWORD millisec)
{
	if (pimpl_)
	{
		pimpl_->ShowWithTimeout(millisec);
	}
}

bool UI::IsCountingDown() const
{
	return pimpl_ && pimpl_->timer != 0;
}

bool UI::IsShown() const
{
	return m_data->shown;
}

void UI::Refresh()
{
	if (pimpl_)
	{
		pimpl_->Refresh();
	}
}

void UI::UpdateInputPosition(RECT const& rc)
{
	if (pimpl_ && pimpl_->panel.IsWindow())
	{		
		pimpl_->panel.MoveTo(rc); 
		NotifyWinEvent(EVENT_OBJECT_IME_CHANGE, pimpl_->panel.m_hWnd, OBJID_CLIENT, CHILDID_SELF);
	}
}

void UI::Update(const Context &ctx, const Status &status)
{
	m_data->ctx = ctx;
	m_data->status = status;
	Refresh();
}

Context& UI::ctx()
{
	return m_data->ctx;
}

Context& UI::octx()
{
	return m_data->octx;
}

Status& UI::status()
{
	return m_data->status;
}

UIStyle& UI::style() 
{
	return m_data->style;
}

UIStyle& UI::ostyle()
{
	return m_data->ostyle;
}

PDWR UI::pdwr()
{
	return m_data->pDWR;
}

bool weasel::UI::GetIsReposition()
{
	if (pimpl_)
		return pimpl_->panel.GetIsReposition();
	return false;
}

void weasel::UI::SetCaretFollowing(const bool following)
{
	m_data->caretFollowing = following;
	if (pimpl_)
		pimpl_->panel.SetCaretFollowing(following);
}

bool UI::GetCaretFollowing() const
{
	return m_data->caretFollowing;
}

std::function<void(int* const, int* const, bool* const)>& UI::uiCallback()
{
	return m_data->UICallback;
}

void UI::SetSelectCallback(std::function<void(int* const, int* const, bool* const)> const& func)
{
	m_data->UICallback = func;
}

std::function<void(const RECT&)>& UI::SetRectCallback()
{
	return m_data->SetRectCallback;
}

void UI::SetRectCallback(std::function<void(const RECT&)> func)
{
	m_data->SetRectCallback = func;
}
