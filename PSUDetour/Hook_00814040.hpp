/*
* This hook is responsible for loading files from the /DATA/ folder.
*/

#pragma once

#include <filesystem>
#include "../PSUEx.h"
#include "../PSUModManager/ModManager.hpp"

namespace sub_814040
{
	// Loads a file handle with a fully qualified path.
	typedef HANDLE *( __cdecl *function814040 )( LPCSTR lpFileName );
	function814040 pOriginal814040 = reinterpret_cast< function814040 >( 0x00814040 );

	// Concatenates the file path with the filename, or some shit idk. Useless for now.
	typedef CHAR *( __cdecl *function8131D0 )( char *a1, char *a2 );
	function8131D0 pOriginal8131D0 = reinterpret_cast< function8131D0 >( 0x008131D0 );

	CHAR *sub_8131D0( char *a1, char *a2 )
	{
		auto ret = pOriginal8131D0( a1, a2 );

		return ret;
	}

	HANDLE sub_814040( LPCSTR lpFileName )
	{
		std::filesystem::path path( lpFileName );
		ModManager::GetInstance().GetModPath( &lpFileName, path.filename().string() );

		return pOriginal814040( lpFileName );
	}

	void Hook()
	{
		DetourAttach( &( PVOID & )pOriginal814040, reinterpret_cast< PVOID >( sub_814040 ) );
		//DetourAttach( &( PVOID & )pOriginal8131D0, reinterpret_cast< PVOID >( sub_8131D0 ) );
	}
}