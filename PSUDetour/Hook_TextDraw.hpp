/*
* Japanese translation hooks, active only when translation.txt is present.
*
* - Draw text (0x004BB310) and measure text (0x004BCAC0): swap strings that
*   have a translation.
* - Game send hook (0x00A8E0EC): shop searches typed in Japanese are sent in
*   English.
*/

#pragma once

#include "../PSUEx.h"
#include "../PSUTranslation/Translation.h"

namespace TextDraw
{
	// Drawing settings passed by value; the string is words[2].
	struct DrawSettings {
		uint32_t words[ 16 ];
	};

	typedef void *( __thiscall *DrawText_t )( void *thisPtr, void *out, DrawSettings settings, int last );
	DrawText_t pOriginal_DrawText = reinterpret_cast< DrawText_t >( 0x004BB310 );

	typedef void *( __cdecl *MeasureText_t )( void *out, const wchar_t *text, uint32_t a, int b );
	MeasureText_t pOriginal_MeasureText = reinterpret_cast< MeasureText_t >( 0x004BCAC0 );

	// Expected first bytes, checked before hooking.
	const uint8_t DRAW_TEXT_CODE[] = { 0x81, 0xEC, 0xE4, 0x00, 0x00, 0x00, 0xD9, 0x84, 0x24, 0x04, 0x01, 0x00, 0x00 };
	const uint8_t MEASURE_TEXT_CODE[] = { 0x83, 0xEC, 0x10, 0x8B, 0x44, 0x24, 0x1C, 0x55, 0x8B, 0x6C, 0x24, 0x1C };

	// Channel::Send reads the send hook pointer at SEND_HOOK_SLOT.
	const uintptr_t CHANNEL_SEND = 0x0079B6D0;
	const uint8_t CHANNEL_SEND_CODE[] = { 0xA1, 0xEC, 0xE0, 0xA8, 0x00, 0x85, 0xC0, 0x53, 0x56, 0x57, 0x8B, 0x7C, 0x24, 0x10, 0x8B, 0xF1 };
	const uintptr_t SEND_HOOK_SLOT = 0x00A8E0EC;

	// Shop search packet.
	const uint32_t SEARCH_SIZE = 0x84;
	const uint16_t SEARCH_OPCODE = 0x3713;
	const size_t SEARCH_NAME = 0x44;

	// Longest string handled, in UTF-16 units.
	const size_t MAX_TEXT = 1024;

	inline bool CodeIs( uintptr_t address, const uint8_t *code, size_t size )
	{
		return memcmp( reinterpret_cast< const void * >( address ), code, size ) == 0;
	}

	// Skips small (non-pointer) values and overlong strings.
	inline bool IsText( const wchar_t *p )
	{
		return reinterpret_cast< uintptr_t >( p ) >= 0x10000 && wcsnlen( p, MAX_TEXT ) < MAX_TEXT;
	}

	void *__fastcall Hook_DrawText( void *thisPtr, void *edx, void *out, DrawSettings settings, int last )
	{
		auto text = reinterpret_cast< const wchar_t * >( settings.words[ 2 ] );
		if( IsText( text ) )
		{
			Translation::ReloadIfChanged();
			if( auto to = Translation::Translated( text ) )
				settings.words[ 2 ] = reinterpret_cast< uint32_t >( to );
		}
		return pOriginal_DrawText( thisPtr, out, settings, last );
	}

	// Measure the translation so boxes fit the drawn text.
	void *__cdecl Hook_MeasureText( void *out, const wchar_t *text, uint32_t a, int b )
	{
		if( IsText( text ) )
		{
			if( auto to = Translation::Translated( text ) )
				text = to;
		}
		return pOriginal_MeasureText( out, text, a, b );
	}

	// Called by Channel::Send with each outgoing packet, before encryption.
	void __cdecl Hook_GameSend( uint8_t *packet )
	{
		if( !packet ) return;
		uint32_t size;
		uint16_t opcode;
		memcpy( &size, packet, 4 );
		memcpy( &opcode, packet + 4, 2 );
		if( size != SEARCH_SIZE || opcode != SEARCH_OPCODE ) return;

		// English names are sent as typed.
		const size_t room = ( SEARCH_SIZE - SEARCH_NAME ) / 2;
		std::wstring typed;
		bool ascii = true;
		for( size_t i = 0; i < room; i++ )
		{
			wchar_t c;
			memcpy( &c, packet + SEARCH_NAME + i * 2, 2 );
			if( !c ) break;
			if( c >= 0x80 ) ascii = false;
			typed += c;
		}
		if( ascii ) return;

		std::wstring english;
		// Must fit the field with its terminator.
		if( !Translation::EnglishFor( typed, english ) || english.size() >= room ) return;
		memset( packet + SEARCH_NAME, 0, SEARCH_SIZE - SEARCH_NAME );
		memcpy( packet + SEARCH_NAME, english.c_str(), english.size() * 2 );

		std::string en( english.begin(), english.end() );
		Translation::Log( "shop search typed in Japanese sent as \"%s\"", en.c_str() );
	}

	void InstallSendHook()
	{
		if( !CodeIs( CHANNEL_SEND, CHANNEL_SEND_CODE, sizeof( CHANNEL_SEND_CODE ) ) )
		{
			Translation::Log( "Channel::Send isn't the expected code; Japanese shop search off" );
			return;
		}
		auto slot = reinterpret_cast< uintptr_t * >( SEND_HOOK_SLOT );
		if( *slot != 0 )
		{
			Translation::Log( "the game's send hook is already set (%08X); Japanese shop search off", ( unsigned )*slot );
			return;
		}
		DWORD old;
		if( !VirtualProtect( slot, 4, PAGE_READWRITE, &old ) )
		{
			Translation::Log( "can't write the game's send hook; Japanese shop search off" );
			return;
		}
		*slot = reinterpret_cast< uintptr_t >( &Hook_GameSend );
		VirtualProtect( slot, 4, old, &old );
		Translation::Log( "Japanese shop search on" );
	}

	void Hook()
	{
		if( !Translation::FileExists() ) return;

		if( !CodeIs( reinterpret_cast< uintptr_t >( pOriginal_DrawText ), DRAW_TEXT_CODE, sizeof( DRAW_TEXT_CODE ) ) )
		{
			Translation::Log( "the text drawing routine isn't the expected code; translation off" );
			return;
		}
		Translation::Load();

		DetourAttach( &( PVOID & )pOriginal_DrawText, reinterpret_cast< PVOID >( Hook_DrawText ) );
		if( CodeIs( reinterpret_cast< uintptr_t >( pOriginal_MeasureText ), MEASURE_TEXT_CODE, sizeof( MEASURE_TEXT_CODE ) ) )
			DetourAttach( &( PVOID & )pOriginal_MeasureText, reinterpret_cast< PVOID >( Hook_MeasureText ) );
		else
			Translation::Log( "the text measuring routine isn't the expected code; measuring not hooked" );

		InstallSendHook();
		Translation::Log( "text drawing hooked" );
	}
}
