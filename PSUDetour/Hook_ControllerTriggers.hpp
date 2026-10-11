/*
* Xbox controller triggers as buttons (TRIGGER_FIX_MOD).
*
* Through DirectInput an Xbox pad's two triggers share one axis, so the game
* can't bind them. DirectInput's device creation is hooked to recognise
* XInput pads (by the "IG_" in their device path). Each time the game reads
* such a pad, the real trigger values are read through XInput and each pulled
* trigger presses a button, so players can bind the triggers like any other
* button in the game's controller settings.
*
* Settings ([custom] in PsuIlluminus.ini):
*   TRIGGER_FIX_MOD      1 to turn it on
*   LEFT_TRIGGER_BUTTON  the button the left trigger presses (default 10)
*   RIGHT_TRIGGER_BUTTON the button the right trigger presses (default 11)
*   TRIGGER_PULL         how far a trigger must be pulled, 1-255 (default 30)
*
* Buttons are numbered as the game's controller settings show them
* (BUTTON 10). DirectInput doesn't say which XInput slot a pad is in, so pads
* are paired with connected XInput slots in order.
*/

#pragma once

#include "../PSUEx.h"

#include <atomic>
#include <mutex>
#include <string>

namespace ControllerTriggers
{
	// DIJOYSTATE and DIJOYSTATE2: their sizes, and where and how many their buttons are.
	const DWORD JOYSTATE_SIZE = 0x50;
	const DWORD JOYSTATE2_SIZE = 0x110;
	const size_t BUTTONS_AT = 0x30;
	const size_t JOYSTATE_BUTTONS = 32;
	const size_t JOYSTATE2_BUTTONS = 128;
	// DIPROP_GUIDANDPATH
	const uintptr_t GUID_AND_PATH = 12;
	const int MAX_PADS = 4;

	// The COM methods hooked, by their place in the interface.
	const int CREATE_DEVICE = 3;		// IDirectInput8::CreateDevice
	const int GET_PROPERTY = 5;			// IDirectInputDevice8::GetProperty
	const int GET_DEVICE_STATE = 9;		// IDirectInputDevice8::GetDeviceState

	struct XInputGamepad {
		WORD buttons;
		BYTE leftTrigger;
		BYTE rightTrigger;
		SHORT thumbs[ 4 ];
	};

	struct XInputState {
		DWORD packet;
		XInputGamepad gamepad;
	};

	// DIPROPGUIDANDPATH
	struct GuidAndPath {
		DWORD size;
		DWORD headerSize;
		DWORD obj;
		DWORD how;
		GUID classGuid;
		WCHAR path[ MAX_PATH ];
	};

	typedef HRESULT( WINAPI *DirectInput8Create_t )( HINSTANCE, DWORD, REFIID, LPVOID *, LPUNKNOWN );
	typedef HRESULT( WINAPI *CreateDevice_t )( void *thisPtr, REFGUID guid, void **device, LPUNKNOWN outer );
	typedef HRESULT( WINAPI *GetProperty_t )( void *thisPtr, uintptr_t prop, void *header );
	typedef HRESULT( WINAPI *GetDeviceState_t )( void *thisPtr, DWORD size, void *data );
	typedef DWORD( WINAPI *XInputGetState_t )( DWORD slot, XInputState *state );

	DirectInput8Create_t pOriginal_DirectInput8Create = nullptr;
	CreateDevice_t pOriginal_CreateDevice = nullptr;
	GetDeviceState_t pOriginal_GetDeviceState = nullptr;
	XInputGetState_t pXInputGetState = nullptr;

	int g_leftButton = 10;
	int g_rightButton = 11;
	int g_pull = 30;

	// Each recognised pad (device pointer) and its XInput slot.
	struct Pad {
		void *device;
		DWORD slot;
	};
	Pad g_pads[ MAX_PADS ] = {};
	std::atomic< int > g_padCount = 0;
	std::mutex g_lock;

	inline void *Method( void *object, int index )
	{
		return ( *reinterpret_cast< void *** >( object ) )[ index ];
	}

