#pragma once

#include <string>
#include <filesystem>
#include <shlobj_core.h>

class PSUIni
{
private:
	inline static std::filesystem::path m_iniPath;
public:

	static PSUIni &Get()
	{
		static PSUIni instance;
		return instance;
	}

private:
	PSUIni()
	{
		wchar_t localAppData[ MAX_PATH ];
		SHGetFolderPathW( NULL, CSIDL_LOCAL_APPDATA, NULL, 0, localAppData );
		m_iniPath = std::filesystem::path( localAppData ) / L".\\SEGA\\PHANTASY STAR UNIVERSE Illuminus\\PsuIlluminus.ini";
	}

public:
	PSUIni( const PSUIni & ) = delete;
	PSUIni &operator=( const PSUIni & ) = delete;

	static bool ReadBool( const wchar_t *section, const wchar_t *key, bool defaultValue = false )
	{
		return GetPrivateProfileIntW( section, key, defaultValue, m_iniPath.c_str() ) != 0;
	}

	static int ReadInt( const wchar_t *section, const wchar_t *key, int defaultValue = 0 )
	{
		return GetPrivateProfileIntW( section, key, defaultValue, m_iniPath.c_str() );
	}

	static std::wstring ReadString( const wchar_t *section, const wchar_t *key, const wchar_t *defaultValue = L"" )
	{
		wchar_t buffer[ 256 ];
		GetPrivateProfileStringW( section, key, defaultValue, buffer, 256, m_iniPath.c_str() );
		return buffer;
	}

	static std::filesystem::path GetLocalPSUAppdata()
	{
		wchar_t localAppData[MAX_PATH];
		SHGetFolderPathW(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, localAppData);
		return std::filesystem::path(localAppData) / L".\\SEGA\\PHANTASY STAR UNIVERSE Illuminus\\";
	}
};

