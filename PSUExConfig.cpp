#include <fstream>

#include "PSUExConfig.h"
#include "Misc/Constants.h"
#include "Misc/Globals.h"
#include "Json.hpp"

using json = nlohmann::json;

Config::Config()
{
	ImGui::StyleColorsDark( &m_style );

	// Load the config file
	config.m_windowInfo.m_position = ImVec2( 0, 0 );
	config.m_windowInfo.m_size = ImVec2( 400, 300 );
	
	// Loot Reader
	loot_reader.m_windowInfo.m_position = ImVec2( Global::screenWidth - 320.0f, 180.0f );
	loot_reader.m_windowInfo.m_size = ImVec2( 300.0f, Global::screenHeight * 0.6f );

	loot_reader.m_minimumElementPercent = 0;
	loot_reader.m_minimumRarity = 0;
	loot_reader.m_maximumRarity = 14;
	loot_reader.m_colorItemByElement = true;
	loot_reader.m_displayMission = false;

	loot_reader.hover.m_displayItem = false;
	loot_reader.hover.m_displayFade = false;
	loot_reader.hover.m_fadeDistance = 200.0f;

	loot_reader.m_columnVisibility.fill( true );
	loot_reader.m_itemTypeFilter.fill( true );
	loot_reader.m_weaponTypeFilter.fill( true );
	loot_reader.m_elementFilter.fill( true );

	// Camera
	camera.m_windowInfo.m_position = ImVec2( 64.0f, 64.0f );

	camera.m_followDistance = 1.0f;
	camera.m_fov = 1.0f;
	camera.m_smoothing = Constant::Camera::FOLLOW_SMOOTHING;
	camera.m_followHeight = Constant::Camera::FOLLOW_HEIGHT;
	camera.m_rotationSpeedH = 1.0f;
	camera.m_rotationSpeedV = 1.0f;
	camera.m_refocusSpeed = Constant::Camera::REFOCUS_SPEED;
	camera.m_turnAcceleration = Constant::Camera::TURN_ACCELERATION;
}

Config::~Config()
{
}

