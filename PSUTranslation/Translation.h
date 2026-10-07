/*
* Translation file parser and lookup.
*
* One entry a line: `English => translation`. `{}` matches any text;
* `{1}`, `{2}` pick a match by position. Escapes: \n \r \t \u{XXXX} \\ \{ \}.
* Lines starting with # are comments.
*/

#pragma once

#include <string>
#include <unordered_map>
#include <vector>

class Translations {
public:
	// Parses a UTF-8 file; bad lines are skipped with a warning.
	void Parse( const std::string &file, std::vector< std::string > &warnings );

	// Adds one entry; returns an error, or an empty string.
	std::string Add( const std::wstring &from, const std::wstring &to );

	// Exact entry first, then patterns in file order.
	bool Translate( const std::wstring &s, std::wstring &out ) const;

	// Reverse lookup for shop search: exact match, or common English prefix.
	bool EnglishFor( const std::wstring &s, std::wstring &out ) const;

	size_t ExactCount() const { return m_exact.size(); }
	size_t PatternCount() const { return m_patterns.size(); }

private:
	// Text, or a hole (index -1 for {}, N - 1 for {N}).
	struct Piece {
		bool m_hole;
		int m_index;
		std::wstring m_text;
	};

	struct Pattern {
		// Literal text around the holes.
		std::vector< std::wstring > m_parts;
		std::vector< Piece > m_to;
	};

	static std::string Pieces( const std::wstring &s, std::vector< Piece > &out );
	static bool Matches( const std::vector< std::wstring > &parts, const std::wstring &s, std::vector< std::wstring > &caught );

	std::unordered_map< std::wstring, std::wstring > m_exact;
	std::vector< Pattern > m_patterns;
};

namespace Translation
{
	// Read from the game folder.
	constexpr const char *FILE_NAME = "translation.txt";
	// Written to the game folder.
	constexpr const char *LOG_NAME = "translation.log";

	// Hooks are installed only if this is true.
	bool FileExists();

	// Reads translation.txt into a new table.
	void Load();

	// Reloads the file if it changed (checked once a second).
	void ReloadIfChanged();

	// Null-terminated translation (never freed), or nullptr.
	const wchar_t *Translated( const wchar_t *s );

	// English for a shop search typed in Japanese.
	bool EnglishFor( const std::wstring &s, std::wstring &out );

	// Appends a line to translation.log.
	void Log( const char *fmt, ... );
}