	// Windows' own XInput, loaded from the system folder: a copy in the game
	// folder (XInput Plus leaves some) may remap the pad or report its
	// triggers as 0.
	void LoadXInput()
	{
		wchar_t folder[ MAX_PATH ];
		UINT length = GetSystemDirectoryW( folder, MAX_PATH );
		if( length == 0 || length >= MAX_PATH ) return;

		for( const wchar_t *dll : { L"xinput1_4.dll", L"xinput1_3.dll", L"xinput9_1_0.dll" } )
		{
			std::wstring path = std::wstring( folder ) + L"\\" + dll;
			HMODULE module = LoadLibraryW( path.c_str() );
			if( !module ) continue;
			pXInputGetState = reinterpret_cast< XInputGetState_t >( GetProcAddress( module, "XInputGetState" ) );
			if( pXInputGetState )
			{
				printf( "ControllerTriggers: reading pads through %ls\n", path.c_str() );
				return;
			}
		}
		printf( "ControllerTriggers: no XInput, so controllers are left as they are\n" );
	}

	// The trigger values of the pad in XInput slot `slot`, if one is connected.
	bool TriggerValues( DWORD slot, BYTE &left, BYTE &right )
	{
		if( !pXInputGetState || slot >= MAX_PADS ) return false;
		XInputState state = {};
		if( pXInputGetState( slot, &state ) != ERROR_SUCCESS ) return false;
		left = state.gamepad.leftTrigger;
		right = state.gamepad.rightTrigger;
		return true;
	}

	// Whether a device path is an XInput pad's: Windows puts "IG_" in the path
	// of every XInput-capable pad (\\?\hid#vid_045e&pid_028e&ig_00#...).
	bool IsXInputPath( const wchar_t *path )
	{
		for( ; path[ 0 ] && path[ 1 ] && path[ 2 ]; path++ )
		{
			if( ( path[ 0 ] & 0xFFDF ) == L'I' && ( path[ 1 ] & 0xFFDF ) == L'G' && path[ 2 ] == L'_' )
				return true;
		}
		return false;
	}

	// Presses the left and right triggers' buttons while each is pulled far enough.
	void Press( BYTE *buttons, size_t count, BYTE left, BYTE right )
	{
		const int pulls[ 2 ][ 2 ] = { { left, g_leftButton }, { right, g_rightButton } };
		for( const auto &pull : pulls )
		{
			if( pull[ 0 ] >= g_pull && pull[ 1 ] >= 0 && static_cast< size_t >( pull[ 1 ] ) < count )
				buttons[ pull[ 1 ] ] |= 0x80;
		}
	}

	HRESULT WINAPI Hook_GetDeviceState( void *thisPtr, DWORD size, void *data )
	{
		HRESULT hr = pOriginal_GetDeviceState( thisPtr, size, data );
		if( FAILED( hr ) || !data || g_padCount == 0 ) return hr;

		size_t buttons;
		if( size == JOYSTATE_SIZE ) buttons = JOYSTATE_BUTTONS;
		else if( size == JOYSTATE2_SIZE ) buttons = JOYSTATE2_BUTTONS;
		else return hr;

		DWORD slot = 0;
		bool found = false;
		{
			std::lock_guard< std::mutex > guard( g_lock );
			for( const Pad &pad : g_pads )
			{
				if( pad.device && pad.device == thisPtr )
				{
					slot = pad.slot;
					found = true;
					break;
				}
			}
		}
		if( !found ) return hr;

		BYTE left, right;
		if( TriggerValues( slot, left, right ) )
			Press( static_cast< BYTE * >( data ) + BUTTONS_AT, buttons, left, right );
		return hr;
	}

