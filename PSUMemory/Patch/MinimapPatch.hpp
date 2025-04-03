#pragma once

#include <map>

namespace PSUHook::MinimapPatch
{
	struct s_minimapData
	{
		float x;
		float y;
		float scale_x;
		float scale_y;
	};

	const static std::map< std::string, s_minimapData > minimapData =
	{
		{ "1360x768", { 0.7550f, -0.6850f, 0.6000f, 0.6000f } },
		{ "1366x768", { 0.7550f, -0.6850f, 0.6000f, 0.6000f } },
		{ "1440x900", { 0.7600f, -0.7000f, 0.6000f, 0.6000f } },
		{ "1600x900", { 0.7700f, -0.7000f, 0.6000f, 0.6000f } },
		{ "1680x1050", { 0.7750f, -0.7300f, 0.4500f, 0.4500f } },
		{ "1920x1080", { 0.7850f, -0.7200f, 0.4500f, 0.4500f } },
		{ "1920x1200", { 0.7850f, -0.7200f, 0.4500f, 0.4500f } },
		{ "2560x1080", { 0.8000f, -0.7300f, 0.4000f, 0.4000f } },
		{ "2560x1440", { 0.8000f, -0.7500f, 0.4000f, 0.4000f } },
		{ "3440x1440", { 0.8110f, -0.7500f, 0.3500f, 0.3500f } },
		{ "3840x1600", { 0.8165f, -0.7800f, 0.2800f, 0.2800f } },
		{ "3840x1080", { 0.8165f, -0.7800f, 0.2800f, 0.2800f } },
		{ "3840x2160", { 0.8165f, -0.7800f, 0.2800f, 0.2800f } },
		{ "5120x1440", { 0.8200f, -0.8000f, 0.2500f, 0.2500f } }
	};

	const static uintptr_t offset_minimap_X = 0x004A9FDC;
	const static uintptr_t offset_minimap_Y = 0x004A9FE0;
	const static uintptr_t offset_mapscale_X = 0x004AA03C;
	const static uintptr_t offset_mapscale_Y = 0x004AA040;

	void ApplyPatch()
	{
		int DisplayResolutionX = PSUIni::Get().ReadInt( L"custom", L"RESOLUTION_WIDTH", 0 );
		int DisplayResolutionY = PSUIni::Get().ReadInt( L"custom", L"RESOLUTION_HEIGHT", 0 );

		std::string DisplayResolution = std::to_string( DisplayResolutionX ) + "x" + std::to_string( DisplayResolutionY );

		auto it = minimapData.find( DisplayResolution );
		if( it == minimapData.end() )
		{
			printf( "MinimapPatch: Resolution not found: %s\n", DisplayResolution.c_str() );
			return;
		}

		PSUMemory::Get().WriteMemory( offset_minimap_X, it->second.x );
		PSUMemory::Get().WriteMemory( offset_minimap_Y, it->second.y );
		PSUMemory::Get().WriteMemory( offset_mapscale_X, it->second.scale_x );
		PSUMemory::Get().WriteMemory( offset_mapscale_Y, it->second.scale_y );
	}

	REGISTER_PATCH( ApplyPatch );
}