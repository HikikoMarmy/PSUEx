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
		int ResolutionIndex = PSUIni::Get().ReadInt( L"screen", L"resolution", 0 );
		int DisplayIndex = PSUIni::Get().ReadInt( L"custom", L"DISPLAY_INDEX", 0 );
		int RequestedX = PSUIni::Get().ReadInt( L"custom", L"RESOLUTION_WIDTH", 640 );
		int RequestedY = PSUIni::Get().ReadInt( L"custom", L"RESOLUTION_HEIGHT", 480 );

		std::vector<MONITORINFOEX> monitors;
		auto MonitorEnumProc = []( HMONITOR hMonitor, HDC, LPRECT, LPARAM lParam ) -> BOOL
		{
			auto *pMonitors = reinterpret_cast< std::vector<MONITORINFOEX>* >( lParam );
			MONITORINFOEX mi;
			mi.cbSize = sizeof( mi );
			if( GetMonitorInfo( hMonitor, &mi ) )
			{
				pMonitors->push_back( mi );
			}
			return TRUE;
		};
		EnumDisplayMonitors( nullptr, nullptr, MonitorEnumProc, reinterpret_cast< LPARAM >( &monitors ) );

		MONITORINFOEX targetMonitor{};
		if( !monitors.empty() && DisplayIndex >= 0 && DisplayIndex < monitors.size() )
		{
			targetMonitor = monitors[ DisplayIndex ];
		}
		else
		{
			// fallback to primary monitor
			targetMonitor.cbSize = sizeof( targetMonitor );
			GetMonitorInfo( MonitorFromPoint( { 0, 0 }, MONITOR_DEFAULTTOPRIMARY ), &targetMonitor );
		}

		int monitorWidth = targetMonitor.rcMonitor.right - targetMonitor.rcMonitor.left;
		int monitorHeight = targetMonitor.rcMonitor.bottom - targetMonitor.rcMonitor.top;

		// Clamp requested resolution to monitor bounds to prevent bleeding
		int ScreenResolutionX = min( RequestedX, monitorWidth );
		int ScreenResolutionY = min( RequestedY, monitorHeight );

		// Center the window on the monitor
		int PosX = targetMonitor.rcMonitor.left + ( monitorWidth - ScreenResolutionX ) / 2;
		int PosY = targetMonitor.rcMonitor.top + ( monitorHeight - ScreenResolutionY ) / 2;

		// Calculate aspect ratio
		float AspectRatio = static_cast< float >( ScreenResolutionX ) / static_cast< float >( ScreenResolutionY );

		// Save to globals, we'll need them later
		Global::screenWidth = ScreenResolutionX;
		Global::screenHeight = ScreenResolutionY;
		Global::windowPosX = PosX;
		Global::windowPosY = PosY;
		Global::monitorLeft = targetMonitor.rcMonitor.left;
		Global::monitorTop = targetMonitor.rcMonitor.top;
		Global::monitorRight = targetMonitor.rcMonitor.right;
		Global::monitorBottom = targetMonitor.rcMonitor.bottom;

		// Write the new resolution to the memory
		// This is important because the client uses these values elsewhere directly
		PSUMemory::Get().WriteMemory( offset_standard_fov, AspectRatio );
		PSUMemory::Get().WriteMemory( offset_standard_ratio, AspectRatio );
		PSUMemory::Get().WriteMemory( offset_standard_x, ScreenResolutionX );
		PSUMemory::Get().WriteMemory( offset_standard_y, ScreenResolutionY );

		PSUMemory::Get().WriteMemory( offset_widescreen_fov, AspectRatio );
		PSUMemory::Get().WriteMemory( offset_widescreen_ratio, AspectRatio );
		PSUMemory::Get().WriteMemory( offset_widescreen_x, ScreenResolutionX );
		PSUMemory::Get().WriteMemory( offset_widescreen_y, ScreenResolutionY );
	}

	REGISTER_PATCH( ApplyPatch );
}

