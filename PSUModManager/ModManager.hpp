#pragma once

#include <string>
#include <unordered_map>
#include <filesystem>

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

		for( const auto &entry : std::filesystem::recursive_directory_iterator( ".\\Addon" ) )
		{
			if( !entry.is_regular_file() ) continue;

			// Lazy check for if the file name is an MD5 hash.
			if( entry.path().filename().string().length() == 32 )
			{
				std::string modName = entry.path().filename().string();
				std::string modPath = entry.path().string();
				m_mods[ modName ] = modPath;

				printf( "Apply Mod: %s\n", modName.c_str() );
			}
		}
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