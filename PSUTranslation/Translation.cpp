#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <cstdarg>
#include <cstdio>
#include <fstream>
#include <mutex>
#include <sstream>
#include <unordered_set>

#include "Translation.h"

static std::wstring Widen( const std::string &s )
{
	if( s.empty() ) return {};
	int n = MultiByteToWideChar( CP_UTF8, 0, s.data(), ( int )s.size(), nullptr, 0 );
	std::wstring out( n, L'\0' );
	MultiByteToWideChar( CP_UTF8, 0, s.data(), ( int )s.size(), out.data(), n );
	return out;
}

static std::string Narrow( const std::wstring &s )
{
	if( s.empty() ) return {};
	int n = WideCharToMultiByte( CP_UTF8, 0, s.data(), ( int )s.size(), nullptr, 0, nullptr, nullptr );
	std::string out( n, '\0' );
	WideCharToMultiByte( CP_UTF8, 0, s.data(), ( int )s.size(), out.data(), n, nullptr, nullptr );
	return out;
}

std::string Translations::Pieces( const std::wstring &s, std::vector< Piece > &out )
{
	std::wstring text;
	auto flush = [ & ]()
	{
		if( !text.empty() ) out.push_back( { false, 0, text } );
		text.clear();
	};

	for( size_t i = 0; i < s.size(); i++ )
	{
		wchar_t c = s[ i ];
		if( c == L'\\' )
		{
			if( ++i >= s.size() ) return "a \\ at the end of the line";
			switch( s[ i ] )
			{
				case L'n': text += L'\n'; break;
				case L'r': text += L'\r'; break;
				case L't': text += L'\t'; break;
				case L'\\': case L'{': case L'}': text += s[ i ]; break;
				case L'u':
				{
					if( ++i >= s.size() || s[ i ] != L'{' ) return "\\u needs a code in braces, like \\u{f805}";
					size_t end = s.find( L'}', i );
					std::wstring hex = s.substr( i + 1, end == std::wstring::npos ? std::wstring::npos : end - i - 1 );
					wchar_t *stop = nullptr;
					unsigned long code = hex.empty() ? 0 : wcstoul( hex.c_str(), &stop, 16 );
					if( end == std::wstring::npos || hex.empty() || *stop || code > 0x10FFFF || ( code >= 0xD800 && code <= 0xDFFF ) )
						return "\\u{" + Narrow( hex ) + "} isn't a character code";
					if( code >= 0x10000 )
					{
						code -= 0x10000;
						text += ( wchar_t )( 0xD800 + ( code >> 10 ) );
						text += ( wchar_t )( 0xDC00 + ( code & 0x3FF ) );
					}
					else
					{
						text += ( wchar_t )code;
					}
					i = end;
					break;
				}
				default:
					return "\\" + Narrow( std::wstring( 1, s[ i ] ) ) + " isn't an escape (\\n, \\r, \\t, \\u{..}, \\\\, \\{, \\})";
			}
		}
		else if( c == L'{' )
		{
			size_t end = s.find( L'}', i );
			if( end == std::wstring::npos ) return "a { without its }";
			std::wstring inner = s.substr( i + 1, end - i - 1 );
			int index = -1;
			if( !inner.empty() )
			{
				wchar_t *stop = nullptr;
				long n = wcstol( inner.c_str(), &stop, 10 );
				if( *stop || n < 1 || inner[ 0 ] == L'+' || inner[ 0 ] == L'-' )
					return "{" + Narrow( inner ) + "} isn't {} or {1}, {2}, ...";
				index = ( int )n - 1;
			}
			flush();
			out.push_back( { true, index, {} } );
			i = end;
		}
		else if( c == L'}' )
		{
			return "a } without its {";
		}
		else
		{
			text += c;
		}
	}
	flush();
	return {};
}

