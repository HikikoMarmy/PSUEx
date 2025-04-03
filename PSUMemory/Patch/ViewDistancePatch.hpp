#pragma once

namespace PSUHook::ViewDistancePatch
{
	const static uintptr_t enemy_render_distance = 0x005966D4;

	void ApplyPatch()
	{
		if( false == PSUIni::Get().ReadBool( L"custom", L"VIEW_DISTANCE_MOD", false ) )
		{
			return;
		}

		PSUMemory::Get().WriteMemory( enemy_render_distance, 1000.0f );
	}

	REGISTER_PATCH( ApplyPatch );
}