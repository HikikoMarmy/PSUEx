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
	int DisplayMode = PSUIni::Get().ReadInt( L"custom", L"DISPLAY_MODE", -1 );
	int LockWindowSize = PSUIni::Get().ReadInt( L"custom", L"LOCK_WINDOW_SIZE", 1 );

	PosX = Global::windowPosX;
	PosY = Global::windowPosY;

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

		RECT rc = { 0, 0, Global::screenWidth, Global::screenHeight };

		AdjustWindowRectEx( &rc, dwStyle, 0, dwExStyle );

		nWidth = rc.right - rc.left;
		nHeight = rc.bottom - rc.top;
	}
	else
	{
		// Borderless/Fullscreen
		dwExStyle = 0;
		dwStyle = WS_POPUP;

		nWidth = Global::screenWidth;
		nHeight = Global::screenHeight;
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

	Config::Get().LoadFromFile();

	return hWnd;
}