void Config::SaveToFile()
{
	json root;

	// Loot Reader
	root[ "loot_reader" ][ "window_info" ][ "enabled" ] = loot_reader.m_windowInfo.m_enabled;
	root[ "loot_reader" ][ "window_info" ][ "x" ] = loot_reader.m_windowInfo.m_position.x;
	root[ "loot_reader" ][ "window_info" ][ "y" ] = loot_reader.m_windowInfo.m_position.y;
	root[ "loot_reader" ][ "window_info" ][ "width" ] = loot_reader.m_windowInfo.m_size.x;
	root[ "loot_reader" ][ "window_info" ][ "height" ] = loot_reader.m_windowInfo.m_size.y;

	root[ "loot_reader" ][ "minimumElementPercent" ] = loot_reader.m_minimumElementPercent;
	root[ "loot_reader" ][ "minimumRarity" ] = loot_reader.m_minimumRarity;
	root[ "loot_reader" ][ "maximumRarity" ] = loot_reader.m_maximumRarity;
	root[ "loot_reader" ][ "colorItemByElement" ] = loot_reader.m_colorItemByElement;
	root[ "loot_reader" ][ "displayMission" ] = loot_reader.m_displayMission;

	// Hover Item
	root[ "loot_reader" ][ "hover" ][ "displayItem" ] = loot_reader.hover.m_displayItem;
	root[ "loot_reader" ][ "hover" ][ "displayFade" ] = loot_reader.hover.m_displayFade;
	root[ "loot_reader" ][ "hover" ][ "fadeDistance" ] = loot_reader.hover.m_fadeDistance;
	root[ "loot_reader" ][ "hover" ][ "floatHeight" ] = loot_reader.hover.m_floatHeight;
	root[ "loot_reader" ][ "hover" ][ "displayElementIcon" ] = loot_reader.hover.m_displayElementIcon;
	root[ "loot_reader" ][ "hover" ][ "displayItemIcon" ] = loot_reader.hover.m_displayItemIcon;
	root[ "loot_reader" ][ "hover" ][ "colorItemByElement" ] = loot_reader.hover.m_colorItemByElement;
	root[ "loot_reader" ][ "hover" ][ "displayItemRarity" ] = loot_reader.hover.m_displayItemRarity;

	for( size_t i = 0; i < loot_reader.m_columnVisibility.size(); i++ )
	{
		root[ "loot_reader" ][ "columnVisibility" ][ i ] = loot_reader.m_columnVisibility[ i ];
	}

	for( size_t i = 0; i < loot_reader.m_itemTypeFilter.size(); i++ )
	{
		root[ "loot_reader" ][ "itemTypeFilter" ][ i ] = loot_reader.m_itemTypeFilter[ i ];
	}

	for( size_t i = 0; i < loot_reader.m_weaponTypeFilter.size(); i++ )
	{
		root[ "loot_reader" ][ "weaponTypeFilter" ][ i ] = loot_reader.m_weaponTypeFilter[ i ];
	}

	for( size_t i = 0; i < loot_reader.m_elementFilter.size(); i++ )
	{
		root[ "loot_reader" ][ "elementFilter" ][ i ] = loot_reader.m_elementFilter[ i ];
	}

	// Camera
	root[ "camera" ][ "window_info" ][ "enabled" ] = camera.m_windowInfo.m_enabled;
	root[ "camera" ][ "window_info" ][ "x" ] = camera.m_windowInfo.m_position.x;
	root[ "camera" ][ "window_info" ][ "y" ] = camera.m_windowInfo.m_position.y;
	root[ "camera" ][ "window_info" ][ "width" ] = camera.m_windowInfo.m_size.x;
	root[ "camera" ][ "window_info" ][ "height" ] = camera.m_windowInfo.m_size.y;

	root[ "camera" ][ "smoothing" ] = camera.m_smoothing;
	root[ "camera" ][ "refocusSpeed" ] = camera.m_refocusSpeed;
	root[ "camera" ][ "followDistance" ] = camera.m_followDistance;
	root[ "camera" ][ "followHeight" ] = camera.m_followHeight;
	root[ "camera" ][ "rotationSpeedH" ] = camera.m_rotationSpeedH;
	root[ "camera" ][ "rotationSpeedV" ] = camera.m_rotationSpeedV;
	root[ "camera" ][ "turnAcceleration" ] = camera.m_turnAcceleration;

	root[ "camera" ][ "fov" ] = camera.m_fov;

	// Save ImGui Style
	json styleJson;
	ImGuiStyle &style = ImGui::GetStyle();

	// Basic style settings
	styleJson[ "Alpha" ] = style.Alpha;
	styleJson[ "WindowPadding" ] = { style.WindowPadding.x, style.WindowPadding.y };
	styleJson[ "FramePadding" ] = { style.FramePadding.x, style.FramePadding.y };
	styleJson[ "ItemSpacing" ] = { style.ItemSpacing.x, style.ItemSpacing.y };
	styleJson[ "WindowRounding" ] = style.WindowRounding;
	styleJson[ "FrameRounding" ] = style.FrameRounding;
	styleJson[ "GrabRounding" ] = style.GrabRounding;

	// Colors
	for( int i = 0; i < ImGuiCol_COUNT; ++i )
	{
		const ImVec4 &col = style.Colors[ i ];
		styleJson[ "Colors" ][ i ] = { col.x, col.y, col.z, col.w };
	}

	root[ "imgui_style" ] = styleJson;

	std::ofstream file( "config.json" );
	if( file.is_open() )
	{
		file << root.dump( 4 );
		file.close();
	}
}

