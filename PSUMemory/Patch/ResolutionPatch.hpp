#pragma once

namespace PSUHook::ResolutionPatch
{
	const static uint32_t offset_standard_ratio = 0x0046FDD0;
	const static uint32_t offset_standard_x = 0x0046FDD4;
	const static uint32_t offset_standard_y = 0x0046FDD8;
	const static uintptr_t offset_standard_fov = 0x0049D4F4;

	const static uint32_t offset_widescreen_ratio = 0x0046FDCC;
	const static uint32_t offset_widescreen_x = 0x0046FDFC;
	const static uint32_t offset_widescreen_y = 0x0046FE00;
	const static uintptr_t offset_widescreen_fov = 0x0049D4F8;

	const static uintptr_t offset_notice_y = 0x004D57C8;

	void ApplyPatch()
	{
		int ResolutionIndex		= PSUIni::Get().ReadInt( L"screen", L"resolution", 0 );
		int ScreenResolutionX	= PSUIni::Get().ReadInt( L"custom", L"RESOLUTION_WIDTH", 640 );
		int ScreenResolutionY	= PSUIni::Get().ReadInt( L"custom", L"RESOLUTION_HEIGHT", 480 );

		if( ScreenResolutionX > GetSystemMetrics( SM_CXSCREEN ) ||
			ScreenResolutionY > GetSystemMetrics( SM_CYSCREEN ) )
		{
			ScreenResolutionX = GetSystemMetrics( SM_CXSCREEN );
			ScreenResolutionY = GetSystemMetrics( SM_CYSCREEN );
		}

		float AspectRatio = ( float )ScreenResolutionX / ( float )ScreenResolutionY;

		//if( 0 == ResolutionIndex )	// 4:3
		{
			PSUMemory::Get().WriteMemory( offset_standard_fov, AspectRatio );
			PSUMemory::Get().WriteMemory( offset_standard_ratio, AspectRatio );
			PSUMemory::Get().WriteMemory( offset_standard_x, ScreenResolutionX );
			PSUMemory::Get().WriteMemory( offset_standard_y, ScreenResolutionY );
		}
		//else if( 5 == ResolutionIndex )	// 16:9/16:10
		{
			PSUMemory::Get().WriteMemory( offset_widescreen_fov, AspectRatio );
			PSUMemory::Get().WriteMemory( offset_widescreen_ratio, AspectRatio );
			PSUMemory::Get().WriteMemory( offset_widescreen_x, ScreenResolutionX );
			PSUMemory::Get().WriteMemory( offset_widescreen_y, ScreenResolutionY );
		}
	}

	REGISTER_PATCH( ApplyPatch );
}

