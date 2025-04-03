
#include <algorithm>
#include "LootDisplay.h"
#include "../Globals.h"
#include "../PSUExConfig.h"
#include "../../PSUStructs/PSUWorld.hpp"
#include "../../PSUStructs/PSUPlayerData.h"
#include "../../PSUMemory/Memory.hpp"
#include "..\Detours4\detours.h"

typedef char( __cdecl *functionLootItemSpawn )( void *a1 );
functionLootItemSpawn pOriginalLootItemSpawn = reinterpret_cast< functionLootItemSpawn >( 0x004CF7C0 );

typedef DWORD *( __thiscall *functionLootItemDelete )( void *a1 );
functionLootItemDelete pOriginalLootItemDelete = reinterpret_cast< functionLootItemDelete >( 0x004CF880 );

std::mutex itemListMtx;

bool __cdecl Hook_LootItemSpawn( void *data )
{
	bool result = false;

	// Memory Dump Data
	printf( "Loot Item Spawn\n" );
	printf( "Data: %p\n", data );

	for( int i = 0; i < 0x64; i++ )
	{
		printf( "%02X ", ( ( char * )data )[ i ] );
		if( i % 16 == 15 ) printf( "\n" );
	}

	// Lock the mutex to prevent reading the linked list
	// while the game is manipulating it.
	itemListMtx.lock();
	{
		result = pOriginalLootItemSpawn( data );
	}
	itemListMtx.unlock();

	printf( "Loot Item Spawn Result: %d\n", result );

	return result;
}

DWORD *__fastcall Hook_LootItemDelete( void *a1 )
{
	return pOriginalLootItemDelete( a1 );
}

LootDisplay::LootDisplay() : IFWinCtrl( "LootDisplay" )
{
	auto &config = Config::Get().loot_reader;
	this->m_bEnabled = config.m_windowInfo.m_enabled;
	this->m_errorItemPass = 0;
	this->m_errorItemList.clear();

	// Sooner or later these need to work, but for now they're kinda fucked.
	//DetourTransactionBegin();
	//{
	//	DetourUpdateThread( GetCurrentThread() );
	//	DetourAttach( &( PVOID & )pOriginalLootItemSpawn, reinterpret_cast< PVOID >( Hook_LootItemSpawn ) );
	//	DetourAttach( &( PVOID & )pOriginalLootItemDelete, reinterpret_cast< PVOID >( Hook_LootItemDelete ) );
	//}
	//DetourTransactionCommit();
}

LootDisplay::~LootDisplay()
{
}

