/*
* This hook is responsible for resolving and allocating flags.
* Unfortunately it doesn't quite work yet, so it's disabled.
*/

#pragma once

#include "../PSUEx.h"

namespace TFlagManager
{
	static std::vector<std::string> numericFlags;
	static std::vector<std::string> accountFlags;
	static std::vector<std::string> booleanFlags;

	typedef char( __cdecl *InitializeServerFlags_t )( char *a1 );
	InitializeServerFlags_t pOriginal735950 = reinterpret_cast< InitializeServerFlags_t >( 0x00735950 );

	typedef int( __thiscall *GetFlagIndex_t )( const char *a1 );
	GetFlagIndex_t pOriginal7359E0 = reinterpret_cast< GetFlagIndex_t >( 0x007359E0 );
	GetFlagIndex_t pOriginal735A20 = reinterpret_cast< GetFlagIndex_t >( 0x00735A20 );
	GetFlagIndex_t pOriginal735A60 = reinterpret_cast< GetFlagIndex_t >( 0x00735A60 );

	void ClearAllFlags()
	{
		numericFlags.clear();
		accountFlags.clear();
		booleanFlags.clear();
	}

	char __cdecl InitializeServerFlags( char *a1 )
	{
		if( !a1 )
		{
			return 0;
		}

		int32_t packet_size = *reinterpret_cast< int32_t * >( a1 );
		int16_t command = _byteswap_ushort( *reinterpret_cast< int16_t * >( a1 + 4 ) );

		// Sometimes the function is called with garbage. Don't know why.
		if( command != 0x0D05 )
		{
			return 0;
		}

		// Read the number of each flag type from the last 12 bytes
		char *end_of_packet = static_cast< char * >( a1 ) + packet_size - 12;
		unsigned int nb_numeric = *reinterpret_cast< uint32_t * >( end_of_packet );
		unsigned int nb_boolean = *reinterpret_cast< uint32_t * >( end_of_packet + 4 );
		unsigned int nb_account = *reinterpret_cast< uint32_t * >( end_of_packet + 8 );

		// Clear existing flags
		numericFlags.clear();
		accountFlags.clear();
		booleanFlags.clear();
		
		// Resize vectors
		numericFlags.resize( nb_numeric );
		accountFlags.resize( nb_account );
		booleanFlags.resize( nb_boolean );

		// Move past the header
		char *data = static_cast< char * >( a1 ) + 44;

		// Read numeric flags
		for( unsigned int i = 0; i < nb_numeric; i++ )
		{
			numericFlags[ i ] = std::string( data + ( i * 16 ), 16 );
		}

		// Read boolean flags
		char *booleanData = data + ( nb_numeric * 16 );
		for( unsigned int i = 0; i < nb_boolean; i++ )
		{
			booleanFlags[ i ] = std::string( booleanData + ( i * 16 ), 16 );
		}

		// Read account flags
		char *accountData = booleanData + ( nb_boolean * 16 );
		for( unsigned int i = 0; i < nb_account; i++ )
		{
			accountFlags[ i ] = std::string( accountData + ( i * 16 ), 16 );
		}

		return 1;
	}

	int __fastcall GetNumericFlagIndex( const char *a1 )
	{
		if( !a1 )
			return -1;

		for( size_t i = 0; i < numericFlags.size(); i++ )
		{
			if( strncmp( a1, numericFlags[ i ].c_str(), 16 ) == 0 )
			{
				return static_cast< int >( i );
			}
		}

		return -1;
	}

	int __fastcall GetAccountFlagIndex( const char *a1 )
	{
		if( !a1 )
			return -1;

		for( size_t i = 0; i < accountFlags.size(); i++ )
		{
			if( strncmp( a1, accountFlags[ i ].c_str(), 16 ) == 0 )
			{
				return static_cast< int >( i );
			}
		}

		return -1;
	}

	int __fastcall GetBooleanFlagIndex( const char *a1 )
	{
		if( !a1 )
		{
			return -1;
		}

		for( size_t i = 0; i < booleanFlags.size(); i++ )
		{
			if( strncmp( a1, booleanFlags[ i ].c_str(), 16 ) == 0 )
			{
				return static_cast< int >( i );
			}
		}
		
		return -1;
	}

	void Hook()
	{
		//DetourAttach( &( PVOID & )pOriginal735950, reinterpret_cast< PVOID >( InitializeServerFlags ) );
		//DetourAttach( &( PVOID & )pOriginal7359E0, reinterpret_cast< PVOID >( GetNumericFlagIndex ) );
		//DetourAttach( &( PVOID & )pOriginal735A20, reinterpret_cast< PVOID >( GetBooleanFlagIndex ) );
		//DetourAttach( &( PVOID & )pOriginal735A60, reinterpret_cast< PVOID >( GetAccountFlagIndex ) );
	}
}