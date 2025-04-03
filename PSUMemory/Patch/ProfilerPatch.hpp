#pragma once

namespace PSUHook::Profiler
{
	const static uintptr_t offset_profiler = 0x0079C9A9;

	void ApplyPatch()
	{
		if( false == PSUIni::Get().ReadBool( L"custom", L"PROFILER_MOD", false ) ) return;

		PSUMemory::Get().WriteMemory( offset_profiler, PSUMemory::MakeByteArray( 0x01 ) );
	}

	REGISTER_PATCH( ApplyPatch );
}