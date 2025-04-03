#pragma once

/*
* This hook is responsible for creating the game window.
* It allows for custom display modes and window positions.
*/

#include "../PSUEx.h"

enum DisplayMode
{
	Fullscreen = 0,
	Windowed = 1,
	Borderless = 2
};

HWND( WINAPI *Original_CreateWindowExA )
(
	DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle,
	int X, int Y, int nWidth, int nHeight,
	HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam
) = CreateWindowExA;

HWND WINAPI Hook_CreateWindowExA
(
	DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle,
	int PosX, int PosY, int nWidth, int nHeight,
	HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam
)
{
	const HWND hDesktop = GetDesktopWindow();

	RECT desktop;
	GetWindowRect( hDesktop, &desktop );

	int DisplayMode = PSUIni::Get().ReadInt( L"custom", L"DISPLAY_MODE", -1 );
	int DisplayIndex = PSUIni::Get().ReadInt( L"custom", L"DISPLAY_INDEX", 0 );
	int ScreenResolutionX = PSUIni::Get().ReadInt( L"custom", L"RESOLUTION_WIDTH", nWidth );
	int ScreenResolutionY = PSUIni::Get().ReadInt( L"custom", L"RESOLUTION_HEIGHT", nHeight );
	int LockWindowSize = PSUIni::Get().ReadInt( L"custom", L"LOCK_WINDOW_SIZE", 1 );

	std::vector<MONITORINFOEX> monitors;

	auto MonitorEnumProc = []( HMONITOR hMonitor, HDC, LPRECT, LPARAM lParam ) -> BOOL
	{
		std::vector<MONITORINFOEX> *pMonitors = reinterpret_cast< std::vector<MONITORINFOEX>* >( lParam );
		MONITORINFOEX mi;
		mi.cbSize = sizeof( mi );
		if( GetMonitorInfo( hMonitor, &mi ) )
		{
			pMonitors->push_back( mi );
		}
		return TRUE;
	};

	// Get all active monitors
	EnumDisplayMonitors( nullptr, nullptr, MonitorEnumProc, reinterpret_cast< LPARAM >( &monitors ) );

	MONITORINFOEX targetMonitor;

	if( !monitors.empty() && DisplayIndex >= 0 && DisplayIndex < monitors.size() )
	{
		targetMonitor = monitors[ DisplayIndex ];

		// Center the window on the target monitor
		PosX = targetMonitor.rcMonitor.left + ( ( targetMonitor.rcMonitor.right - targetMonitor.rcMonitor.left ) - ScreenResolutionX ) / 2;
		PosY = targetMonitor.rcMonitor.top + ( ( targetMonitor.rcMonitor.bottom - targetMonitor.rcMonitor.top ) - ScreenResolutionY ) / 2;

		PosX = max( targetMonitor.rcMonitor.left, PosX );
		PosY = max( targetMonitor.rcMonitor.top, PosY );
	}
	else
	{
		// Default to primary monitor if index is invalid
		PosX = ( GetSystemMetrics( SM_CXSCREEN ) - ScreenResolutionX ) / 2;
		PosY = ( GetSystemMetrics( SM_CYSCREEN ) - ScreenResolutionY ) / 2;

		// Fill in targetMonitor manually for clamping to primary screen
		targetMonitor.cbSize = sizeof( targetMonitor );
		GetMonitorInfo( MonitorFromPoint( { PosX, PosY }, MONITOR_DEFAULTTOPRIMARY ), &targetMonitor );
	}

	int monitorWidth = targetMonitor.rcMonitor.right - targetMonitor.rcMonitor.left;
	int monitorHeight = targetMonitor.rcMonitor.bottom - targetMonitor.rcMonitor.top;

	if( DisplayMode == DisplayMode::Windowed )
	{
		// Windowed + Minimize
		dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
		dwStyle = WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU;

		// Allow resizing if not locked
		if( !LockWindowSize )
		{
			dwStyle |= WS_SIZEBOX | WS_MAXIMIZEBOX;
		}

		RECT rc = { 0, 0, ScreenResolutionX, ScreenResolutionY };

		AdjustWindowRectEx( &rc, dwStyle, 0, dwExStyle );

		nWidth = rc.right - rc.left;
		nHeight = rc.bottom - rc.top;
	}
	else
	{
		// Borderless/Fullscreen
		dwExStyle = 0;
		dwStyle = WS_POPUP;

		if( ScreenResolutionX > monitorWidth )
			ScreenResolutionX = monitorWidth;

		if( ScreenResolutionY > monitorHeight )
			ScreenResolutionY = monitorHeight;
	}

	auto hWnd = Original_CreateWindowExA
	(
		dwExStyle,
		lpClassName,
		lpWindowName,
		dwStyle,
		PosX, PosY,
		nWidth, nHeight,
		hWndParent,
		hMenu,
		hInstance,
		lpParam
	);

	Global::hWnd = hWnd;
	Global::screenWidth = ScreenResolutionX;
	Global::screenHeight = ScreenResolutionY;

	Config::Get().LoadFromFile();

	return hWnd;
}