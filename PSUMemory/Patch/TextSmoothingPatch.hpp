#pragma once

namespace PSUHook::SmootTextPatch
{
	void ApplyPatch()
	{
		if( false == PSUIni::Get().ReadBool( L"custom", L"SMOOTH_TEXT_MOD", false ) )
		{
			return;
		}

		PSUMemory::Get().WriteMemory( 0x005512F0, 2 );
	}

	REGISTER_PATCH( ApplyPatch );
}