void Config::LoadFromFile()
{
	json root;

	std::ifstream file( "config.json" );
	if( file.is_open() )
	{
		file >> root;
		file.close();
	}
	else
	{
		printf( "Failed to open config file\n" );
		return;
	}

	printf( "Loaded config file\n" );

	// Loot Reader
	if( root.contains( "loot_reader" ) && root[ "loot_reader" ].is_object() )
	{
		auto &lootReaderNode = root[ "loot_reader" ];

		// Window Info Parsing
		if( lootReaderNode.contains( "window_info" ) && lootReaderNode[ "window_info" ].is_object() )
		{
			auto &windowInfo = lootReaderNode[ "window_info" ];

			if( windowInfo.contains( "enabled" ) && windowInfo[ "enabled" ].is_boolean() )
				loot_reader.m_windowInfo.m_enabled = windowInfo[ "enabled" ].get<bool>();

			if( windowInfo.contains( "x" ) && windowInfo[ "x" ].is_number() )
				loot_reader.m_windowInfo.m_position.x = windowInfo[ "x" ].get<float>();

			if( windowInfo.contains( "y" ) && windowInfo[ "y" ].is_number() )
				loot_reader.m_windowInfo.m_position.y = windowInfo[ "y" ].get<float>();

			if( windowInfo.contains( "width" ) && windowInfo[ "width" ].is_number() )
				loot_reader.m_windowInfo.m_size.x = windowInfo[ "width" ].get<float>();

			if( windowInfo.contains( "height" ) && windowInfo[ "height" ].is_number() )
				loot_reader.m_windowInfo.m_size.y = windowInfo[ "height" ].get<float>();
		}

		// Jesus christ, this is so clean and easy to read. - dave 2025
		if( lootReaderNode.contains( "minimumElementPercent" ) && lootReaderNode[ "minimumElementPercent" ].is_number() )
			loot_reader.m_minimumElementPercent = lootReaderNode[ "minimumElementPercent" ].get<int8_t>();

		if( lootReaderNode.contains( "minimumRarity" ) && lootReaderNode[ "minimumRarity" ].is_number() )
			loot_reader.m_minimumRarity = lootReaderNode[ "minimumRarity" ].get<int>();

		if( lootReaderNode.contains( "maximumRarity" ) && lootReaderNode[ "maximumRarity" ].is_number() )
			loot_reader.m_maximumRarity = lootReaderNode[ "maximumRarity" ].get<int>();

		if( lootReaderNode.contains( "colorItemByElement" ) && lootReaderNode[ "colorItemByElement" ].is_boolean() )
			loot_reader.m_colorItemByElement = lootReaderNode[ "colorItemByElement" ].get<bool>();

		if( lootReaderNode.contains( "displayMission" ) && lootReaderNode[ "displayMission" ].is_boolean() )
			loot_reader.m_displayMission = lootReaderNode[ "displayMission" ].get<bool>();

		if( lootReaderNode.contains( "hover" ) && lootReaderNode[ "hover" ].is_object() )
		{
			auto &hoverNode = lootReaderNode[ "hover" ];

			if( hoverNode.contains( "displayItem" ) && hoverNode[ "displayItem" ].is_boolean() )
				loot_reader.hover.m_displayItem = hoverNode[ "displayItem" ].get<bool>();

			if( hoverNode.contains( "displayFade" ) && hoverNode[ "displayFade" ].is_boolean() )
				loot_reader.hover.m_displayFade = hoverNode[ "displayFade" ].get<bool>();

			if( hoverNode.contains( "fadeDistance" ) && hoverNode[ "fadeDistance" ].is_number() )
				loot_reader.hover.m_fadeDistance = hoverNode[ "fadeDistance" ].get<float>();

			if( hoverNode.contains( "floatHeight" ) && hoverNode[ "floatHeight" ].is_number() )
				loot_reader.hover.m_floatHeight = hoverNode[ "floatHeight" ].get<float>();

			if( hoverNode.contains( "displayElementIcon" ) && hoverNode[ "displayElementIcon" ].is_boolean() )
				loot_reader.hover.m_displayElementIcon = hoverNode[ "displayElementIcon" ].get<bool>();

			if( hoverNode.contains( "displayItemIcon" ) && hoverNode[ "displayItemIcon" ].is_boolean() )
				loot_reader.hover.m_displayItemIcon = hoverNode[ "displayItemIcon" ].get<bool>();

			if( hoverNode.contains( "colorItemByElement" ) && hoverNode[ "colorItemByElement" ].is_boolean() )
				loot_reader.hover.m_colorItemByElement = hoverNode[ "colorItemByElement" ].get<bool>();

			if( hoverNode.contains( "displayItemRarity" ) && hoverNode[ "displayItemRarity" ].is_boolean() )
				loot_reader.hover.m_displayItemRarity = hoverNode[ "displayItemRarity" ].get<bool>();
		}

		// Arrays Parsing
		if( lootReaderNode.contains( "columnVisibility" ) && lootReaderNode[ "columnVisibility" ].is_array() )
		{
			auto &columnVisibilityArray = lootReaderNode[ "columnVisibility" ];
			for( size_t i = 0; i < loot_reader.m_columnVisibility.size() && i < columnVisibilityArray.size(); ++i )
			{
				if( columnVisibilityArray[ i ].is_boolean() )
					loot_reader.m_columnVisibility[ i ] = columnVisibilityArray[ i ].get<bool>();
			}
		}

		// It was like reading a harry potter. - Intern Jim 2005-2005
		if( lootReaderNode.contains( "itemTypeFilter" ) && lootReaderNode[ "itemTypeFilter" ].is_array() )
		{
			auto &itemTypeFilterArray = lootReaderNode[ "itemTypeFilter" ];
			for( size_t i = 0; i < loot_reader.m_itemTypeFilter.size() && i < itemTypeFilterArray.size(); ++i )
			{
				if( itemTypeFilterArray[ i ].is_boolean() )
					loot_reader.m_itemTypeFilter[ i ] = itemTypeFilterArray[ i ].get<bool>();
			}
		}

		if( lootReaderNode.contains( "weaponTypeFilter" ) && lootReaderNode[ "weaponTypeFilter" ].is_array() )
		{
			auto &weaponTypeFilterArray = lootReaderNode[ "weaponTypeFilter" ];
			for( size_t i = 0; i < loot_reader.m_weaponTypeFilter.size() && i < weaponTypeFilterArray.size(); ++i )
			{
				if( weaponTypeFilterArray[ i ].is_boolean() )
					loot_reader.m_weaponTypeFilter[ i ] = weaponTypeFilterArray[ i ].get<bool>();
			}
		}

		if( lootReaderNode.contains( "elementFilter" ) && lootReaderNode[ "elementFilter" ].is_array() )
		{
			auto &elementFilterArray = lootReaderNode[ "elementFilter" ];
			for( size_t i = 0; i < loot_reader.m_elementFilter.size() && i < elementFilterArray.size(); ++i )
			{
				if( elementFilterArray[ i ].is_boolean() )
					loot_reader.m_elementFilter[ i ] = elementFilterArray[ i ].get<bool>();
			}
		}
	}

	// Cool I love it. So clean and easy to read. - Brian 2025
	// Wow, it's so easy to read. I love it. - Steven Whelks UK 2025
	if( root.contains( "camera" ) && root[ "camera" ].is_object() )
	{
		auto &cameraNode = root[ "camera" ];

		// Window Info
		if( cameraNode.contains( "window_info" ) && cameraNode[ "window_info" ].is_object() )
		{
			auto &windowInfo = cameraNode[ "window_info" ];

			if( windowInfo.contains( "enabled" ) && windowInfo[ "enabled" ].is_boolean() )
				camera.m_windowInfo.m_enabled = windowInfo[ "enabled" ].get<bool>();

			if( windowInfo.contains( "x" ) && windowInfo[ "x" ].is_number() )
				camera.m_windowInfo.m_position.x = windowInfo[ "x" ].get<float>();

			if( windowInfo.contains( "y" ) && windowInfo[ "y" ].is_number() )
				camera.m_windowInfo.m_position.y = windowInfo[ "y" ].get<float>();

			if( windowInfo.contains( "width" ) && windowInfo[ "width" ].is_number() )
				camera.m_windowInfo.m_size.x = windowInfo[ "width" ].get<float>();

			if( windowInfo.contains( "height" ) && windowInfo[ "height" ].is_number() )
				camera.m_windowInfo.m_size.y = windowInfo[ "height" ].get<float>();
		}

		// Would read again! C+. - Jimmy 2025
		if( cameraNode.contains( "smoothing" ) && cameraNode[ "smoothing" ].is_number() )
			camera.m_smoothing = cameraNode[ "smoothing" ].get<float>();

		if( cameraNode.contains( "refocusSpeed" ) && cameraNode[ "refocusSpeed" ].is_number() )
			camera.m_refocusSpeed = cameraNode[ "refocusSpeed" ].get<float>();

		if( cameraNode.contains( "followDistance" ) && cameraNode[ "followDistance" ].is_number() )
			camera.m_followDistance = cameraNode[ "followDistance" ].get<float>();

		if( cameraNode.contains( "followHeight" ) && cameraNode[ "followHeight" ].is_number() )
			camera.m_followHeight = cameraNode[ "followHeight" ].get<float>();

		if( cameraNode.contains( "rotationSpeedH" ) && cameraNode[ "rotationSpeedH" ].is_number() )
			camera.m_rotationSpeedH = cameraNode[ "rotationSpeedH" ].get<float>();

		if( cameraNode.contains( "rotationSpeedV" ) && cameraNode[ "rotationSpeedV" ].is_number() )
			camera.m_rotationSpeedV = cameraNode[ "rotationSpeedV" ].get<float>();

		if( cameraNode.contains( "turnAcceleration" ) && cameraNode[ "turnAcceleration" ].is_number() )
			camera.m_turnAcceleration = cameraNode[ "turnAcceleration" ].get<float>();

		if( cameraNode.contains( "fov" ) && cameraNode[ "fov" ].is_number() )
			camera.m_fov = cameraNode[ "fov" ].get<float>();
	}

	// Load ImGui Style
	if( root.contains( "imgui_style" ) && root[ "imgui_style" ].is_object() )
	{
		auto &styleJson = root[ "imgui_style" ];

		if( styleJson.contains( "Alpha" ) ) m_style.Alpha = styleJson[ "Alpha" ].get<float>();

		if( styleJson.contains( "WindowPadding" ) )
		{
			m_style.WindowPadding = ImVec2(
				styleJson[ "WindowPadding" ][ 0 ].get<float>(),
				styleJson[ "WindowPadding" ][ 1 ].get<float>() );
		}

		if( styleJson.contains( "FramePadding" ) )
		{
			m_style.FramePadding = ImVec2(
				styleJson[ "FramePadding" ][ 0 ].get<float>(),
				styleJson[ "FramePadding" ][ 1 ].get<float>() );
		}

		if( styleJson.contains( "ItemSpacing" ) )
		{
			m_style.ItemSpacing = ImVec2(
				styleJson[ "ItemSpacing" ][ 0 ].get<float>(),
				styleJson[ "ItemSpacing" ][ 1 ].get<float>() );
		}

		if( styleJson.contains( "WindowRounding" ) ) m_style.WindowRounding = styleJson[ "WindowRounding" ].get<float>();
		if( styleJson.contains( "FrameRounding" ) )  m_style.FrameRounding = styleJson[ "FrameRounding" ].get<float>();
		if( styleJson.contains( "GrabRounding" ) )   m_style.GrabRounding = styleJson[ "GrabRounding" ].get<float>();

		if( styleJson.contains( "Colors" ) )
		{
			for( int i = 0; i < ImGuiCol_COUNT; ++i )
			{
				const auto &col = styleJson[ "Colors" ][ i ];
				if( col.is_array() && col.size() == 4 )
				{
					m_style.Colors[ i ] = ImVec4(
						col[ 0 ].get<float>(),
						col[ 1 ].get<float>(),
						col[ 2 ].get<float>(),
						col[ 3 ].get<float>() );
				}
			}
		}
	}
}
