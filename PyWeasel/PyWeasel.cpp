﻿#include "stdafx.h"
#include <PyWeasel.h>

PyWeaselHandler::PyWeaselHandler()
{
}

PyWeaselHandler::~PyWeaselHandler()
{
}

void PyWeaselHandler::Initialize()
{
	// 初始化Python解释器, 有必要在PyWeaselHandler创建之前调用
	Py_Initialize();
	try
	{
		python::object module = python::import("engine.weasel");
		m_service = module.attr("service");
	}
	catch (python::error_already_set e)
	{
		python::handle_exception();
	}

	if (m_service.is_none())
	{
		throw std::exception("weasel service not found!");
	}
}

void PyWeaselHandler::Finalize()
{
	try
	{
		m_service.attr("cleanup")();
	}
	catch (python::error_already_set e)
	{
	}
	// Boost.Python doesn't support Py_Finalize yet, so don't call it!
}

UINT PyWeaselHandler::FindSession(UINT session_id)
{
	bool found = false;
	try
	{
		found = python::extract<bool>(m_service.attr("has_session")(session_id));
	}
	catch (python::error_already_set e)
	{
		return 0;
	}
	return found ? session_id : 0;
}

UINT PyWeaselHandler::AddSession(LPWSTR buffer)
{
	UINT id = 0;
	try
	{
		id = python::extract<UINT>(m_service.attr("create_session")());
		if (!id)
		{
			return 0;
		}
		python::object session = m_service.attr("get_session")(id);
		if (session.is_none())
		{
			return 0;
		}
		std::wstring resp = python::extract<std::wstring>(session.attr("get_response")());
		_Respond(buffer, resp);

	}
	catch (python::error_already_set e)
	{
		return 0;
	}

	return id;
}

UINT PyWeaselHandler::RemoveSession(UINT session_id)
{
	bool done = false;
	try
	{
		done = python::extract<bool>(m_service.attr("destroy_session")(session_id));
	}
	catch (python::error_already_set e)
	{
		return 0;
	}
	return done ? session_id : 0;
}

BOOL PyWeaselHandler::ProcessKeyEvent(weasel::KeyEvent keyEvent, UINT session_id, LPWSTR buffer)
{
	bool taken = false;
	try
	{
		python::object session = m_service.attr("get_session")(session_id);
		if (session.is_none())
		{
			return FALSE;
		}
		
		taken = python::extract<bool>(session.attr("process_key_event")(keyEvent.keycode, keyEvent.mask));
		std::wstring resp = python::extract<std::wstring>(session.attr("get_response")());
		_Respond(buffer, resp);
	}
	catch (python::error_already_set e)
	{
		return FALSE;
	}
	
	return (BOOL)taken;
}

bool PyWeaselHandler::_Respond(LPWSTR buffer, std::wstring_view msg)
{
	memset(buffer, 0, WEASEL_IPC_BUFFER_SIZE);
	wbufferstream bs(buffer, WEASEL_IPC_BUFFER_LENGTH);
	bs << msg.data();
	if (!bs.good())
	{
		// response text toooo long!
		return false;
	}
	return true;
}
