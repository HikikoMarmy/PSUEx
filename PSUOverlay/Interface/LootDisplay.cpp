#include <algorithm>
#include "LootDisplay.h"

#include "../Misc/Globals.h"
#include "../Misc/StringUtil.h"
#include "../Misc/MathUtil.h"
#include "../PSUStructs/PSUWorld.hpp"
#include "../PSUMemory/Memory.hpp"

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
	this->m_bEnabled = Config::Get().loot_reader.m_windowInfo.m_enabled;
	this->m_errorItemPass = 0;
	this->m_errorItemList.clear();

	m_itemCount = 0;
	m_lastCacheUpdate = std::chrono::high_resolution_clock::now();

	std::memset( &m_cachedItemData[ 0 ], 0, sizeof(m_cachedItemData));

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
	auto x = m_lootConfig->m_windowInfo.m_position.x;
	auto y = m_lootConfig->m_windowInfo.m_position.y;

	if( x < 0 || x > Global::screenWidth )
	{
		x = 0;
	}

	if( y < 0 || y > Global::screenHeight )
	{
		y = 0;
	}

	ImGui::SetNextWindowPos( ImVec2( x, y ), ImGuiCond_FirstUseEver );
	ImGui::SetNextWindowSize( m_lootConfig->m_windowInfo.m_size, ImGuiCond_FirstUseEver );
}

void LootDisplay::Render()
{
	m_lootConfig = &Config::Get().loot_reader;
	m_lootConfig->m_windowInfo.m_enabled = this->m_bEnabled;

	if( !this->m_bEnabled )
		return;

	m_player = &PSUPlayer::Get();

	if( !m_player )
		return;
	
	if( PSUWorld::GetType( m_player->quest_id ) != PSUWorld::WorldType::Mission )
	{
		// Temporary. Should really be checked on the item delete hook, if it ever works.
		if( m_errorItemList.size() > 0 )
			m_errorItemList.clear();

		if( m_itemCount > 0 )
		{
			std::memset( &m_cachedItemData[ 0 ], 0, sizeof( m_cachedItemData ) );
			m_itemCount = 0;
		}

		if( m_lootConfig->m_displayMission )
			return;
	}
	else
	{
		RefreshCachedItemData();
	}

	SetInitialWindowPosition();
	

	ImGui::Begin( "Loot Display", &m_bEnabled );
	{
		// Capture current position
		m_lootConfig->m_windowInfo.m_position = ImGui::GetWindowPos();
		m_lootConfig->m_windowInfo.m_size = ImGui::GetWindowSize();

		static ImGuiTableFlags flags = ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable |
			ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV |
			ImGuiTableFlags_SizingStretchSame;

		// Check column visibility count
		int visibleColumns = 0;
		for( int i = 0; i < enum_column_type::max_number_of_columns; i++ )
		{
			if( m_lootConfig->m_columnVisibility[ i ] ) visibleColumns++;
		}

		if(  visibleColumns == 0 )
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
			//if( m_lootConfig->m_columnVisibility[ enum_column_type::rarity ] )
				ImGui::TableSetupColumn( "Rarity", ImGuiTableColumnFlags_WidthStretch, weight_small );

			//if( m_lootConfig->m_columnVisibility[ enum_column_type::name ] )
				ImGui::TableSetupColumn( "Name", ImGuiTableColumnFlags_WidthStretch, weight_name );

			//if( m_lootConfig->m_columnVisibility[ enum_column_type::element ] )
				ImGui::TableSetupColumn( "Ele/%", ImGuiTableColumnFlags_WidthStretch, weight_small );

			//if( m_lootConfig->m_columnVisibility[ enum_column_type::distance ] )
				ImGui::TableSetupColumn( "Dist.", ImGuiTableColumnFlags_WidthStretch, weight_small );

			ImGui::TableHeadersRow();

			for( int i = 0; i < m_itemCount; i++ )
			{
				auto &cachedItem = m_cachedItemData[ i ];

				if( CheckDisplayFilter( &cachedItem ) )
				{
					switch( cachedItem.item_id.var.item_type )
					{
						case enum_item_type::weapon:
						{
							Display_Item_Weapon( &cachedItem );
						} break;
						case enum_item_type::line_shield:
						{
							Display_Item_Lineshield( &cachedItem );
						} break;
						default:
						{
							Display_Item_Misc( &cachedItem );
						} break;
					}

					if( m_lootConfig->hover.m_displayItem )
					{
						DrawHoverItem( &cachedItem );
					}
				}
				
				/*while( entity != NULL && entity->state != enum_state::invalid )
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

						if( m_lootConfig->hover.m_displayItem )
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
				}*/
			}
			ImGui::EndTable();
		}

	}
	ImGui::End();
}

