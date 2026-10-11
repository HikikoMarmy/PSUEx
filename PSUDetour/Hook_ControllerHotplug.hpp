/*
* Controllers plugged in after the game started (CONTROLLER_HOTPLUG_MOD).
*
* The game looks for controllers once, at startup: it lists DirectInput's
* attached game controllers and sets each up in one of its two slots
* (0x00786E80), and afterwards only reads those slots. A controller there at
* startup comes back after being unplugged, but one that wasn't there never
* shows up.
*
* The game's per-frame controller poll (0x00786C90) is hooked. Before it
* runs, while a slot is free and XInput has more pads than the game has set
* up, the attached controllers are listed as the game does at startup, and
* each one no slot has yet is set up in a free slot by the game's own slot
* setup. From then on the game reads it as if it had been there all along
* (TRIGGER_FIX_MOD and CONTROLLER_BACKGROUND_MOD apply to it too, as they
* work on the game's own setup).
*
* Settings ([custom] in PsuIlluminus.ini):
*   CONTROLLER_HOTPLUG_MOD  1 to turn it on
*/

#pragma once

#include "../PSUEx.h"

#include <string>

namespace ControllerHotplug
{
	// The game's per-frame controller poll: sub esp,0x118; mov eax,[0x9700E0]
	typedef void( __cdecl *Poll_t )();
	Poll_t pOriginal_Poll = reinterpret_cast< Poll_t >( 0x00786C90 );
	const uint8_t POLL_CODE[] = { 0x81, 0xEC, 0x18, 0x01, 0x00, 0x00, 0xA1, 0xE0, 0x00, 0x97, 0x00 };

	// The game's IDirectInput8A (a global pointer), and its two controller
	// slots of 0x84 bytes, each's device (IDirectInputDevice8A, 0 for none)
	// at +0x2C.
	const uintptr_t DIRECT_INPUT = 0x00A89140;
	const uintptr_t SLOTS = 0x00A8DCB0;
	const uintptr_t SLOT_STRIDE = 0x84;
	const uintptr_t SLOT_DEVICE = 0x2C;
	const int SLOT_COUNT = 2;

	// Sets a slot up for a device: the slot in EAX, the device's instance
	// GUID (16 bytes) on the stack, which the caller takes off; AL = 1 if it
	// set it up. A slot that has a device is left alone.
	const uintptr_t SLOT_SETUP = 0x00786E80;

	// The code the above is read from, checked before any of it is used.
	// The slot setup: sub esp,0x2c; push ebp; push esi; mov esi,eax;
	// imul esi,esi,0x84; add esi,0xa8dcb0; lea ebp,[eax+eax*8]; mov eax,[esi+0x2c]
	const uint8_t SLOT_SETUP_CODE[] = { 0x83, 0xEC, 0x2C, 0x55, 0x56, 0x8B, 0xF0, 0x69, 0xF6, 0x84, 0x00, 0x00, 0x00, 0x81,
		0xC6, 0xB0, 0xDC, 0xA8, 0x00, 0x8D, 0x2C, 0xC0, 0x8B, 0x46, 0x2C };
	// The startup enumeration's callback: mov eax,[0xa8dca8]; cmp eax,2
	const uint8_t CALLBACK_CODE[] = { 0xA1, 0xA8, 0xDC, 0xA8, 0x00, 0x83, 0xF8, 0x02 };
	// ... calling the slot setup with the slot in EAX.
	const uint8_t CALLBACK_SETUP_CODE[] = { 0xE8, 0xEE, 0xFD, 0xFF, 0xFF, 0x83, 0xC4, 0x10, 0x84, 0xC0 };
	// The startup enumeration: mov eax,[0xa89140]; mov ecx,[eax]; push 1
	// (attached only); push 0; push 0x787060; push 4 (game controllers)
	const uint8_t ENUMERATION_CODE[] = { 0xA1, 0x40, 0x91, 0xA8, 0x00, 0x8B, 0x08, 0x6A, 0x01, 0x6A, 0x00, 0x68, 0x60, 0x70,
		0x78, 0x00, 0x6A, 0x04 };
	// The per-frame poll over the slots: mov edi,0xa8ddb8; mov ebp,0xa8dcb0; mov esi,[ebp+0x2c]
	const uint8_t POLL_SLOTS_CODE[] = { 0xBF, 0xB8, 0xDD, 0xA8, 0x00, 0xBD, 0xB0, 0xDC, 0xA8, 0x00, 0x8B, 0x75, 0x2C };

