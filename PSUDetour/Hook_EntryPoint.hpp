/*
* This is where we apply our EXE patches.
*/

#pragma once

#include "../PSUEx.h"

typedef int ( CALLBACK *Function_WinMain )
(
	HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd
);

Function_WinMain Original_WinMain = reinterpret_cast< Function_WinMain >( 0x00774AD0 );

int CALLBACK Hook_WinMain
(
	HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd
)
{
	PSUHook::PatchRegistry::ApplyAll();
	return Original_WinMain( hInstance, hPrevInstance, lpCmdLine, nShowCmd );
}