void LootDisplay::SetInitialWindowPosition()
{
	auto &config = Config::Get().loot_reader;

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

void LootDisplay::Render()
{
	auto &config = Config::Get().loot_reader;
	config.m_windowInfo.m_enabled = this->m_bEnabled;

	if( !this->m_bEnabled ) return;

	SetInitialWindowPosition();

	auto &player = PSUPlayer::Get();
	
	if( PSUWorld::GetType( player.quest_id ) != PSUWorld::WorldType::Mission )
	{
		// Temporary. Should really be checked on the item delete hook, if it ever works.
		if( m_errorItemList.size() > 0 )
		{
			m_errorItemList.clear();
		}

		if( config.m_displayMission )
		{
			return;
		}
	}

	auto ptr_table = PSUMemory::ReadMemory< uintptr_t >( ptr_to_ptr_table );

	ImGui::Begin( "Loot Display", &m_bEnabled );
	{
		// Capture current position
		config.m_windowInfo.m_position = ImGui::GetWindowPos();
		config.m_windowInfo.m_size = ImGui::GetWindowSize();

		static ImGuiTableFlags flags = ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable |
			ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV |
			ImGuiTableFlags_SizingStretchSame;

		// Check column visibility count
		int visibleColumns = 0;
		for( int i = 0; i < enum_column_type::max_number_of_columns; i++ )
		{
			if( config.m_columnVisibility[ i ] ) visibleColumns++;
		}

		if( !ptr_table || visibleColumns == 0 )
		{
			ImGui::End();
			return;
		}

		// Start of our table display for the goodies
		if( ImGui::BeginTable( "Goodies", enum_column_type::max_number_of_columns, flags ) )
		{
			constexpr float weight_small = 1.2f;
			constexpr float weight_name = 4.0f;

			// Setup columns based on visibility settings
			//if( config.m_columnVisibility[ enum_column_type::rarity ] )
				ImGui::TableSetupColumn( "Rarity", ImGuiTableColumnFlags_WidthStretch, weight_small );

			//if( config.m_columnVisibility[ enum_column_type::name ] )
				ImGui::TableSetupColumn( "Name", ImGuiTableColumnFlags_WidthStretch, weight_name );

			//if( config.m_columnVisibility[ enum_column_type::element ] )
				ImGui::TableSetupColumn( "Ele/%", ImGuiTableColumnFlags_WidthStretch, weight_small );

			//if( config.m_columnVisibility[ enum_column_type::distance ] )
				ImGui::TableSetupColumn( "Dist.", ImGuiTableColumnFlags_WidthStretch, weight_small );

			ImGui::TableHeadersRow();

			if( ptr_table != 0 )
			{
				uintptr_t ptr_to_begin = PSUMemory::ReadAbsoluteMemory< uintptr_t >( ptr_table + 0x1C );

				s_loot_entity *entity = PSUMemory::ReadAbsolutePtr< s_loot_entity >( ptr_to_begin );

				while( entity != NULL && entity->state != enum_state::invalid )
				{
					if( CheckDisplayFilter( entity ) )
					{
						switch( entity->item.item_id.var.item_type )
						{
							case enum_item_type::weapon:
							{
								Display_Item_Weapon( entity );
							} break;

							case enum_item_type::line_shield:
							{
								Display_Item_Lineshield( entity );
							} break;

							default:
							{
								Display_Item_Misc( entity );
							} break;
						}

						if( config.hover.m_displayItem )
						{
							DrawHoverItem( entity );
						}
					}

					if( entity->state == enum_state::error )
					{
						// Attempt to re-spawn error items.
						TryFixErrorItem( entity );
						entity->state = enum_state::try_spawn;
					}

					entity = entity->next;
				}
			}
			ImGui::EndTable();
		}

	}
	ImGui::End();
}

void LootDisplay::Display_Item_Weapon( s_loot_entity *entity )
{
	auto &data = entity->item.var_data.as_weapon;
	auto &config = Config::Get().loot_reader;
	auto &player = PSUPlayer::Get();
	auto category = entity->item.item_id.var.item_category - 1;

	if( category < 0 || category >= 27 ) return;

	if( !config.m_weaponTypeFilter[ category ] ) return;
	if( !config.m_elementFilter[ data.element ] ) return;

	ImGui::TableNextRow();

	// Rarity Column
	if( config.m_columnVisibility[ enum_column_type::rarity ] )
	{
		ImGui::TableSetColumnIndex( 0 );

		auto icon = ImageManager::Get()[ ICON_STAR_00 + data.rank ];
		ImGui::Image( icon->m_texId, icon->m_size );
		ImGui::SameLine();
		ImGui::Text( "%d", data.rarity + 1 );
	}

	// Name Column
	if( config.m_columnVisibility[ enum_column_type::name ] )
	{
		ImGui::TableSetColumnIndex( 1 );
		auto textColor = config.m_colorItemByElement ? m_color_element[ data.element ] : IM_COL32( 255, 255, 255, 255 );
		ImGui::PushStyleColor( ImGuiCol_Text, textColor );
		ImGui::Text( "%S", entity->item.name );
		ImGui::PopStyleColor();
	}

	// Element Column
	if( config.m_columnVisibility[ enum_column_type::element ] )
	{
		ImGui::TableSetColumnIndex( 2 );
		auto textColor = config.m_colorItemByElement ? m_color_element[ data.element ] : IM_COL32( 255, 255, 255, 255 );
		ImGui::PushStyleColor( ImGuiCol_Text, textColor );

		if( data.element_percent >= 0 )
		{
			auto icon = ImageManager::Get()[ ICON_ELEMENT_NEUTRAL + data.element ];
			ImGui::Image( icon->m_texId, icon->m_size );
			ImGui::SameLine();
			ImGui::Text( "%d%%", data.element_percent );
		}
		else
		{
			ImGui::Text( "---" );
		}
		ImGui::PopStyleColor();
	}

	// Distance Column
	if( config.m_columnVisibility[ enum_column_type::distance ] )
	{
		ImGui::TableSetColumnIndex( 3 );
		ImGui::Text( "%.1fm", player.position.DistanceFrom( entity->position ) / 10.0f );
	}
}

void LootDisplay::Display_Item_Lineshield( s_loot_entity *entity )
{
	auto &config = Config::Get().loot_reader;
	auto &player = PSUPlayer::Get();
	auto &data = entity->item.var_data.as_lineshield;

	if( !config.m_itemTypeFilter[ line_shield ] ) return;
	if( !config.m_elementFilter[ data.element ] ) return;

	ImGui::TableNextRow();

	// Rarity Column
	if( config.m_columnVisibility[ enum_column_type::rarity ] )
	{
		ImGui::TableSetColumnIndex( 0 );
		auto icon = ImageManager::Get()[ ICON_STAR_00 + data.rank ];
		ImGui::Image( icon->m_texId, icon->m_size );
		ImGui::SameLine();
		ImGui::Text( "%d", data.rarity + 1 );
	}

	// Name Column
	if( config.m_columnVisibility[ enum_column_type::name ] )
	{
		ImGui::TableSetColumnIndex( 1 );

		auto textColor = config.m_colorItemByElement ? m_color_element[ data.element ] : IM_COL32( 255, 255, 255, 255 );
		ImGui::PushStyleColor( ImGuiCol_Text, textColor );

		ImGui::Text( "%S", entity->item.name );

		ImGui::PopStyleColor();
	}

	// Element Column
	if( config.m_columnVisibility[ enum_column_type::element ] )
	{
		ImGui::TableSetColumnIndex( 2 );

		auto textColor = config.m_colorItemByElement ? m_color_element[ data.element ] : IM_COL32( 255, 255, 255, 255 );
		ImGui::PushStyleColor( ImGuiCol_Text, textColor );

		if( data.element_percent >= 0 )
		{
			auto icon = ImageManager::Get()[ ICON_ELEMENT_NEUTRAL + data.element ];
			ImGui::Image( icon->m_texId, icon->m_size );
			ImGui::SameLine();
			ImGui::Text( "%d%%", data.element_percent );
		}
		else
		{
			ImGui::Text( "---" );
		}

		ImGui::PopStyleColor();
	}

	// Distance Column
	if( config.m_columnVisibility[ enum_column_type::distance ] )
	{
		ImGui::TableSetColumnIndex( 3 );
		ImGui::Text( "%.1fm", player.position.DistanceFrom( entity->position ) / 10.0f );
	}
}

void LootDisplay::Display_Item_Misc( s_loot_entity *entity )
{
	auto &player = PSUPlayer::Get();
	auto &data = entity->item.var_data.generic;

	auto rank = 1 + ( data.rarity / 3 );

	ImGui::TableNextRow();
	{
		ImGui::TableSetColumnIndex( 0 );
		{
			auto icon = ImageManager::Get()[ ICON_STAR_00 + rank ];
			ImGui::Image( icon->m_texId, icon->m_size );
			ImGui::SameLine();
			ImGui::Text( "%d", ( data.rarity + 1 ) );
		}
		ImGui::TableSetColumnIndex( 1 );
		{
			ImGui::Text( "%S", entity->item.name );
		}
		ImGui::TableSetColumnIndex( 2 );
		{
			ImGui::Text( "");
		}

		ImGui::TableSetColumnIndex( 3 );
		{
			ImGui::Text( "%.1fm", player.position.DistanceFrom( entity->position ) / 10.0f );
		}
	}
}

sptr_image LootDisplay::GetItemIcon( const s_loot_entity *entity )
{
	auto item_id = entity->item.item_id;
	auto type = item_id.var.item_type;
	auto category = item_id.var.item_category;
	auto index = item_id.var.item_index;

	switch( item_id.var.item_type )
	{
		case enum_item_type::weapon:
		{
			return ImageManager::Get()[ ICON_WEAPON_SWORD + category - 1 ];
		} break;

		case enum_item_type::line_shield:
		{
			return ImageManager::Get()[ ICON_ITEM_LINE_SHIELD ];
		} break;

		case enum_item_type::consumable:
		{
			return GetConsumableIcon( entity );
		} break;

		case enum_item_type::material:
		{
			return ImageManager::Get()[ ICON_ITEM_MATERIAL_01 + category - 1 ];
		} break;

		case enum_item_type::shield_unit:
		{
			return ImageManager::Get()[ ICON_ITEM_UNIT_01 + category - 1 ];
		} break;

		case enum_item_type::clothing:
		{
			return ImageManager::Get()[ ICON_ITEM_CLOTHING_01 + category - 1 ];
		} break;

		case enum_item_type::part:
		{
			return ImageManager::Get()[ ICON_ITEM_CAST_01 + category - 1 ];
		} break;

		case enum_item_type::decor:
		{
			return ImageManager::Get()[ ICON_ITEM_DECORATION ];
		} break;

		case enum_item_type::trap:
		{
			return ImageManager::Get()[ ICON_ITEM_TRAP_01 + category - 1 ];
		} break;

		case enum_item_type::board:
		{
			return ImageManager::Get()[ ICON_ITEM_BOARD ];
		} break;

		case enum_item_type::pm_device:
		{
			return ImageManager::Get()[ ICON_ITEM_PM_DEVICE ];
		} break;

		case enum_item_type::grinder:
		{
			return ImageManager::Get()[ ICON_ITEM_CONSUMABLE_06 ];
		} break;

		case enum_item_type::boost:
		{
			return ImageManager::Get()[ ICON_ITEM_BOOST ];
		} break;

		case enum_item_type::tool:
		{
			if( category == 1 )
			{
				return ImageManager::Get()[ ICON_ITEM_TOOL_01 ];
			}
			else if( category == 2 )
			{
				return ImageManager::Get()[ ICON_ITEM_TOOL_02 + category - 1 ];
			}
		} break;

		case enum_item_type::meseta:
		{
			return ImageManager::Get()[ ICON_ITEM_MESETA ];
		} break;

		default:
		{
			return nullptr;
		}
	}

	return nullptr;
}

sptr_image LootDisplay::GetConsumableIcon( const s_loot_entity *entity )
{
	auto item_id = entity->item.item_id;
	auto category = item_id.var.item_category;
	auto index = item_id.var.item_index;

	switch( category )
	{
		case 1:
		{
			switch( index )
			{
				// Healing
				case 0:	case 1:	case 2: case 3:
					return ImageManager::Get()[ ICON_ITEM_CONSUMABLE_01 ];

				// Status
				case 5:	case 6:
					return ImageManager::Get()[ ICON_ITEM_CONSUMABLE_03 ];

				// Resurrection
				case 7:	case 8: case 9:
					return ImageManager::Get()[ ICON_ITEM_CONSUMABLE_04 ];

				// Buffs
				case 10: case 11: case 12: case 13: case 14:
					return ImageManager::Get()[ ICON_ITEM_CONSUMABLE_02 ];
			}
		} break;

		case 2:
		{
			// Photon Charges
			return ImageManager::Get()[ ICON_ITEM_CONSUMABLE_05 ];
		} break;

		case 3:
		{
			// Synth Ingredients
			return ImageManager::Get()[ ICON_ITEM_CONSUMABLE_07 ];
		} break;

		case 4:
		{
			// Gold Bar/Valuables
			return ImageManager::Get()[ ICON_ITEM_CONSUMABLE_06 ];
		} break;
	}

	return nullptr;
}

void LootDisplay::DrawHoverItem( s_loot_entity *entity )
{
	auto &config = Config::Get().loot_reader;
	auto &player = PSUPlayer::Get();

	Vector3F worldPos = entity->position;
	worldPos.y += config.hover.m_floatHeight;

	ImVec2 centerPos;
	if( !ToScreenSpace( worldPos, centerPos ) ) return;

	auto screenPos = centerPos;

	auto &data = entity->item.var_data.generic;
	auto item_type = entity->item.item_id.var.item_type;
	auto item_category = entity->item.item_id.var.item_category;
	auto rank = 1 + ( data.rarity / 3 );

	auto quantity = 0;
	auto element_id = 0;
	auto element_percent = 0;

	auto item_name = WideToUTF8( entity->item.name );
	std::string additional_name = "";

	std::vector<s_icon_text_pair> additionalInfo;

	// Only Weapons and Shields have element data.
	switch( entity->item.item_id.var.item_type )
	{
		case weapon:
		case line_shield:
		{
			auto &data = entity->item.var_data.generic;
			element_id = data.element;
			element_percent = data.element_percent;
			quantity = 0;
			rank = data.rank;
		} break;

		case meseta:
		{
			item_name = "Meseta";
			quantity = data.quantity;
		} break;

		default:
		{
			quantity = data.quantity;
		} break;
	}

	// Stackable Items need to show their quantity.
	if( quantity > 0 )
	{
		if( config.hover.m_displayItemIcon )
		{
			additional_name += " x" + std::to_string( quantity );
		}
		else
		{
			item_name += " x" + std::to_string( quantity );
		}
	}

	auto textSize = ImGui::CalcTextSize( item_name.c_str() );
	auto itemIcon = GetItemIcon( entity );

	// Calculate fade alpha
	float distance = player.position.DistanceFrom( entity->position );
	float alpha = 1.0f;

	if( config.hover.m_displayFade )
	{
		float maxDistance = config.hover.m_fadeDistance;
		float fadeStart = maxDistance - 50.0f;
		alpha = 1.0f - ( distance - fadeStart ) / ( maxDistance - fadeStart );
		alpha = std::clamp( alpha, 0.0f, 1.0f );
		if( alpha <= 0.0f ) return;
	}

	// Setup colors with fade alpha
	auto iconColor = IM_COL32( 255, 255, 255, static_cast< int >( alpha * 255 ) );
	auto whiteColor = IM_COL32( 255, 255, 255, static_cast< int >( alpha * 255 ) );
	auto shadowColor = IM_COL32( 0, 0, 0, static_cast< int >( alpha * 200 ) );

	ImColor elementColor( m_color_element[ element_id ] );
	elementColor.Value.w = alpha;

	ImColor textColor = whiteColor;

	if( config.hover.m_colorItemByElement )
	{
		textColor = elementColor;
	}
	else
	{
		textColor = whiteColor;
	}

	// Show Rarity
	if( config.hover.m_displayItemRarity && item_type != enum_item_type::meseta )
	{
		sptr_image starIcon = ImageManager::Get()[ ICON_STAR_00 + rank ];
		additionalInfo.push_back( { starIcon, std::to_string( data.rarity + 1 ), whiteColor } );
	}

	// Show Item Icon and additional name
	if( config.hover.m_displayItemIcon )
	{
		additionalInfo.push_back( { itemIcon, additional_name, whiteColor } );
	}

	// Show Element Icon and percentage
	if( config.hover.m_displayElementIcon && data.element_percent > 0 )
	{
		sptr_image elementIcon = ImageManager::Get()[ ICON_ELEMENT_NEUTRAL + data.element ];
		additionalInfo.push_back( { elementIcon, std::to_string( data.element_percent ), textColor } );
	}

	// Center item name
	screenPos.x -= textSize.x * 0.5f;
	screenPos.y -= textSize.y * 0.5f * config.hover.m_floatHeight;

	//ImDrawList *drawList = ImGui::GetForegroundDrawList();

	//DrawTextShadowed( drawList, screenPos, textColor, shadowColor, item_name, 1.15 );

	s_hover_draw_param drawParams{
	.centerPos = centerPos,
	.screenPos = screenPos,
	.textSize = textSize,
	.itemName = item_name,
	.iconTextPairs = additionalInfo,
	.iconColor = iconColor,
	.shadowColor = shadowColor,
	.textColor = textColor
	};

	DrawIconsBelow( drawParams );
}

void LootDisplay::DrawTextShadowed(
	ImDrawList *drawList, const ImVec2 &pos, const ImColor &textColor, const ImColor &shadowColor, const std::string &text, float fontScale, const ImVec2 &shadowOffset )
{
	if( !drawList || text.empty() )
		return;

	ImGui::SetWindowFontScale( fontScale );

	drawList->AddText( ImVec2( pos.x + shadowOffset.x, pos.y + shadowOffset.y ), shadowColor, text.c_str() );
	drawList->AddText( pos, textColor, text.c_str() );

	ImGui::SetWindowFontScale( 1.0f );
}

void LootDisplay::DrawIconsBelow( s_hover_draw_param &params )
{
	ImDrawList *drawList = ImGui::GetForegroundDrawList();

	DrawTextShadowed( drawList, params.screenPos, params.textColor, params.shadowColor, params.itemName, params.fontScale );

	if( params.iconTextPairs.empty() ) return;

	const float padding = 4.0f;
	const float textOffset = 2.0f;

	float totalWidth = 0.0f;
	for( const auto &p : params.iconTextPairs )
	{
		float iconW = ( p.icon ) ? p.icon->m_size.x : 0.0f;
		float textW = ImGui::CalcTextSize( p.text.c_str() ).x;
		totalWidth += iconW + ( p.text.empty() ? 0.0f : ( textOffset + textW ) ) + padding;
	}
	totalWidth -= padding;

	ImVec2 pos;
	pos.y = params.screenPos.y + params.textSize.y + 4.0f;
	pos.x = params.centerPos.x - totalWidth * 0.5f;

	for( const auto &p : params.iconTextPairs )
	{
		if( p.icon )
		{
			drawList->AddImage( ( ImTextureID )p.icon->m_texId,
								pos,
								ImVec2( pos.x + p.icon->m_size.x, pos.y + p.icon->m_size.y ),
								ImVec2( 0, 0 ), ImVec2( 1, 1 ),
								params.iconColor
			);
			pos.x += p.icon->m_size.x;
		}

		if( !p.text.empty() )
		{
			pos.x += textOffset;
			DrawTextShadowed( drawList, pos, params.textColor, params.shadowColor, p.text, params.fontScale );
			pos.x += ImGui::CalcTextSize( p.text.c_str() ).x;
		}

		pos.x += padding;
	}
}

bool LootDisplay::CheckDisplayFilter( s_loot_entity *entity )
{
	auto config = Config::Get().loot_reader;

	if( entity->item.item_id.var.item_type == meseta && config.m_itemTypeFilter[ meseta ] )
	{
		return true;
	}

	if( !config.m_itemTypeFilter[ entity->item.item_id.var.item_type ] )
	{
		return false;
	}

	auto rarity = entity->item.var_data.generic.rarity;

	if( rarity < config.m_minimumRarity || rarity > config.m_maximumRarity )
	{
		return false;
	}

	return true;
}

void LootDisplay::TryFixErrorItem( s_loot_entity *entity )
{
	auto it = m_errorItemList.find( entity->item.unique_id );
	Vector3F originalPosition;

	if( it == m_errorItemList.end() )
	{
		// Raise the item by 2 units, just in case it somehow sank under the map.
		entity->position.y += 2.0f;
		originalPosition = entity->position;
		m_errorItemList.insert( std::make_pair( entity->item.unique_id, originalPosition ) );
	}
	else
	{
		originalPosition = it->second;
	}

	// We'll rotate the item spawn around the original position.
	constexpr float _2PI = 3.14159265358979323846f * 2.0f;
	constexpr int num_directions = 16;
	constexpr float angle_steps = _2PI / num_directions;
	constexpr float offset_distance = 5.0f;

	int directionIndex = m_errorItemPass % num_directions;
	float angle = directionIndex * angle_steps;

	Vector3F offset = {
		std::cos( angle ) * offset_distance,
		0.0f,
		std::sin( angle ) * offset_distance
	};

	Vector3F attemptPosition = originalPosition + offset;
	entity->position = attemptPosition;

	printf( "Item [%d] Attempt Respawn at: (%.2f, %.2f, %.2f)\n",
			entity->item.unique_id,
			entity->position.x,
			entity->position.y,
			entity->position.z );

	m_errorItemPass = ( m_errorItemPass + 1 ) % num_directions;
}

bool LootDisplay::ToScreenSpace( const Vector3F &worldPos, ImVec2 &screenPos )
{
	// Calculate the vector from the camera position to the item position
	Vector3F toItem = worldPos - Global::cameraPosition;
	toItem.Normalize();

	// Check if the item is in front of the camera using the dot product
	float dotProduct = toItem.Dot( Global::cameraForward );

	// If the item is behind the camera, skip rendering
	if( dotProduct <= 0.0f )
		return false;

	// Combine view and projection matrices
	Matrix4x4 viewProjMatrix = Global::viewMatrix * Global::projectionMatrix;

	// Transform the world position to clip space
	Vector3F clipSpacePos = viewProjMatrix.Multiply( worldPos );

	// If the position is behind the camera, don't render
	if( clipSpacePos.z <= 0.0f )
		return false;

	// Convert to screen space (Perspective Divide)
	screenPos.x = ( clipSpacePos.x / clipSpacePos.z + 1.0f ) * 0.5f * Global::screenWidth;
	screenPos.y = ( 1.0f - ( clipSpacePos.y / clipSpacePos.z ) ) * 0.5f * Global::screenHeight;

	return true;
}

std::string LootDisplay::WideToUTF8( const wchar_t *wideString )
{
	if( !wideString ) return std::string();

	int bufferSize = WideCharToMultiByte( CP_UTF8, 0, wideString, -1, nullptr, 0, nullptr, nullptr );
	if( bufferSize == 0 ) return std::string();

	std::string utf8String( bufferSize, 0 );
	WideCharToMultiByte( CP_UTF8, 0, wideString, -1, &utf8String[ 0 ], bufferSize, nullptr, nullptr );

	// Remove null terminator if present
	if( !utf8String.empty() && utf8String.back() == '\0' )
		utf8String.pop_back();

	return utf8String;
}