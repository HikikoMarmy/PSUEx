#include <ranges>
#include <algorithm>

#include "ConfigDisplay.h"
#include "../Globals.h"
#include "../PSUExConfig.h"
#include "../PSUStructs/Item.h"

ConfigDisplay::ConfigDisplay() : IFWinCtrl( "ConfigDisplay" )
{
	this->m_bEnabled = false;
	this->m_bSaveConfig = false;
}

ConfigDisplay::~ConfigDisplay()
{
}

void ConfigDisplay::SetInitialWindowPosition()
{
	auto &config = Config::Get().config;
	auto x = config.m_windowInfo.m_position.x;
	auto y = config.m_windowInfo.m_position.y;

	if( x < 0 || x > Global::screenWidth )
	{
		x = 0;
	}
	if( y < 0 || y > Global::screenHeight )
	{
		y = 0;
	}

	ImGui::SetNextWindowPos( ImVec2( x, y ), ImGuiCond_FirstUseEver );
	ImGui::SetNextWindowSize( config.m_windowInfo.m_size, ImGuiCond_FirstUseEver );
}

void ConfigDisplay::Render()
{
	if( !this->m_bEnabled )
	{
		if( this->m_bSaveConfig )
		{
			Config::Get().SaveToFile();
			this->m_bSaveConfig = false;
		}

		return;
	}

	SetInitialWindowPosition();

	this->m_bSaveConfig = true;

	auto &config = Config::Get().config;

	ImGui::Begin( "ConfigDisplay", &this->m_bEnabled );
	{
		config.m_windowInfo.m_position = ImGui::GetWindowPos();
		config.m_windowInfo.m_size = ImGui::GetWindowSize();

		ImGuiTabBarFlags config_tab_flags = ImGuiTabBarFlags_None;
		if( ImGui::BeginTabBar( "Config Tabs", config_tab_flags ) )
		{
			if( ImGui::BeginTabItem( "Loot Reader" ) )
			{
				RenderLootReaderConfig();
				ImGui::EndTabItem();
			}

			if( ImGui::BeginTabItem( "Over-Item Display" ) )
			{
				RenderHoverItemConfig();
				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
		}
	}
	ImGui::End();
}

void ConfigDisplay::RenderLootReaderConfig()
{
	if( ImGui::BeginTabBar( "Config Tabs", ImGuiTabBarFlags_None ) )
	{
		if( ImGui::BeginTabItem( "Display" ) )
		{
			RenderDisplayTab();
			ImGui::EndTabItem();
		}

		if( ImGui::BeginTabItem( "Item Types" ) )
		{
			RenderItemFilterTab();
			ImGui::EndTabItem();
		}

		if( ImGui::BeginTabItem( "Rarity" ) )
		{
			RenderRarityFilterTab();
			ImGui::EndTabItem();
		}

		if( ImGui::BeginTabItem( "Element" ) )
		{
			RenderElementFilterTab();
			ImGui::EndTabItem();
		}
	}
	ImGui::EndTabBar();
}

void ConfigDisplay::RenderDisplayTab()
{
	auto &config = Config::Get().loot_reader;
	float spacing = ImGui::GetStyle().ItemInnerSpacing.x;

	ImGui::Checkbox( "Only Show in Mission", &config.m_displayMission );

	//ImGui::Separator();

	//ImGui::Checkbox( "Rarity Column", &config.m_columnVisibility[ 0 ] );
	//ImGui::Checkbox( "Name Column", &config.m_columnVisibility[ 1 ] );
	//ImGui::Checkbox( "Element Column", &config.m_columnVisibility[ 2 ] );
	//ImGui::Checkbox( "Distance Column", &config.m_columnVisibility[ 3 ] );

	ImGui::Separator();

	ImGui::Checkbox( "Color by Element", &config.m_colorItemByElement );
}

void ConfigDisplay::RenderItemFilterTab()
{
	auto &config = Config::Get().loot_reader;

	ImGui::AlignTextToFramePadding();

	if( ImGui::BeginTabBar( "ItemFilterBar", ImGuiTabBarFlags_None ) )
	{
		//if( ImGui::BeginTabItem( "Consumable" ) )
		//{
		//	ImGui::EndTabItem();
		//}

		if( ImGui::BeginTabItem( "Weapons" ) )
		{
			for( int i = 0; i < 27; i++ )
			{
				auto icon = ImageManager::Get()[ ICON_WEAPON_SWORD + i ];
				ImGui::Image( icon->m_texId, icon->m_size );
				ImGui::SameLine();
				ImGui::Checkbox( item_weapon_name[ i ], &config.m_weaponTypeFilter[ i ] );
			}
			ImGui::EndTabItem();
		}

		//if( ImGui::BeginTabItem( "Units" ) )
		//{
		//	ImGui::EndTabItem();
		//}

		if( ImGui::BeginTabItem( "Misc." ) )
		{
			ImGui::Checkbox( "Meseta", &config.m_itemTypeFilter[ meseta ] );
			ImGui::Checkbox( "Material", &config.m_itemTypeFilter[ material ] );
			ImGui::Checkbox( "Unit", &config.m_itemTypeFilter[ shield_unit ] );

			ImGui::Checkbox( "Clothing", &config.m_itemTypeFilter[ clothing ] );
			ImGui::Checkbox( "CAST Part", &config.m_itemTypeFilter[ part ] );

			ImGui::Checkbox( "Room Decor", &config.m_itemTypeFilter[ decor ] );
			ImGui::Checkbox( "Trap", &config.m_itemTypeFilter[ trap ] );

			ImGui::Checkbox( "Board", &config.m_itemTypeFilter[ board ] );

			ImGui::Checkbox( "PM Device", &config.m_itemTypeFilter[ pm_device ] );
			ImGui::Checkbox( "Grinder", &config.m_itemTypeFilter[ grinder ] );

			ImGui::Checkbox( "Boost", &config.m_itemTypeFilter[ boost ] );
			ImGui::Checkbox( "Tool", &config.m_itemTypeFilter[ tool ] );
			ImGui::EndTabItem();
		}
	}
	ImGui::EndTabBar();
}

void ConfigDisplay::RenderRarityFilterTab()
{
	float spacing = ImGui::GetStyle().ItemInnerSpacing.x;
	auto &config = Config::Get().loot_reader;
	auto &imgMng = ImageManager::Get();

	// Maximum Rarity
	ImGui::AlignTextToFramePadding();
	{
		ImGui::Text( "Maximum Rarity:" );
		ImGui::SameLine( 0, spacing );
		ImGui::Text( "%d", config.m_maximumRarity + 1 );
		ImGui::SameLine( 150, spacing );
		ImGui::PushItemFlag( ImGuiItemFlags_ButtonRepeat, true );

		if( ImGui::ArrowButton( "##up", ImGuiDir_Up ) )
		{
			config.m_maximumRarity = std::clamp< int8_t >( config.m_maximumRarity + 1, 0, 14 );
		}

		ImGui::SameLine( 0.0f, spacing );

		if( ImGui::ArrowButton( "##down", ImGuiDir_Down ) )
		{
			config.m_maximumRarity = std::clamp< int8_t >( config.m_maximumRarity - 1, 0, 14 );
			if( config.m_minimumRarity > config.m_maximumRarity )
			{
				config.m_minimumRarity = config.m_maximumRarity;
			}
		}
		ImGui::PopItemFlag();
	}

	// Minimum Rarity
	ImGui::AlignTextToFramePadding();
	{
		ImGui::Text( "Minimum Rarity:" );
		ImGui::SameLine( 0, spacing );
		ImGui::Text( "%d", config.m_minimumRarity + 1 );
		ImGui::SameLine( 150, spacing );
		ImGui::PushItemFlag( ImGuiItemFlags_ButtonRepeat, true );

		if( ImGui::ArrowButton( "##upmin", ImGuiDir_Up ) )
		{
			config.m_minimumRarity = std::clamp< int8_t>( config.m_minimumRarity + 1, 0, config.m_maximumRarity );
		}

		ImGui::SameLine( 0.0f, spacing );

		if( ImGui::ArrowButton( "##downmin", ImGuiDir_Down ) )
		{
			config.m_minimumRarity = std::clamp< int8_t>( config.m_minimumRarity - 1, 0, config.m_maximumRarity );
		}
		ImGui::PopItemFlag();
	}

	for( int i = 0; i < 15; i++ )
	{
		auto icon = imgMng[ ICON_STAR_01 + ( i / 3 ) ];

		if( i < config.m_minimumRarity || i > config.m_maximumRarity )
		{
			icon = imgMng[ ICON_STAR_00 ];
		}

		ImGui::Image( icon->m_texId, icon->m_size );
		ImGui::SameLine();
	}
}

void ConfigDisplay::RenderElementFilterTab()
{
	float spacing = ImGui::GetStyle().ItemInnerSpacing.x;
	auto &config = Config::Get().loot_reader;

	ImGui::Checkbox( "Color by Element", &config.m_colorItemByElement );

	//mGui::ColorEdit3( "Neutral", &config.m_elementColor[ 0 ].Value.x );
	//mGui::ColorEdit3( "Fire", &config.m_elementColor[ 1 ].Value.x );
	//mGui::ColorEdit3( "Ice", &config.m_elementColor[ 2 ].Value.x );
	//mGui::ColorEdit3( "Lightning", &config.m_elementColor[ 3 ].Value.x );
	//mGui::ColorEdit3( "Ground", &config.m_elementColor[ 4 ].Value.x );
	//mGui::ColorEdit3( "Light", &config.m_elementColor[ 5 ].Value.x );
	//mGui::ColorEdit3( "Dark", &config.m_elementColor[ 6 ].Value.x );

	ImGui::Separator();

	ImGui::AlignTextToFramePadding();
	{
		ImGui::Text( "Minimum Element Percent:" );
		ImGui::SameLine( 0, spacing );
		ImGui::Text( "%d", config.m_minimumElementPercent );
		ImGui::SameLine( 220, spacing );
		ImGui::PushItemFlag( ImGuiItemFlags_ButtonRepeat, true );

		if( ImGui::ArrowButton( "##upmin", ImGuiDir_Up ) )
		{
			config.m_minimumElementPercent = std::clamp< int8_t>( config.m_minimumElementPercent + 1, 0, 50 );
		}

		ImGui::SameLine( 0.0f, spacing );
		if( ImGui::ArrowButton( "##downmin", ImGuiDir_Down ) )
		{
			config.m_minimumElementPercent = std::clamp< int8_t>( config.m_minimumElementPercent - 1, 0, 50 );
		}
		ImGui::PopItemFlag();
	}

	ImGui::Separator();

	ImGui::AlignTextToFramePadding();

	for( int i = 0; i < enum_element_type::max_number_of_elements; i++ )
	{
		ImGui::AlignTextToFramePadding();

		auto icon = ImageManager::Get()[ ICON_ELEMENT_NEUTRAL + i ];
		ImGui::Image( icon->m_texId, icon->m_size );
		ImGui::SameLine( 0, spacing );
		ImGui::Checkbox( m_elementName[ i ], &config.m_elementFilter[ i ] );
	}
}

void ConfigDisplay::RenderHoverItemConfig()
{
	auto &config = Config::Get().loot_reader;
	ImGui::AlignTextToFramePadding();

	ImGui::Text( "On-Screen Item Names Settings" );

	// Main Enable Option
	ImGui::Checkbox( "Enable", &config.hover.m_displayItem );

	ImGui::Separator();

	// Appearance Settings
	if( ImGui::CollapsingHeader( "Appearance Settings" ) )
	{
		ImGui::Indent();

		// Floating Height Adjustment
		ImGui::SliderFloat( "Float Height", &config.hover.m_floatHeight, -5.0f, 15.0f, "%.1f" );

		// Display Element Icon & Item Icon
		ImGui::Separator();
		ImGui::Checkbox( "Display Item Icon", &config.hover.m_displayItemIcon );
		ImGui::Checkbox( "Display Item Rarity", &config.hover.m_displayItemRarity );
		ImGui::Checkbox( "Display Element Icon", &config.hover.m_displayElementIcon );
		
		// Color by Element Checkbox
		ImGui::Separator();
		ImGui::Checkbox( "Color By Element", &config.hover.m_colorItemByElement );

		ImGui::Unindent();
	}

	ImGui::Separator();

	// Fading Settings
	if( ImGui::CollapsingHeader( "Fading Settings" ) )
	{
		ImGui::Indent();

		// Fade Over Distance
		ImGui::Checkbox( "Fade Over Distance", &config.hover.m_displayFade );

		// Minimum Distance Slider
		ImGui::SliderFloat( "Minimum Fade Distance", &config.hover.m_fadeDistance, 100.0f, 1000.0f, "%.1f" );

		ImGui::Unindent();
	}

	ImGui::Separator();
}
