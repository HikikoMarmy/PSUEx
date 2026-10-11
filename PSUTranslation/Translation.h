/*
* Translation file parser and lookup.
*
* A language file, Lang\<LANGUAGE>\<LANGUAGE>.json (LANGUAGE in [custom] in
* PsuIlluminus.ini):
*
*   {
*     "name": "...",
*     "exact": { "group": { "English": "translation", ... }, ... },
*     "patterns": [ [ "English with {}", "translation with {}" ], ... ]
*   }
*
* "exact" text is taken as it is (JSON's own escapes only); entries may also
* sit in "exact" directly, outside a group. "patterns" are tried in order;
* `{}` matches any text, `{1}`, `{2}` pick a match by position, and `\{`,
* `\}`, `\\` are a literal brace or backslash.
*
* Or, with no LANGUAGE, translation.txt: one entry a line,
* `English => translation`, with the same `{}`, and escapes
* \n \r \t \u{XXXX} \\ \{ \}. Lines starting with # are comments.
*/

#pragma once

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class Translations {
public:
	// Parses a UTF-8 translation.txt; bad lines are skipped with a warning.
	void Parse( const std::string &file, std::vector< std::string > &warnings );

	// Parses a language .json; bad entries are skipped with a warning.
	// `name` gets the file's "name" (the language, for the log), if any.
	void ParseJson( const std::string &file, std::vector< std::string > &warnings, std::string &name );

	// Adds one entry; returns an error, or an empty string.
	std::string Add( const std::wstring &from, const std::wstring &to );

	// Adds one exact entry, taken as it is (no escapes, no {}).
	void AddExact( const std::wstring &from, const std::wstring &to )
	{
		m_exact[ from ] = to;
		m_translations.insert( to );
	}

	// Whether `s` is an exact entry's translation (a translated string
	// drawn again, as the NPC boxes' text is).
	bool IsTranslation( const std::wstring &s ) const { return m_translations.count( s ) != 0; }

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
	std::unordered_set< std::wstring > m_translations;
	std::vector< Pattern > m_patterns;
};

namespace Translation
{
	// Read from the game folder when no LANGUAGE is set.
	constexpr const char *FILE_NAME = "translation.txt";
	// Language folders, in the game folder: Lang\<LANGUAGE>\.
	constexpr const char *LANG_FOLDER = "Lang";

	// LANGUAGE from [custom] in PsuIlluminus.ini, or empty if unset or not a
	// plain name (letters, digits, - and _, up to 16).
	const std::string &Language();

	// The file translations are read from: Lang\<LANGUAGE>\<LANGUAGE>.json,
	// or translation.txt if no LANGUAGE is set.
	const std::string &FilePath();
	// Written to the game folder.
	constexpr const char *LOG_NAME = "translation.log";

	// Hooks are installed only if this is true.
	bool FileExists();

	// Reads the translation file into a new table.
	void Load();

	// Reloads the file if it changed (checked once a second).
	void ReloadIfChanged();

	// Null-terminated translation (never freed), or nullptr.
	const wchar_t *Translated( const wchar_t *s );

	// English for a shop search typed in Japanese.
	bool EnglishFor( const std::wstring &s, std::wstring &out );

	// Appends a line to translation.log.
	void Log( const char *fmt, ... );

	// Written to the game folder with LOG_UNTRANSLATED=1.
	constexpr const char *UNTRANSLATED_NAME = "untranslated.txt";

	// Writes each distinct string drawn that has a letter in it and no
	// translation (up to 1,000) to untranslated.txt, as a line ready to fill
	// in: for a language file, a JSON entry for "exact" (`"English": "",`),
	// else a translation.txt line (`English => `). A string that is itself a
	// translation, or has kana or kanji, isn't written (it's translated text
	// drawn again). Only after the login: nothing is written before the
	// game says "Acquiring character information." (or its Japanese), and it
	// stops again at the login screen's prompt or ID check (English or
	// Japanese), so what's typed at login (the PSU ID) stays out of it.
	void SetLogUntranslated( bool on );
}