std::string Translations::Add( const std::wstring &fromText, const std::wstring &toText )
{
	std::vector< Piece > from, to;
	std::string e = Pieces( fromText, from );
	if( e.empty() ) e = Pieces( toText, to );
	if( !e.empty() ) return e;
	if( from.empty() ) return "the English is empty";

	size_t holes = 0;
	for( auto &p : from )
	{
		if( p.m_hole && p.m_index >= 0 ) return "the English can only use {}, not {1}";
		if( p.m_hole ) holes++;
	}

	if( holes == 0 )
	{
		std::wstring a, b;
		for( auto &p : to )
		{
			if( p.m_hole ) return "the translation has {} but the English doesn't";
			b += p.m_text;
		}
		for( auto &p : from ) a += p.m_text;
		m_exact[ a ] = b;
		return {};
	}

	size_t inOrder = 0;
	for( auto &p : to )
	{
		if( !p.m_hole ) continue;
		if( p.m_index < 0 && ++inOrder > holes )
			return "the translation has more {} than the English's " + std::to_string( holes );
		if( p.m_index >= ( int )holes )
			return "{" + std::to_string( p.m_index + 1 ) + "} but the English has " + std::to_string( holes ) + " {}";
	}

	Pattern pattern;
	pattern.m_parts.emplace_back();
	for( auto &p : from )
	{
		if( p.m_hole ) pattern.m_parts.emplace_back();
		else pattern.m_parts.back() += p.m_text;
	}
	for( size_t i = 1; i + 1 < pattern.m_parts.size(); i++ )
	{
		if( pattern.m_parts[ i ].empty() ) return "two {} side by side can't be told apart";
	}
	pattern.m_to = std::move( to );
	m_patterns.push_back( std::move( pattern ) );
	return {};
}

void Translations::Parse( const std::string &file, std::vector< std::string > &warnings )
{
	std::istringstream in( file );
	std::string line;
	for( int n = 1; std::getline( in, line ); n++ )
	{
		// Strip BOM and CR.
		if( line.compare( 0, 3, "\xEF\xBB\xBF" ) == 0 ) line.erase( 0, 3 );
		if( !line.empty() && line.back() == '\r' ) line.pop_back();

		size_t first = line.find_first_not_of( " \t" );
		if( first == std::string::npos || line[ first ] == '#' ) continue;

		size_t arrow = line.find( " => " );
		if( arrow == std::string::npos )
		{
			warnings.push_back( "line " + std::to_string( n ) + ": no \" => \" between the English and the translation" );
			continue;
		}
		std::string e = Add( Widen( line.substr( 0, arrow ) ), Widen( line.substr( arrow + 4 ) ) );
		if( !e.empty() ) warnings.push_back( "line " + std::to_string( n ) + ": " + e );
	}
}

// Fills caught with each hole's text; holes match as little as possible.
bool Translations::Matches( const std::vector< std::wstring > &parts, const std::wstring &s, std::vector< std::wstring > &caught )
{
	const std::wstring &first = parts.front(), &last = parts.back();
	if( s.size() < first.size() + last.size() ) return false;
	if( s.compare( 0, first.size(), first ) != 0 ) return false;
	if( s.compare( s.size() - last.size(), last.size(), last ) != 0 ) return false;

	std::wstring body = s.substr( first.size(), s.size() - first.size() - last.size() );
	caught.clear();
	size_t at = 0;
	for( size_t i = 1; i + 1 < parts.size(); i++ )
	{
		size_t found = body.find( parts[ i ], at );
		if( found == std::wstring::npos ) return false;
		caught.push_back( body.substr( at, found - at ) );
		at = found + parts[ i ].size();
	}
	caught.push_back( body.substr( at ) );
	return true;
}

bool Translations::Translate( const std::wstring &s, std::wstring &out ) const
{
	auto exact = m_exact.find( s );
	if( exact != m_exact.end() )
	{
		out = exact->second;
		return true;
	}

	std::vector< std::wstring > caught;
	for( auto &p : m_patterns )
	{
		if( !Matches( p.m_parts, s, caught ) ) continue;
		out.clear();
		size_t next = 0;
		for( auto &piece : p.m_to )
		{
			if( !piece.m_hole )
			{
				out += piece.m_text;
				continue;
			}
			const std::wstring &c = caught[ piece.m_index < 0 ? next++ : ( size_t )piece.m_index ];
			auto own = m_exact.find( c );
			out += own != m_exact.end() ? own->second : c;
		}
		return true;
	}
	return false;
}

bool Translations::EnglishFor( const std::wstring &s, std::wstring &out ) const
{
	if( s.empty() ) return false;

	const std::wstring *best = nullptr;
	for( auto &[ from, to ] : m_exact )
	{
		if( to == s && ( !best || from.size() < best->size() ) ) best = &from;
	}
	if( best )
	{
		out = *best;
		return true;
	}

	// Case-insensitive, like the server's search.
	auto lower = []( wchar_t c ) { return c >= L'A' && c <= L'Z' ? ( wchar_t )( c + 32 ) : c; };
	bool any = false;
	std::wstring common;
	for( auto &[ from, to ] : m_exact )
	{
		if( to.compare( 0, s.size(), s ) != 0 ) continue;
		if( !any )
		{
			common = from;
			any = true;
			continue;
		}
		size_t n = 0;
		while( n < common.size() && n < from.size() && lower( common[ n ] ) == lower( from[ n ] ) ) n++;
		common.resize( n );
	}
	while( !common.empty() && common.back() == L' ' ) common.pop_back();
	if( !any || common.size() < 3 ) return false;
	out = common;
	return true;
}