void LootDisplay::RefreshCachedItemData()
{
	auto now = std::chrono::high_resolution_clock::now();
	if( now - m_lastCacheUpdate < std::chrono::milliseconds( 50 ) )
		return;

	m_lastCacheUpdate = now;

	auto ptr_table = PSUMemory::ReadMemory< uintptr_t >( ptr_to_ptr_table );

	if( !ptr_table )
		return;

	auto ptr_to_begin = PSUMemory::ReadAbsoluteMemory< uintptr_t >( ptr_table + 0x1C );
	auto current = PSUMemory::ReadAbsolutePtr< s_loot_entity >( ptr_to_begin );

	std::memset( &m_cachedItemData[ 0 ], 0, sizeof( m_cachedItemData ) );

	auto index = 0;

	while( current && current->state != enum_state::invalid && index < std::size( m_cachedItemData ) )
	{
		if( current->state == enum_state::error )
		{
			// Attempt to re-spawn error items.
			TryFixErrorItem( current );
			current->state = enum_state::try_spawn;
		}

		auto item_name = StringUtil::WideToUTF8( current->item.name );
		auto &cachedItem = m_cachedItemData[ index ];
		auto &data = current->item.var_data.generic;

		cachedItem.quantity = 0;

		switch( current->item.item_id.var.item_type )
		{
			// Only Weapons and Shields have element data.
			case weapon:
			case line_shield:
			{
				cachedItem.element = data.element;
				cachedItem.percent = data.element_percent;
				cachedItem.rank = data.rank;
			} break;

			case meseta:
			{
				item_name = "Meseta";
				
				// We can't get quantity from generic data, so we must parse it from the name.
				std::wstring_view name = current->item.name;
				if( auto pos = name.find( L" x" ); pos != std::wstring_view::npos )
				{
					std::wstring_view number = name.substr( pos + 2 );

					std::string digits;
					for( wchar_t ch : number )
					{
						if( !iswdigit( ch ) ) break;
						digits += static_cast< char >( ch );
					}

					std::from_chars_result result = std::from_chars( digits.data(), digits.data() + digits.size(), cachedItem.quantity );
				}
			} break;

			default:
			{
				cachedItem.quantity = data.quantity;
			} break;
		}

		std::strncpy( cachedItem.name, item_name.c_str(), sizeof( cachedItem.name ) - 1 );

		cachedItem.icon = GetItemIcon( current );
		cachedItem.item_id = current->item.item_id;
		cachedItem.position = current->position;
		cachedItem.unique_id = current->item.unique_id;
		cachedItem.rarity = data.rarity;

		index++;
		current = current->next;
	}

	m_itemCount = index;
}

void LootDisplay::Display_Item_Weapon( s_cached_item *item )
{
	auto category = item->item_id.var.item_category - 1;

	if( category < 0 || category >= 27 ) return;

	if( !m_lootConfig->m_weaponTypeFilter[ category ] ) return;
	if( !m_lootConfig->m_elementFilter[ item->element ] ) return;

	ImGui::TableNextRow();

	// Rarity Column
	if( m_lootConfig->m_columnVisibility[ enum_column_type::rarity ] )
	{
		ImGui::TableSetColumnIndex( 0 );

		auto icon = ImageManager::Get()[ ICON_STAR_00 + item->rank ];
		ImGui::Image( icon->m_texId, icon->m_size );
		ImGui::SameLine();
		ImGui::Text( "%d", item->rarity + 1 );
	}

	// Name Column
	if( m_lootConfig->m_columnVisibility[ enum_column_type::name ] )
	{
		ImGui::TableSetColumnIndex( 1 );
		auto textColor = m_lootConfig->m_colorItemByElement ? m_color_element[ item->element ] : IM_COL32( 255, 255, 255, 255 );
		ImGui::PushStyleColor( ImGuiCol_Text, textColor );
		ImGui::Text( "%s", item->name );
		ImGui::PopStyleColor();
	}

	// Element Column
	if( m_lootConfig->m_columnVisibility[ enum_column_type::element ] )
	{
		ImGui::TableSetColumnIndex( 2 );
		auto textColor = m_lootConfig->m_colorItemByElement ? m_color_element[ item->element ] : IM_COL32( 255, 255, 255, 255 );
		ImGui::PushStyleColor( ImGuiCol_Text, textColor );

		if( item->percent >= 0 )
		{
			auto icon = ImageManager::Get()[ ICON_ELEMENT_NEUTRAL + item->element ];
			ImGui::Image( icon->m_texId, icon->m_size );
			ImGui::SameLine();
			ImGui::Text( "%d%%", item->percent );
		}
		else
		{
			ImGui::Text( "---" );
		}
		ImGui::PopStyleColor();
	}

	// Distance Column
	if( m_lootConfig->m_columnVisibility[ enum_column_type::distance ] )
	{
		ImGui::TableSetColumnIndex( 3 );
		ImGui::Text( "%.1fm", m_player->position.DistanceFrom( item->position ) / 10.0f );
	}
}

