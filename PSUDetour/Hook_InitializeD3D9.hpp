/*
* This hook is responsible for grabbing the D3D9 interface and initializing the overlay.
* We grab it just as the game creates it.
*/

#pragma once

#include <intrin.h>
#include "../PSUEx.h"

namespace sub_776590
{
	typedef char( __cdecl *FunctionInitializeDirect3D )( int a1 );
    FunctionInitializeDirect3D m_pOriginalInitializeDirect3D = reinterpret_cast< FunctionInitializeDirect3D >( 0x00776590 );

	char __cdecl InitializeDirect3D( int a1 )
	{
        printf( "Hook_CreateClientWindow called!\n" );

        char result = m_pOriginalInitializeDirect3D( a1 );

        IDirect3D9 *g_pD3D = *( IDirect3D9 ** )0x00A888C0;

        if( g_pD3D )
        {
            printf( "Successfully retrieved g_pD3D: %p\n", g_pD3D );
			PSUOverlay::Get().Initialize( g_pD3D );
        }
        else
        {
            printf( "Failed to retrieve g_pD3D.\n" );
        }

        return result;
	}

	void Hook()
	{
		DetourAttach( &( PVOID & )m_pOriginalInitializeDirect3D, reinterpret_cast< PVOID >( InitializeDirect3D ) );
	}
}