namespace Translation
{
	// Cap on cached translations.
	constexpr size_t MAX_TRANSLATED = 20000;
	// Cap on remembered misses (cleared when full).
	constexpr size_t MAX_MISSES = 4096;

	static std::mutex g_lock;
	static Translations *g_table = nullptr;
	static std::unordered_map< std::wstring, const wchar_t * > g_hits;
	static std::unordered_set< std::wstring > g_misses;
	static size_t g_kept = 0;
	static bool g_full = false;
	static FILETIME g_fileTime = {};
	static DWORD g_lastCheck = 0;

	void Log( const char *fmt, ... )
	{
		FILE *f = fopen( LOG_NAME, "a" );
		if( !f ) return;
		SYSTEMTIME t;
		GetLocalTime( &t );
		fprintf( f, "%02d:%02d:%02d ", t.wHour, t.wMinute, t.wSecond );
		va_list args;
		va_start( args, fmt );
		vfprintf( f, fmt, args );
		va_end( args );
		fputc( '\n', f );
		fclose( f );
	}

	static bool FileTime( FILETIME &out )
	{
		WIN32_FILE_ATTRIBUTE_DATA data;
		if( !GetFileAttributesExA( FILE_NAME, GetFileExInfoStandard, &data ) ) return false;
		out = data.ftLastWriteTime;
		return true;
	}

	bool FileExists()
	{
		FILETIME t;
		return FileTime( t );
	}

	void Load()
	{
		FILETIME time = {};
		FileTime( time );

		std::ifstream in( FILE_NAME, std::ios::binary );
		if( !in )
		{
			Log( "%s: can't read it", FILE_NAME );
			return;
		}
		std::stringstream text;
		text << in.rdbuf();

		auto table = new Translations();
		std::vector< std::string > warnings;
		table->Parse( text.str(), warnings );
		for( size_t i = 0; i < warnings.size() && i < 20; i++ ) Log( "%s: %s", FILE_NAME, warnings[ i ].c_str() );
		if( warnings.size() > 20 ) Log( "%s: %u more warnings", FILE_NAME, ( unsigned )( warnings.size() - 20 ) );
		Log( "%s: %u exact entries, %u patterns", FILE_NAME, ( unsigned )table->ExactCount(), ( unsigned )table->PatternCount() );

		std::lock_guard< std::mutex > guard( g_lock );
		// Old table is leaked on purpose: the game may still use its strings.
		g_table = table;
		g_hits.clear();
		g_misses.clear();
		g_fileTime = time;
	}

	void ReloadIfChanged()
	{
		DWORD now = GetTickCount();
		{
			std::lock_guard< std::mutex > guard( g_lock );
			if( now - g_lastCheck < 1000 ) return;
			g_lastCheck = now;
		}
		FILETIME time;
		if( FileTime( time ) && CompareFileTime( &time, &g_fileTime ) != 0 )
		{
			Log( "%s changed; reading it again", FILE_NAME );
			Load();
		}
	}

	const wchar_t *Translated( const wchar_t *s )
	{
		std::wstring key( s );
		std::lock_guard< std::mutex > guard( g_lock );
		if( !g_table ) return nullptr;

		auto hit = g_hits.find( key );
		if( hit != g_hits.end() ) return hit->second;
		if( g_misses.count( key ) ) return nullptr;

		std::wstring to;
		if( !g_table->Translate( key, to ) )
		{
			if( g_misses.size() >= MAX_MISSES ) g_misses.clear();
			g_misses.insert( std::move( key ) );
			return nullptr;
		}
		if( g_kept >= MAX_TRANSLATED )
		{
			if( !g_full ) Log( "%u translated strings kept; new ones are left untranslated", ( unsigned )MAX_TRANSLATED );
			g_full = true;
			return nullptr;
		}
		// Never freed: the game may keep the pointer.
		wchar_t *kept = new wchar_t[ to.size() + 1 ];
		memcpy( kept, to.c_str(), ( to.size() + 1 ) * sizeof( wchar_t ) );
		g_kept++;
		g_hits.emplace( std::move( key ), kept );
		return kept;
	}

	bool EnglishFor( const std::wstring &s, std::wstring &out )
	{
		std::lock_guard< std::mutex > guard( g_lock );
		return g_table && g_table->EnglishFor( s, out );
	}
}
