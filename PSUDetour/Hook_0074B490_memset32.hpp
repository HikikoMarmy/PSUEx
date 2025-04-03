/*
* This hook is for a memset32 function that is used in the game.
* We use it to check if the source pointer is NULL,
* and if the size is less than or equal to 0 to avoid edgecase crashes in the game.
*/

#pragma once

#include "../PSUEx.h"

namespace sub_74B490
{
	typedef int ( __cdecl *function74B490 )( void *ptr, signed int size, int value );
	function74B490 pOriginal74B490 = reinterpret_cast< function74B490 >( 0x0074B490 );

	int sub_74B490( void *ptr, signed int size, int value )
	{
		if( !ptr || size <= 0 )
		{
			printf( "Skipping memset32." );
			return 0;
		}

		return pOriginal74B490( ptr, size, value );
	}

	void Hook()
	{
		DetourAttach( &( PVOID & )pOriginal74B490, reinterpret_cast< PVOID >( sub_74B490 ) );
	}
}