	struct Check {
		uintptr_t address;
		const uint8_t *code;
		size_t size;
	};
	const Check CHECKS[] = {
		{ 0x00786E80, SLOT_SETUP_CODE, sizeof( SLOT_SETUP_CODE ) },
		{ 0x00787060, CALLBACK_CODE, sizeof( CALLBACK_CODE ) },
		{ 0x0078708D, CALLBACK_SETUP_CODE, sizeof( CALLBACK_SETUP_CODE ) },
		{ 0x007870DC, ENUMERATION_CODE, sizeof( ENUMERATION_CODE ) },
		{ 0x00786CAC, POLL_SLOTS_CODE, sizeof( POLL_SLOTS_CODE ) },
	};

	// DI8DEVCLASS_GAMECTRL and DIEDFL_ATTACHEDONLY, as the game asks.
	const DWORD GAME_CONTROLLERS = 4;
	const DWORD ATTACHED_ONLY = 1;
	// IDirectInput8::EnumDevices, IDirectInputDevice8::GetDeviceInfo.
	const int ENUM_DEVICES = 4;
	const int GET_DEVICE_INFO = 15;
	// DIDEVICEINSTANCEA: its size, and where its instance GUID is.
	const size_t DEVICE_INSTANCE_SIZE = 0x244;
	const size_t INSTANCE_GUID_AT = 4;
	const int MAX_PADS = 4;

	// How often XInput is asked which pads there are (asking an empty slot
	// is slow); after a pad appears, how long before looking (DirectInput
	// sees it a little after XInput), and how many looks before giving up
	// until the pads change again.
	const DWORD CHECK_EVERY_MS = 1000;
	const DWORD SETTLE_MS = 1000;
	const int LOOKS = 4;

	struct XInputState {
		DWORD packet;
		BYTE gamepad[ 12 ];
	};
	typedef DWORD( WINAPI *XInputGetState_t )( DWORD slot, XInputState *state );
	typedef HRESULT( WINAPI *EnumDevices_t )( void *thisPtr, DWORD type, BOOL( CALLBACK *callback )( const void *, void * ), void *ref, DWORD flags );
	typedef HRESULT( WINAPI *GetDeviceInfo_t )( void *thisPtr, void *instance );

	XInputGetState_t pXInputGetState = nullptr;
	bool g_on = false;
	DWORD g_checked = 0;
	bool g_everChecked = false;
	int g_pads = -1;
	DWORD g_lookAt = 0;
	bool g_looking = false;
	int g_looks = 0;

	inline void *Method( void *object, int index )
	{
		return ( *reinterpret_cast< void *** >( object ) )[ index ];
	}

