#pragma once

#include "imgui.h"
#include "backends/imgui_impl_dx9.h"
#include "backends/imgui_impl_win32.h"

#include <memory>
#include <vector>
#include <string>
#include <array>

class IFWinCtrl
{
public:
	IFWinCtrl( std::string controlId )
		: controlId( controlId )
	{
		m_bEnabled = true;
	}

	IFWinCtrl( std::string controlId, bool bEnabled )
		: controlId( controlId )
	{
		m_bEnabled = bEnabled;
	}

	virtual ~IFWinCtrl() = default;

	virtual void Render() = 0;

	virtual void SetInitialWindowPosition()
	{
		m_position = ImVec2( 0, 0 );
		m_size = ImVec2( 0, 0 );
	}

	void Toggle()
	{
		m_bEnabled = !m_bEnabled;
	}

	bool IsEnabled() const
	{
		return m_bEnabled;
	}

	void SetEnabled( bool bEnabled )
	{
		m_bEnabled = bEnabled;
	}

	std::string GetControlId() const
	{
		return controlId;
	}

protected:
	std::string controlId;
	ImVec2 m_position;
	ImVec2 m_size;
	bool m_bEnabled;
};

typedef std::shared_ptr< IFWinCtrl > WinCtrlPtr;