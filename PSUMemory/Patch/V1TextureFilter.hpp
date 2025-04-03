#pragma once

namespace PSUHook::V1TextureFilterPatch
{
	void ApplyPatch()
	{
		if( false == PSUIni::Get().ReadBool( L"custom", L"V1_TEXTURE_MOD", false ) )
		{
			return;
		}

		PSUMemory::Get().WriteMemory( 0x00551360, 0 );
	}

	REGISTER_PATCH( ApplyPatch );
}