	// Windows' own XInput, from the system folder.
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
			if( pXInputGetState ) return;
		}
	}

	// How many pads XInput has connected.
	int ConnectedPads()
	{
		int count = 0;
		for( DWORD i = 0; i < MAX_PADS; i++ )
		{
			XInputState state = {};
			if( pXInputGetState( i, &state ) == ERROR_SUCCESS ) count++;
		}
		return count;
	}

	void *SlotDevice( int slot )
	{
		return *reinterpret_cast< void ** >( SLOTS + slot * SLOT_STRIDE + SLOT_DEVICE );
	}

	// A device's instance GUID.
	bool InstanceGuid( void *device, GUID &guid )
	{
		uint8_t info[ DEVICE_INSTANCE_SIZE ] = {};
		*reinterpret_cast< DWORD * >( info ) = DEVICE_INSTANCE_SIZE;
		auto getInfo = reinterpret_cast< GetDeviceInfo_t >( Method( device, GET_DEVICE_INFO ) );
		if( FAILED( getInfo( device, info ) ) ) return false;
		memcpy( &guid, info + INSTANCE_GUID_AT, sizeof( GUID ) );
		return true;
	}

	struct Found {
		GUID guids[ 16 ];
		int count;
	};

	// DIEnumDevicesCallbackA: notes each controller's instance GUID.
	BOOL CALLBACK OnFound( const void *instance, void *ref )
	{
		Found *found = static_cast< Found * >( ref );
		if( instance && found->count < 16 )
		{
			memcpy( &found->guids[ found->count ], static_cast< const uint8_t * >( instance ) + INSTANCE_GUID_AT, sizeof( GUID ) );
			found->count++;
		}
		return TRUE;	// DIENUM_CONTINUE
	}

	// The game's slot setup for `slot` and the device `guid`.
	bool Setup( DWORD slot, const GUID &guid )
	{
		DWORD words[ 4 ];
		memcpy( words, &guid, sizeof( words ) );
		uintptr_t func = SLOT_SETUP;
		DWORD ok = 0;
#ifdef _MSC_VER
		__asm
		{
			lea ecx, words
			push dword ptr[ ecx + 12 ]
			push dword ptr[ ecx + 8 ]
			push dword ptr[ ecx + 4 ]
			push dword ptr[ ecx ]
			mov eax, slot
			call func
			add esp, 16
			mov ok, eax
		}
#else
		asm volatile(
			"pushl 12(%2)\n\t"
			"pushl 8(%2)\n\t"
			"pushl 4(%2)\n\t"
			"pushl (%2)\n\t"
			"call *%3\n\t"
			"addl $16, %%esp"
			: "=a"( ok )
			: "a"( slot ), "r"( words ), "r"( func )
			: "ecx", "edx", "memory", "cc" );
#endif
		return ( ok & 0xFF ) != 0;
	}

	// Sets up each attached controller no slot has, in the free slots.
	// Returns how many were added.
	int AddNew()
	{
		void *directInput = *reinterpret_cast< void ** >( DIRECT_INPUT );
		if( !directInput ) return 0;

		GUID taken[ SLOT_COUNT ] = {};
		int takenCount = 0;
		for( int i = 0; i < SLOT_COUNT; i++ )
		{
			void *device = SlotDevice( i );
			if( device && InstanceGuid( device, taken[ takenCount ] ) ) takenCount++;
		}

		Found attached = {};
		auto enumDevices = reinterpret_cast< EnumDevices_t >( Method( directInput, ENUM_DEVICES ) );
		HRESULT hr = enumDevices( directInput, GAME_CONTROLLERS, OnFound, &attached, ATTACHED_ONLY );
		if( FAILED( hr ) )
		{
			printf( "ControllerHotplug: couldn't list the attached controllers (0x%08lx)\n", static_cast< unsigned long >( hr ) );
			return 0;
		}

		int added = 0;
		for( int k = 0; k < attached.count; k++ )
		{
			bool has = false;
			for( int t = 0; t < takenCount; t++ )
			{
				if( IsEqualGUID( attached.guids[ k ], taken[ t ] ) ) has = true;
			}
			if( has ) continue;

			int slot = -1;
			for( int i = 0; i < SLOT_COUNT && slot < 0; i++ )
			{
				if( !SlotDevice( i ) ) slot = i;
			}
			if( slot < 0 ) break;

			if( Setup( static_cast< DWORD >( slot ), attached.guids[ k ] ) )
			{
				added++;
				printf( "ControllerHotplug: a controller plugged in after the game started is set up (the game's controller %d)\n", slot + 1 );
			}
			else
			{
				printf( "ControllerHotplug: the game couldn't set up a controller plugged in after it started\n" );
			}
		}
		return added;
	}

	// Once a frame, on the game's thread, before it reads its controllers.
	void Frame()
	{
		DWORD now = GetTickCount();
		if( !g_everChecked || now - g_checked >= CHECK_EVERY_MS )
		{
			g_everChecked = true;
			g_checked = now;
			int pads = ConnectedPads();
			if( pads != g_pads )
			{
				// Changed (or the first look): look again, from a moment on.
				g_pads = pads;
				g_looks = LOOKS;
				g_looking = true;
				g_lookAt = now;
			}
		}
		if( !g_looking || now - g_lookAt < SETTLE_MS ) return;

		int filled = 0;
		for( int i = 0; i < SLOT_COUNT; i++ )
		{
			if( SlotDevice( i ) ) filled++;
		}
		// Nothing to add: every slot taken, or no more pads than set up.
		if( filled >= SLOT_COUNT || g_pads <= filled )
		{
			g_looking = false;
			return;
		}

		int added = AddNew();
		if( g_looks > 0 ) g_looks--;
		g_looking = added == 0 && g_looks > 0;
		g_lookAt = now;
	}

	void __cdecl Hook_Poll()
	{
		Frame();
		pOriginal_Poll();
	}

	bool CodeIs( uintptr_t address, const uint8_t *code, size_t size )
	{
		return memcmp( reinterpret_cast< const void * >( address ), code, size ) == 0;
	}

	// Called inside DetoursHookAll's transaction.
	void Hook()
	{
		if( false == PSUIni::Get().ReadBool( L"custom", L"CONTROLLER_HOTPLUG_MOD", false ) )
		{
			return;
		}

		if( !CodeIs( reinterpret_cast< uintptr_t >( pOriginal_Poll ), POLL_CODE, sizeof( POLL_CODE ) ) )
		{
			printf( "ControllerHotplug: the controller poll isn't the expected code; not on\n" );
			return;
		}
		for( const Check &check : CHECKS )
		{
			if( !CodeIs( check.address, check.code, check.size ) )
			{
				printf( "ControllerHotplug: the game's controller setup isn't as expected (at 0x%08x); not on\n", static_cast< unsigned >( check.address ) );
				return;
			}
		}

		LoadXInput();
		if( !pXInputGetState )
		{
			printf( "ControllerHotplug: no XInput; not on\n" );
			return;
		}

		DetourAttach( &( PVOID & )pOriginal_Poll, reinterpret_cast< PVOID >( Hook_Poll ) );
		printf( "ControllerHotplug: controllers plugged in after the game starts are set up\n" );
	}
}
