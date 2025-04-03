#pragma once

namespace PSUHook::FastGrindingPatch
{
	void ApplyPatch()
	{
		if( false == PSUIni::Get().ReadBool( L"custom", L"QUICK_GRINDING_MOD", false ) )
		{
			return;
		}

		// SUCCESS GRIND: Nop call to success waiting grinding time.
		PSUMemory::Get().WriteMemory( 0x0025E013, PSUMemory::MakeByteArray( 0x90, 0x90, 0x90, 0x90, 0x90 ) );

		// FAILURE GRIND: Offset to grinding time float (make it 0). This keeps the break bar effect but makes it instant.
		PSUMemory::Get().WriteMemory( 0x001BFB67, PSUMemory::MakeByteArray( 0xD8, 0x0D, 0xEC, 0x9F, 0x92, 0x00 ) );
		PSUMemory::Get().WriteMemory( 0x001BFB77, PSUMemory::MakeByteArray( 0xD8, 0x05, 0xEC, 0x9F, 0x92, 0x00 ) );
		PSUMemory::Get().WriteMemory( 0x001BFB86, PSUMemory::MakeByteArray( 0xD8, 0x15, 0xEC, 0x9F, 0x92, 0x00 ) );

		// GC Grinding Bar
		PSUMemory::Get().WriteMemory( 0x002E66E2, PSUMemory::MakeByteArray( 0x90, 0x90, 0x90, 0x90, 0x90 ) );
	}

	REGISTER_PATCH( ApplyPatch );
}