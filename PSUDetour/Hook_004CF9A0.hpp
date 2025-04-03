/*
* This hook is responsible for spawning loot items onto the ground.
* It's also fucked and always returns false, so I don't use it.
*/

#pragma once

#include "../PSUEx.h"

namespace sub_004CF9A0
{
	typedef bool( __cdecl *functionType )( void *item );

	functionType pOriginalFunction = reinterpret_cast< functionType >( 0x004CF9A0 );

	bool sub_004CF9A0( void *item )
	{
		//printf( "sub_TEMPORARY %p %p\n", item, &item );

		//printf( "Drop Item ID: %08X\n", _byteswap_ulong( (item ).item_id.as_i32 ) );

		bool result = pOriginalFunction( item );

		printf( "Result: %d\n", result );

		return result;
	}

	void Hook()
	{
		DetourAttach( &( PVOID & )pOriginalFunction, reinterpret_cast< PVOID >( sub_004CF9A0 ) );
	}
}