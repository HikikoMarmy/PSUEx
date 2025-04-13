#pragma once

#include <string>
#include <stringapiset.h>

namespace StringUtil
{
	std::string WideToUTF8( const wchar_t *wideString )
	{
		if( !wideString ) return {};

		// Length without the null terminator
		int wideLen = static_cast< int >( wcslen( wideString ) );
		if( wideLen == 0 ) return {};

		// Get required size
		int size = WideCharToMultiByte( CP_UTF8, 0, wideString, wideLen, nullptr, 0, nullptr, nullptr );
		if( size <= 0 ) return {};

		std::string utf8String( size, 0 );
		WideCharToMultiByte( CP_UTF8, 0, wideString, wideLen, utf8String.data(), size, nullptr, nullptr );
		return utf8String;
	}
}