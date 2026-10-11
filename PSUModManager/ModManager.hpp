#pragma once

#include <string>
#include <unordered_map>
#include <filesystem>
#include "../PSUTranslation/Translation.h"

class ModManager
{
private:
	std::unordered_map< std::string, std::string > m_mods;

public:
	static ModManager &GetInstance()
	{
		static ModManager instance;
		return instance;
	}

	ModManager( ModManager const & ) = delete;
	void operator=( ModManager const & ) = delete;

	ModManager()
	{
		m_mods.clear();

		// The language's data files first, so a file in Addon wins.
		const std::string &language = Translation::Language();
		if( !language.empty() )
		{
			std::string folder = std::string( ".\\" ) + Translation::LANG_FOLDER + "\\" + language + "\\data";
			std::error_code error;
			if( std::filesystem::is_directory( folder, error ) )
				Translation::Log( "%s: %u data files", folder.c_str(), ( unsigned )AddFolder( folder ) );
		}

		AddFolder( ".\\Addon" );
	}

	// Adds the files named like DATA's in folder; returns how many.
	size_t AddFolder( const std::string &folder )
	{
		size_t added = 0;
		for( const auto &entry : std::filesystem::recursive_directory_iterator( folder ) )
		{
			if( !entry.is_regular_file() ) continue;

			// Lazy check for if the file name is an MD5 hash.
			if( entry.path().filename().string().length() == 32 )
			{
				std::string modName = entry.path().filename().string();
				std::string modPath = entry.path().string();
				m_mods[ modName ] = modPath;
				added++;

				printf( "Apply Mod: %s\n", modName.c_str() );
			}
		}
		return added;
	}

	void GetModPath( LPCSTR *originalPath, const std::string &modName )
	{
		if( m_mods.find( modName ) != m_mods.end() )
		{
			printf( "Load Modded NBL: %s : %s\n", modName.c_str(), m_mods[ modName ].c_str() );
			strncpy_s( ( char * )*originalPath, 260, m_mods[ modName ].c_str(), m_mods[ modName ].length() );
		}
	}
};