void LootDisplay::Display_Item_Lineshield( s_cached_item *item )
{
	if( !m_lootConfig->m_itemTypeFilter[ line_shield ] ) return;
	if( !m_lootConfig->m_elementFilter[ item->element ] ) return;

	ImGui::TableNextRow();

	// Rarity Column
	if( m_lootConfig->m_columnVisibility[ enum_column_type::rarity ] )
	{
		ImGui::TableSetColumnIndex( 0 );
		auto icon = ImageManager::Get()[ ICON_STAR_00 + item->rank ];
		ImGui::Image( icon->m_texId, icon->m_size );
		ImGui::SameLine();
		ImGui::Text( "%d", item->rarity + 1 );
	}

	// Name Column
	if( m_lootConfig->m_columnVisibility[ enum_column_type::name ] )
	{
		ImGui::TableSetColumnIndex( 1 );

		auto textColor = m_lootConfig->m_colorItemByElement ? m_color_element[ item->element ] : IM_COL32( 255, 255, 255, 255 );
		ImGui::PushStyleColor( ImGuiCol_Text, textColor );

		ImGui::Text( "%s", item->name );

		ImGui::PopStyleColor();
	}

	// Element Column
	if( m_lootConfig->m_columnVisibility[ enum_column_type::element ] )
	{
		ImGui::TableSetColumnIndex( 2 );

		auto textColor = m_lootConfig->m_colorItemByElement ? m_color_element[ item->element ] : IM_COL32( 255, 255, 255, 255 );
		ImGui::PushStyleColor( ImGuiCol_Text, textColor );

		if( item->percent >= 0 )
		{
			auto icon = ImageManager::Get()[ ICON_ELEMENT_NEUTRAL + item->element ];
			ImGui::Image( icon->m_texId, icon->m_size );
			ImGui::SameLine();
			ImGui::Text( "%d%%", item->percent );
		}
		else
		{
			ImGui::Text( "---" );
		}

		ImGui::PopStyleColor();
	}

	// Distance Column
	if( m_lootConfig->m_columnVisibility[ enum_column_type::distance ] )
	{
		ImGui::TableSetColumnIndex( 3 );
		ImGui::Text( "%.1fm", m_player->position.DistanceFrom( item->position ) / 10.0f );
	}
}

