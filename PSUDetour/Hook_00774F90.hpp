/*
* This hook is responsible for initializing DirectInput
*/

#pragma once

#include "../PSUEx.h"

namespace sub_774F90
{
	typedef HRESULT( __cdecl *functionType )();
	functionType pOriginalFunction = reinterpret_cast< functionType >( 0x00774F90 );

	HRESULT Hook_774F90()
	{
		printf( "Hooked 00774F90\n" );

		// Initialize DirectInput
		// Hopefully we can hook the controller detection logic here.
		return pOriginalFunction();
	}

	// This function is when the game draws most of the UI, it seems.
	// Tried scaling it but icky results.
    typedef int( __thiscall *sub_793C40_t )( void *thisPtr, int a4, int vertexCount, void *vertexPtr, int drawType );
	sub_793C40_t Original_sub_793C40 = reinterpret_cast< sub_793C40_t >( 0x00793C40 );

    int __fastcall Hook_sub_793C40( void *thisPtr, void * /*dummy*/, int a4, int vertexCount, void *vertexPtr, int drawType )
    {
        return Original_sub_793C40( thisPtr, a4, vertexCount, vertexPtr, drawType );
    }

	void Hook()
	{
		//DetourAttach( &( PVOID & )pOriginalFunction, reinterpret_cast< PVOID >( Hook_774F90 ) );
		//DetourAttach( &( PVOID & )Original_sub_793C40, reinterpret_cast< PVOID >( Hook_sub_793C40 ) );
	}
}