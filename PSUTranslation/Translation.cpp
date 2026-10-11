#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <cstdarg>
#include <cstdio>
#include <cwchar>
#include <cwctype>
#include <fstream>
#include <mutex>
#include <sstream>
#include <unordered_set>

#include "Translation.h"
#include "../Json.hpp"
#include "../PSUIni.hpp"

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
		AddExact( a, b );
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

void Translations::ParseJson( const std::string &file, std::vector< std::string > &warnings, std::string &name )
{
	using json = nlohmann::json;
	json root = json::parse( file, nullptr, false );
	if( root.is_discarded() || !root.is_object() )
	{
		warnings.push_back( "not a JSON object (check for a missing comma or quote)" );
		return;
	}

	auto label = root.find( "name" );
	if( label != root.end() && label->is_string() ) name = label->get< std::string >();

	auto exact = root.find( "exact" );
	if( exact != root.end() && exact->is_object() )
	{
		for( auto &[ key, value ] : exact->items() )
		{
			if( value.is_string() )
			{
				AddExact( Widen( key ), Widen( value.get< std::string >() ) );
				continue;
			}
			if( !value.is_object() )
			{
				warnings.push_back( "exact \"" + key + "\": not a translation or a group of them" );
				continue;
			}
			for( auto &[ from, to ] : value.items() )
			{
				if( to.is_string() ) AddExact( Widen( from ), Widen( to.get< std::string >() ) );
				else warnings.push_back( "exact \"" + key + "\" \"" + from + "\": the translation isn't text" );
			}
		}
	}
	else if( exact != root.end() )
	{
		warnings.push_back( "\"exact\" isn't an object" );
	}

	auto patterns = root.find( "patterns" );
	if( patterns != root.end() && patterns->is_array() )
	{
		size_t n = 0;
		for( auto &entry : *patterns )
		{
			n++;
			if( !entry.is_array() || entry.size() != 2 || !entry[ 0 ].is_string() || !entry[ 1 ].is_string() )
			{
				warnings.push_back( "pattern " + std::to_string( n ) + ": not [ \"English\", \"translation\" ]" );
				continue;
			}
			std::string e = Add( Widen( entry[ 0 ].get< std::string >() ), Widen( entry[ 1 ].get< std::string >() ) );
			if( !e.empty() ) warnings.push_back( "pattern " + std::to_string( n ) + ": " + e );
		}
	}
	else if( patterns != root.end() )
	{
		warnings.push_back( "\"patterns\" isn't a list" );
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

	// LOG_UNTRANSLATED: on, logged in (between the two strings below), and
	// the strings written so far (at most MAX_LOGGED).
	constexpr size_t MAX_LOGGED = 1000;
	// The game's own messages, in English and in Japanese (the Japanese
	// install's data): logging starts once the character list is loading,
	// and stops at the login screen's prompt or its ID check, so what's
	// typed at login (the PSU ID) stays out of it.
	static const wchar_t *const LOGGED_IN[] = {
		L"Acquiring character information.",
		L"\u30AD\u30E3\u30E9\u30AF\u30BF\u30FC\u60C5\u5831\u3092\u53D6\u5F97\u3057\u3066\u3044\u307E\u3059\u3002",
	};
	static const wchar_t *const LOGGING_IN[] = {
		L"Checking PSU ID and Password.",
		L"PSU ID \u306E\u8A8D\u8A3C\u3092\u884C\u3063\u3066\u3044\u307E\u3059\u3002",
		L"PSU ID\u3068\u30D1\u30B9\u30EF\u30FC\u30C9\u3092\u5165\u529B\u3057\u3066\u4E0B\u3055\u3044\u3002\n"
		L"\u5165\u529B\u3055\u308C\u305F\u30D1\u30B9\u30EF\u30FC\u30C9\u306F\u300C*\u300D\u3067\u8868\u793A\u3055\u308C\u307E\u3059\u3002",
	};
	static bool g_logUntranslated = false;
	static bool g_loggedIn = false;
	static std::unordered_set< std::wstring > g_logged;

	void SetLogUntranslated( bool on )
	{
		std::lock_guard< std::mutex > guard( g_lock );
		g_logUntranslated = on;
	}

	// `s` as a translation file's English: escapes for \\, {, }, line breaks,
	// tabs and the game's codes (control and private-use characters), so the
	// line reads back as `s`.
	static std::wstring Escape( const std::wstring &s )
	{
		std::wstring out;
		for( wchar_t c : s )
		{
			if( c == L'\\' || c == L'{' || c == L'}' )
			{
				out += L'\\';
				out += c;
			}
			else if( c == L'\n' ) out += L"\\n";
			else if( c == L'\r' ) out += L"\\r";
			else if( c == L'\t' ) out += L"\\t";
			else if( c < 0x20 || ( c >= 0x7F && c < 0xA0 ) || ( c >= 0xE000 && c <= 0xF8FF ) )
			{
				wchar_t code[ 16 ];
				swprintf( code, 16, L"\\u{%x}", static_cast< unsigned >( c ) );
				out += code;
			}
			else out += c;
		}
		return out;
	}

	// `s` as a JSON string's contents: escapes for " and \, line breaks,
	// tabs and the game's codes (control and private-use characters).
	static std::wstring EscapeJson( const std::wstring &s )
	{
		std::wstring out;
		for( wchar_t c : s )
		{
			if( c == L'"' || c == L'\\' )
			{
				out += L'\\';
				out += c;
			}
			else if( c == L'\n' ) out += L"\\n";
			else if( c == L'\r' ) out += L"\\r";
			else if( c == L'\t' ) out += L"\\t";
			else if( c < 0x20 || ( c >= 0x7F && c < 0xA0 ) || ( c >= 0xE000 && c <= 0xF8FF ) )
			{
				wchar_t code[ 16 ];
				swprintf( code, 16, L"\\u%04x", static_cast< unsigned >( c ) );
				out += code;
			}
			else out += c;
		}
		return out;
	}

	static bool IsJson();

	// Writes `s` to untranslated.txt the first time it's seen, if it has a
	// letter in it. Called with g_lock held.
	static void NoteUntranslated( const std::wstring &s )
	{
		if( !g_logUntranslated || !g_loggedIn || g_logged.size() >= MAX_LOGGED ) return;
		// A translation drawn again (the NPC boxes' wrapper swaps its
		// string before 0x004BB310 sees it) isn't untranslated: one of the
		// file's translations, or (for one made by a {} line) text with kana
		// or kanji.
		if( g_table && g_table->IsTranslation( s ) ) return;
		bool letter = false;
		for( wchar_t c : s )
		{
			if( ( c >= 0x3040 && c <= 0x30FF ) || ( c >= 0x4E00 && c <= 0x9FFF ) || ( c >= 0xFF00 && c <= 0xFFEF ) ) return;
			if( iswalpha( c ) ) letter = true;
		}
		if( !letter || !g_logged.insert( s ).second ) return;

		FILE *f = fopen( UNTRANSLATED_NAME, "ab" );
		if( !f ) return;
		// A byte order mark on a new file, so Notepad reads it as UTF-8.
		fseek( f, 0, SEEK_END );
		if( ftell( f ) == 0 ) fputs( "\xEF\xBB\xBF", f );
		std::string line = IsJson()
			? "\"" + Narrow( EscapeJson( s ) ) + "\": \"\",\r\n"
			: Narrow( Escape( s ) ) + " => \r\n";
		fwrite( line.data(), 1, line.size(), f );
		fclose( f );
		if( g_logged.size() == 1 ) Log( "writing text with no translation to %s", UNTRANSLATED_NAME );
		if( g_logged.size() == MAX_LOGGED ) Log( "%u untranslated strings written; no more this session", ( unsigned )MAX_LOGGED );
	}

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

	const std::string &Language()
	{
		static const std::string language = []() -> std::string
		{
			std::wstring code = PSUIni::Get().ReadString( L"custom", L"LANGUAGE", L"" );
			if( code.empty() ) return {};
			// A plain name only: it's used in a path.
			bool plain = code.size() <= 16;
			for( wchar_t c : code )
			{
				if( !( ( c >= L'a' && c <= L'z' ) || ( c >= L'A' && c <= L'Z' ) || ( c >= L'0' && c <= L'9' ) || c == L'-' || c == L'_' ) )
					plain = false;
			}
			if( !plain )
			{
				Log( "LANGUAGE=%s isn't a language folder's name (letters, digits, - and _); no language set", Narrow( code ).c_str() );
				return {};
			}
			return Narrow( code );
		}();
		return language;
	}

	const std::string &FilePath()
	{
		static const std::string path = Language().empty()
			? std::string( FILE_NAME )
			: std::string( LANG_FOLDER ) + "\\" + Language() + "\\" + Language() + ".json";
		return path;
	}

	static bool IsJson()
	{
		return !Language().empty();
	}

	static bool FileTime( FILETIME &out )
	{
		WIN32_FILE_ATTRIBUTE_DATA data;
		if( !GetFileAttributesExA( FilePath().c_str(), GetFileExInfoStandard, &data ) ) return false;
		out = data.ftLastWriteTime;
		return true;
	}

	bool FileExists()
	{
		FILETIME t;
		if( FileTime( t ) ) return true;
		if( IsJson() ) Log( "LANGUAGE=%s but there's no %s; translation off", Language().c_str(), FilePath().c_str() );
		return false;
	}

	void Load()
	{
		FILETIME time = {};
		FileTime( time );

		const char *name = FilePath().c_str();
		std::ifstream in( name, std::ios::binary );
		if( !in )
		{
			Log( "%s: can't read it", name );
			return;
		}
		std::stringstream text;
		text << in.rdbuf();

		auto table = new Translations();
		std::vector< std::string > warnings;
		std::string language;
		if( IsJson() ) table->ParseJson( text.str(), warnings, language );
		else table->Parse( text.str(), warnings );
		for( size_t i = 0; i < warnings.size() && i < 20; i++ ) Log( "%s: %s", name, warnings[ i ].c_str() );
		if( warnings.size() > 20 ) Log( "%s: %u more warnings", name, ( unsigned )( warnings.size() - 20 ) );
		// The language's own name, if the file says it: "Lang\ja\ja.json (Japanese)".
		std::string shown = language.empty() ? std::string( name ) : std::string( name ) + " (" + language + ")";
		Log( "%s: %u exact entries, %u patterns", shown.c_str(), ( unsigned )table->ExactCount(), ( unsigned )table->PatternCount() );

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
			Log( "%s changed; reading it again", FilePath().c_str() );
			Load();
		}
	}

	// Description boxes (the GAS counter's, NPCs' and counters') draw a
	// string that starts with a box code, 0xF701 or 0xF702, followed by one
	// character the code takes (often a 0), and then the text: the game skips
	// both and draws on. The text is translated, the two kept in front.
	static bool StartsWithBoxCode( const wchar_t *s )
	{
		return s[ 0 ] == 0xF701 || s[ 0 ] == 0xF702;
	}

	static const wchar_t *TranslatedAfterBoxCode( const wchar_t *s );

	const wchar_t *Translated( const wchar_t *s )
	{
		if( StartsWithBoxCode( s ) ) return TranslatedAfterBoxCode( s );
		std::wstring key( s );
		std::lock_guard< std::mutex > guard( g_lock );
		if( g_logUntranslated )
		{
			for( const wchar_t *marker : LOGGED_IN )
				if( key == marker ) g_loggedIn = true;
			for( const wchar_t *marker : LOGGING_IN )
				if( key == marker ) g_loggedIn = false;
		}
		if( !g_table ) return nullptr;

		auto hit = g_hits.find( key );
		if( hit != g_hits.end() ) return hit->second;
		if( g_misses.count( key ) ) return nullptr;

		std::wstring to;
		if( !g_table->Translate( key, to ) )
		{
			NoteUntranslated( key );
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

	// Longest text read after a box code, in UTF-16 units.
	constexpr size_t MAX_BOX_TEXT = 1024;

	static const wchar_t *TranslatedAfterBoxCode( const wchar_t *s )
	{
		const wchar_t *rest = s + 2;
		size_t length = wcsnlen( rest, MAX_BOX_TEXT );
		if( length == 0 || length >= MAX_BOX_TEXT ) return nullptr;
		std::wstring text( rest, length );
		// The cache key keeps the code and its character, so the same text
		// after another code gets its own copy.
		std::wstring key = std::wstring( s, 2 ) + text;
		std::lock_guard< std::mutex > guard( g_lock );
		if( !g_table ) return nullptr;

		auto hit = g_hits.find( key );
		if( hit != g_hits.end() ) return hit->second;
		if( g_misses.count( key ) ) return nullptr;

		std::wstring to;
		if( !g_table->Translate( text, to ) )
		{
			NoteUntranslated( text );
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
		wchar_t *kept = new wchar_t[ 2 + to.size() + 1 ];
		kept[ 0 ] = s[ 0 ];
		kept[ 1 ] = s[ 1 ];
		memcpy( kept + 2, to.c_str(), ( to.size() + 1 ) * sizeof( wchar_t ) );
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
