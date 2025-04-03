/*
* This hook is responsible for catching chat messages as they're logged.
*/

#pragma once

#include "../PSUEx.h"
#include "../PSUOverlay/Interface/LogDisplay.h"

#include <iostream>

namespace sub_50B790
{
	struct s_log_message {
		wchar_t name[ 32 ];
		wchar_t msg[ 127 ];
	};

	typedef FILE* ( __cdecl *function50B790 )( int guardian_id, wchar_t *source, char message_type, void* esi );
	function50B790 pOriginal50B790 = reinterpret_cast< function50B790 >( 0x0050B790 );

	FILE* __cdecl LogChatMessage( int guardian_id, wchar_t *source, char message_type, void* esi )
	{
		__asm
		{
			push esi  // Preserve the original ESI
			mov esi, esi  // Ensure ESI is set before calling the function
		}

		printf( "[DEBUG] LogChatMessage - Guardian ID: %d\n", guardian_id );
		printf( "[DEBUG] LogChatMessage - ESI Pointer: %p\n", esi );
		printf( "[DEBUG] LogChatMessage - Source Pointer: %p\n", source );
		printf( "[DEBUG] LogChatMessage - Source String: %S\n", source );

		FILE *result = pOriginal50B790( guardian_id, source, message_type, esi );

		__asm pop esi  // Restore original ESI

		printf( "[DEBUG] LogChatMessage - After Call Source: %S\n", source );
		return result;
	}

	typedef void( __cdecl *function59AC30 )( double, int );
	function59AC30 pOriginal59AC30 = reinterpret_cast< function59AC30 >( 0x0059AC30 );

	void DumpMemory( int address, int size )
	{
		for( int i = 0; i < size; i++ )
		{
			printf( "%02X ", *( char * )( address + i ) );

			if( i % 16 == 15 )
				printf( "\n" );
		}
	}

	void __cdecl Hook_59AC30( double a1, int a2 )
	{
		// Dump Memory
		DumpMemory( a2, 0x100 );

		if( a2 )
		{
			// Check if the chat message buffer is valid
			unsigned __int16 *messageBuffer = ( unsigned __int16 * )( a2 + 132 );
			if( messageBuffer )
			{
				// Convert the message to a wstring (assuming it's a wide string)
				std::wstring message( reinterpret_cast< wchar_t * >( messageBuffer ) );

				// Print the message to the console (or log to a file if preferred)
				std::wcout << L"[Chat Message Intercepted]: " << message << std::endl;
			}
		}

		// Call the original function
		pOriginal59AC30( a1, a2 );
	}


	void Hook()
	{
		//DetourAttach( &( PVOID & )pOriginal59AC30, reinterpret_cast< PVOID >( Hook_59AC30 ) );
		//DetourAttach( &( PVOID & )pOriginal50B790, reinterpret_cast< PVOID >( LogChatMessage ) );
	}
}