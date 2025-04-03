/*
* Just a sandbox for testing UI scaling hooks.
* 
* In theory it should be possible to scale the matrix used for UI rendering
* but needs more investigation.
*/

#pragma once

#include "../PSUEx.h"

namespace UIScaling
{
	typedef int( __thiscall *function793960 )( float *thisPtr, float width, float height );
	function793960 pOriginal793960 = reinterpret_cast< function793960 >( 0x00793960 );

	typedef int( __cdecl *function793B90 )( float width, float height );
	function793B90 pOriginal793B90 = reinterpret_cast< function793B90 >( 0x00793B90 );

	typedef void( __thiscall *function5802A0 )( int thisPtr, void *a2 );
	function5802A0 pOriginal5802A0 = reinterpret_cast< function5802A0 >( 0x005802A0 );

	typedef void( __cdecl *function76E1D0 )( float *outWidth, float *outHeight );
	function76E1D0 pOriginal_GetScreenSize_76E1D0 = reinterpret_cast< function76E1D0 >( 0x0076E1D0 );

	typedef double( __cdecl *function793B80 )( );
	function793B80 pOriginal793B80 = reinterpret_cast< function793B80 >( 0x00793B80 );

	int __fastcall Hooked_sub_793960( float *thisPtr, float width, float height )
	{
		if( !thisPtr ) return 0;

		//auto returnAddress = ( uintptr_t )_ReturnAddress();
		//printf( "Call Address: 0x%p\n", ( void * )returnAddress );

		auto result = pOriginal793960( thisPtr, width, height );

		thisPtr[ 0 ] = width;
		thisPtr[ 1 ] = height;
		thisPtr[ 2 ] = 1.0f / width;	// Causes base window shift
		thisPtr[ 3 ] = 1.0f / height;	// Causes base window shift
		thisPtr[ 4 ] = 0.0f;			// Causes window element shift
		thisPtr[ 5 ] = 0.0f;			// Causes window element shift
		thisPtr[ 6 ] = thisPtr[ 3 ] * width;  // aspect ratio

		//printf( "thisPtr Data: %f %f %f %f %f %f %f\n", thisPtr[ 0 ], thisPtr[ 1 ], thisPtr[ 2 ], thisPtr[ 3 ], thisPtr[ 4 ], thisPtr[ 5 ], thisPtr[ 6 ] );

		return static_cast< int >( width );
	}

	int __cdecl Hooked_sub_793B90( float width, float height )
	{
		// Makes texture canvas bigger but not placement.
		if( width == 640 && height == 480 )
		{
			//auto returnAddress = ( uintptr_t )_ReturnAddress();

			//printf( "Call Address: 0x%p\n", ( void * )returnAddress );

			//width = width * 1.25f;
			//height = height * 1.25f;
		}
		return pOriginal793B90( width, height );  // Force a resolution
	}

	void __fastcall Hooked_sub_5802A0( int thisPtr, void *a2 )
	{
		if( !a2 )
		{
			printf( "[HOOK] sub_5802A0: screenSize is NULL!\n" );
			return;
		}

		// Try treating `a2` as a pointer to a pointer
		void **doublePtr = reinterpret_cast< void ** >( a2 );

		if( IsBadReadPtr( doublePtr, sizeof( void * ) ) )
		{
			printf( "[HOOK] sub_5802A0: a2 is an invalid pointer!\n" );
			pOriginal5802A0( thisPtr, a2 );
			return;
		}

		// Read the actual float* pointer
		float *screenSize = reinterpret_cast< float * >( *doublePtr );

		// Double-check valid memory before accessing
		if( IsBadReadPtr( screenSize, sizeof( float ) * 2 ) )
		{
			printf( "[HOOK] sub_5802A0: Invalid memory access!\n" );
			pOriginal5802A0( thisPtr, a2 );
			return;
		}

		printf( "[HOOK] sub_5802A0 called!\n" );
		printf( "Original UI Size: %f %f\n ", screenSize[ 0 ], screenSize[ 1 ] );

		// Override screen size to prevent shrinking
		screenSize[ 0 ] = 1920.0f;  // Force UI to match screen width
		screenSize[ 1 ] = 1080.0f;  // Force UI to match screen height

		// Call original function with modified values
		pOriginal5802A0( thisPtr, a2 );
	}

	void __cdecl Hooked_GetScreenSize_76E1D0( float *outWidth, float *outHeight )
	{
		//printf( "[HOOK] GetScreenSize_76E1D0 called!\n" );
		//printf( "Call Address: 0x%p\n", _ReturnAddress() );

		auto returnAddress = ( uintptr_t )_ReturnAddress();

		pOriginal_GetScreenSize_76E1D0( outWidth, outHeight );

		if( returnAddress == 0x005D426E ||	//x
			returnAddress == 0x004BA40B ||	//x
			returnAddress == 0x0058A5C1 ||
			returnAddress == 0x00571417 )
		{
			// Return 1920x1080 for the main menu
			//*outWidth = *outWidth * 1.1;
			//*outHeight = *outHeight * 1.1;

			//printf( "Call Address: 0x%p\n", (void*)returnAddress );
		}
	}

	void Hook()
	{
		//DetourAttach( &( PVOID & )pOriginal793960, reinterpret_cast< PVOID >( Hooked_sub_793960 ) );
		//DetourAttach( &( PVOID & )pOriginal793B90, reinterpret_cast< PVOID >( Hooked_sub_793B90 ) );
		//DetourAttach( &( PVOID & )pOriginal5802A0, reinterpret_cast< PVOID >( Hooked_sub_5802A0 ) );
		//DetourAttach( &( PVOID & )pOriginal_GetScreenSize_76E1D0, reinterpret_cast< PVOID >( Hooked_GetScreenSize_76E1D0 ) );
		//DetourAttach( &( PVOID & )pOriginal793B80, reinterpret_cast< PVOID >( Hooked_sub_793B80 ) );
	}
}