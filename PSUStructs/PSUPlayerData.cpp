#include "PSUPlayerData.h"
#include "../PSUMemory/Memory.hpp"

bool PSUPlayer::GetPlayerPtr()
{
	// Note: The pointer to player data is assigned in sub_536D80,
	// and should probably be detoured to update only when necessary.

	// The pointer is invalid when:
	// 1. The player is not logged in.
	// 2. The player is changing areas.
	// 3. The player is changing universe.
	ptr_to_player = PSUMemory::ReadMemory< uintptr_t >( ptr_to_ptr_table );

	return( 0 != ptr_to_player );
}

PSUPlayer &PSUPlayer::Get()
{
	static PSUPlayer dummyInstance;
	static PSUPlayer *memoryInstance = &dummyInstance;

	if( !GetPlayerPtr() )
	{
		return dummyInstance;
	}
	else [[likely]]
	{
		memoryInstance = PSUMemory::ReadAbsolutePtr< PSUPlayer >( ptr_to_player );

		if( !memoryInstance )
		{
			printf( "Failed to read player data pointer.\n" );
			return dummyInstance;
		}

		return *memoryInstance;
	}
}