	void OnDevice( void *device )
	{
		{
			std::lock_guard< std::mutex > guard( g_lock );
			for( Pad &pad : g_pads )
			{
				if( pad.device == device ) pad = {};
			}
		}

		GuidAndPath prop = {};
		prop.size = sizeof( prop );
		prop.headerSize = 0x10;
		auto getProperty = reinterpret_cast< GetProperty_t >( Method( device, GET_PROPERTY ) );
		if( FAILED( getProperty( device, GUID_AND_PATH, &prop ) ) ) return;
		prop.path[ MAX_PATH - 1 ] = 0;
		if( !IsXInputPath( prop.path ) ) return;

		{
			std::lock_guard< std::mutex > guard( g_lock );

			// The first connected XInput slot no other pad has.
			int slot = -1;
			for( DWORD i = 0; i < MAX_PADS && slot < 0; i++ )
			{
				bool taken = false;
				for( const Pad &pad : g_pads )
				{
					if( pad.device && pad.slot == i ) taken = true;
				}
				BYTE left, right;
				if( !taken && TriggerValues( i, left, right ) ) slot = static_cast< int >( i );
			}
			if( slot < 0 )
			{
				printf( "ControllerTriggers: an Xbox controller connected but no free XInput slot matches it\n" );
				return;
			}

			Pad *empty = nullptr;
			for( Pad &pad : g_pads )
			{
				if( !pad.device )
				{
					empty = &pad;
					break;
				}
			}
			if( !empty ) return;
			*empty = { device, static_cast< DWORD >( slot ) };

			int count = 0;
			for( const Pad &pad : g_pads )
			{
				if( pad.device ) count++;
			}
			g_padCount = count;
			printf( "ControllerTriggers: Xbox controller %d found\n", slot + 1 );
		}

		if( !pOriginal_GetDeviceState )
		{
			pOriginal_GetDeviceState = reinterpret_cast< GetDeviceState_t >( Method( device, GET_DEVICE_STATE ) );
			DetourTransactionBegin();
			DetourUpdateThread( GetCurrentThread() );
			DetourAttach( &( PVOID & )pOriginal_GetDeviceState, reinterpret_cast< PVOID >( Hook_GetDeviceState ) );
			if( DetourTransactionCommit() != NO_ERROR )
				printf( "ControllerTriggers: couldn't hook GetDeviceState\n" );
		}
	}

	HRESULT WINAPI Hook_CreateDevice( void *thisPtr, REFGUID guid, void **device, LPUNKNOWN outer )
	{
		HRESULT hr = pOriginal_CreateDevice( thisPtr, guid, device, outer );
		if( SUCCEEDED( hr ) && device && *device ) OnDevice( *device );
		return hr;
	}

	HRESULT WINAPI Hook_DirectInput8Create( HINSTANCE instance, DWORD version, REFIID riid, LPVOID *out, LPUNKNOWN outer )
	{
		HRESULT hr = pOriginal_DirectInput8Create( instance, version, riid, out, outer );
		if( FAILED( hr ) || !out || !*out || pOriginal_CreateDevice ) return hr;

		LoadXInput();
		if( !pXInputGetState ) return hr;

		pOriginal_CreateDevice = reinterpret_cast< CreateDevice_t >( Method( *out, CREATE_DEVICE ) );
		DetourTransactionBegin();
		DetourUpdateThread( GetCurrentThread() );
		DetourAttach( &( PVOID & )pOriginal_CreateDevice, reinterpret_cast< PVOID >( Hook_CreateDevice ) );
		if( DetourTransactionCommit() != NO_ERROR )
		{
			printf( "ControllerTriggers: couldn't hook CreateDevice\n" );
			return hr;
		}
		printf( "ControllerTriggers: the left trigger presses BUTTON %d and the right BUTTON %d\n", g_leftButton, g_rightButton );
		return hr;
	}

	// Called inside DetoursHookAll's transaction.
	void Hook()
	{
		if( false == PSUIni::Get().ReadBool( L"custom", L"TRIGGER_FIX_MOD", false ) )
		{
			return;
		}

		g_leftButton = PSUIni::Get().ReadInt( L"custom", L"LEFT_TRIGGER_BUTTON", 10 );
		g_rightButton = PSUIni::Get().ReadInt( L"custom", L"RIGHT_TRIGGER_BUTTON", 11 );
		g_pull = PSUIni::Get().ReadInt( L"custom", L"TRIGGER_PULL", 30 );
		// 0 would hold the buttons down all the time.
		if( g_pull < 1 ) g_pull = 1;
		if( g_pull > 255 ) g_pull = 255;

		// The game imports dinput8.dll, so it's loaded by now.
		HMODULE dinput = GetModuleHandleA( "dinput8.dll" );
		if( !dinput )
		{
			printf( "ControllerTriggers: dinput8.dll isn't loaded\n" );
			return;
		}
		pOriginal_DirectInput8Create = reinterpret_cast< DirectInput8Create_t >( GetProcAddress( dinput, "DirectInput8Create" ) );
		if( !pOriginal_DirectInput8Create ) return;
		DetourAttach( &( PVOID & )pOriginal_DirectInput8Create, reinterpret_cast< PVOID >( Hook_DirectInput8Create ) );
	}
}