void LootDisplay::Display_Item_Misc( s_cached_item *item )
{
	auto rank = 1 + ( item->rarity / 3 );

	ImGui::TableNextRow();
	{
		ImGui::TableSetColumnIndex( 0 );
		{
			auto icon = ImageManager::Get()[ ICON_STAR_00 + rank ];
			ImGui::Image( icon->m_texId, icon->m_size );
			ImGui::SameLine();
			ImGui::Text( "%d", ( item->rarity + 1 ) );
		}
		ImGui::TableSetColumnIndex( 1 );
		{
			ImGui::Text( "%s", item->name );
		}
		ImGui::TableSetColumnIndex( 2 );
		{
			ImGui::Text( "");
		}

		ImGui::TableSetColumnIndex( 3 );
		{
			ImGui::Text( "%.1fm", m_player->position.DistanceFrom( item->position ) / 10.0f );
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

void LootDisplay::DrawHoverItem( s_cached_item *item )
{
	Vector3F worldPos = item->position;
	worldPos.y += m_lootConfig->hover.m_floatHeight;

	Vector2F centerPos;
	if( !MathUtil::ToScreenSpace( worldPos, centerPos, Global::viewProjectionMatrix ) ) return;

	auto screenPos = centerPos;

	auto item_type = item->item_id.var.item_type;
	auto item_category = item->item_id.var.item_category;
	auto rank = 1 + ( item->rarity / 3 );

	std::string item_name = item->name;
	std::string additional_name = "";

	std::vector<s_icon_text_pair> additionalInfo;

	// Stackable Items need to show their quantity.
	if( item->quantity > 0 )
	{
		if( m_lootConfig->hover.m_displayItemIcon )
		{
			additional_name += " x" + std::to_string( item->quantity );
		}
		else
		{
			item_name += " x" + std::to_string( item->quantity );
		}
	}

	auto textSize = ImGui::CalcTextSize( item_name.c_str() );
	auto &itemIcon = item->icon;

	// Calculate fade alpha
	float distance = m_player->position.DistanceFrom( item->position );
	float alpha = 1.0f;

	if( m_lootConfig->hover.m_displayFade )
	{
		float maxDistance = m_lootConfig->hover.m_fadeDistance;

		if( distance > maxDistance )
			return;

		float fadeStart = maxDistance - 50.0f;
		alpha = 1.0f - ( distance - fadeStart ) / ( maxDistance - fadeStart );
		alpha = std::clamp( alpha, 0.0f, 1.0f );
		if( alpha <= 0.0f ) return;
	}

	// Setup colors with fade alpha
	auto iconColor = IM_COL32( 255, 255, 255, static_cast< int >( alpha * 255 ) );
	auto whiteColor = IM_COL32( 255, 255, 255, static_cast< int >( alpha * 255 ) );
	auto shadowColor = IM_COL32( 0, 0, 0, static_cast< int >( alpha * 200 ) );

	ImColor textColor = whiteColor;
	ImColor elementColor( m_color_element[ item->element ] );
	elementColor.Value.w = alpha;

	if( m_lootConfig->hover.m_colorItemByElement )
	{
		textColor = elementColor;
	}
	else
	{
		textColor = whiteColor;
	}

	// Show Rarity
	if( m_lootConfig->hover.m_displayItemRarity && item_type != enum_item_type::meseta )
	{
		sptr_image starIcon = ImageManager::Get()[ ICON_STAR_00 + rank ];
		additionalInfo.push_back( { starIcon, std::to_string( item->rarity + 1 ), whiteColor } );
	}

	// Show Item Icon and additional name
	if( m_lootConfig->hover.m_displayItemIcon )
	{
		additionalInfo.push_back( { itemIcon, additional_name, whiteColor } );
	}

	// Show Element Icon and percentage
	if( m_lootConfig->hover.m_displayElementIcon && item->percent > 0 )
	{
		sptr_image elementIcon = ImageManager::Get()[ ICON_ELEMENT_NEUTRAL + item->element ];
		additionalInfo.push_back( { elementIcon, std::to_string( item->percent ), textColor } );
	}

	// Center item name
	screenPos.x -= textSize.x * 0.5f;
	screenPos.y -= textSize.y * 0.5f * m_lootConfig->hover.m_floatHeight;

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
	ImDrawList *drawList,
	const Vector2F &pos,
	const ImColor &textColor,
	const ImColor &shadowColor,
	const std::string &text,
	float fontScale,
	const Vector2F &shadowOffset )
{
	if( !drawList || text.empty() )
		return;

	ImGui::SetWindowFontScale( fontScale );

	drawList->AddText( ImVec2( pos.x + shadowOffset.x, pos.y + shadowOffset.y ), shadowColor, text.c_str() );
	drawList->AddText( ImVec2{ pos.x, pos.y }, textColor, text.c_str());

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

	Vector2F pos;
	pos.y = params.screenPos.y + params.textSize.y + 4.0f;
	pos.x = params.centerPos.x - totalWidth * 0.5f;

	for( const auto &p : params.iconTextPairs )
	{
		if( p.icon )
		{
			drawList->AddImage( ( ImTextureID )p.icon->m_texId,
								ImVec2{ pos.x, pos.y },
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

bool LootDisplay::CheckDisplayFilter( s_cached_item *item )
{
	auto config = Config::Get().loot_reader;

	if( item->item_id.var.item_type == meseta && m_lootConfig->m_itemTypeFilter[ meseta ] )
	{
		return true;
	}

	if( !m_lootConfig->m_itemTypeFilter[ item->item_id.var.item_type ] )
	{
		return false;
	}

	if( item->rarity < m_lootConfig->m_minimumRarity || item->rarity > m_lootConfig->m_maximumRarity )
	{
		return false;
	}

	return true;
}

void LootDisplay::TryFixErrorItem( s_loot_entity *entity )
{
	// If the player is close to the item, just teleport it to the player.
	if( m_player->position.DistanceFrom( entity->position ) < 25.0f )
	{
		entity->position = m_player->position;
		return;
	}

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