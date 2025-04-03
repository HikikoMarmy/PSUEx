#pragma once

#include <vector>

namespace PSUHook::NoVideoPatch
{
	const static uintptr_t offset_skip_video = 0x00260EBF;

	void ApplyPatch()
	{
		if( false == PSUIni::Get().ReadBool( L"custom", L"NO_VIDEO_MOD", false ) )
		{
			return;
		}

		PSUMemory::Get().WriteMemory( offset_skip_video, PSUMemory::MakeByteArray( 0x90, 0x90, 0x90 ) );
	}

	REGISTER_PATCH( ApplyPatch );
}