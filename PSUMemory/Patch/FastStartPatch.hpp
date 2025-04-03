#pragma once

#include <map>

namespace PSUHook::FastStartPatch
{
	const static uintptr_t offset_sonic_logo_ss = 0x0051E278;
	const static uintptr_t offset_sonic_logo_ws = 0x0051E388;

	void ApplyPatch()
	{
		if( false == PSUIni::Get().ReadBool( L"custom", L"QUICK_START_MOD", false ) )
		{
			return;
		}

		std::array<uint8_t, 12> zeroBytes = { 0x00 };
		PSUMemory::Get().WriteMemory( offset_sonic_logo_ss, zeroBytes );
		PSUMemory::Get().WriteMemory( offset_sonic_logo_ws, zeroBytes );
	}

	REGISTER_PATCH( ApplyPatch );
}