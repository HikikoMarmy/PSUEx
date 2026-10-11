#pragma once

/*
* Keeps the controller working while the game window isn't in focus.
*
* The game's controller setup asks DirectInput for exclusive, foreground-only
* access (DISCL_EXCLUSIVE | DISCL_FOREGROUND, 5), so the pad goes quiet when
* the window loses focus. This asks for background access instead
* (DISCL_EXCLUSIVE | DISCL_BACKGROUND, 9).
*/

namespace PSUHook::ControllerBackgroundPatch
{
	// push 5; push ecx; push eax; call [edx+0x34] (SetCooperativeLevel)
	const static uintptr_t pad_cooperative_level = 0x00386EE7;
	const static uint8_t pad_cooperative_code[] = { 0x6A, 0x05, 0x51, 0x50, 0xFF, 0x52, 0x34 };

	void ApplyPatch()
	{
		if( false == PSUIni::Get().ReadBool( L"custom", L"CONTROLLER_BACKGROUND_MOD", false ) )
		{
			return;
		}

		// Only patch the code we expect, so another game version isn't changed.
		for( size_t i = 0; i < sizeof( pad_cooperative_code ); i++ )
		{
			if( PSUMemory::Get().ReadMemory< uint8_t >( pad_cooperative_level + i ) != pad_cooperative_code[ i ] )
			{
				printf( "ControllerBackgroundPatch: unexpected code, not applied\n" );
				return;
			}
		}

		PSUMemory::Get().WriteMemory< uint8_t >( pad_cooperative_level + 1, 0x09 );
	}

	REGISTER_PATCH( ApplyPatch );
}
