#pragma once

#include <thread>
#include <mutex>

#include "IFWinCtrl.h"
#include "..\PSUStructs\Item.h"
#include "..\PSUStructs\Vector3F.hpp"
#include "..\PSUStructs\Matrix4x4.hpp"

#include "..\ImageMng.h"

class LootDisplay : public IFWinCtrl
{
private:
	const static inline uintptr_t ptr_to_ptr_table = 0x0079D4F4;

	const static inline ImU32 m_color_element[ 7 ] =
	{
		IM_COL32( 255, 255, 255, 255 ),		// Neutral
		IM_COL32( 255, 110, 110, 255 ),		// Fire
		IM_COL32( 115, 115, 255, 255 ),		// Ice
		IM_COL32( 255, 255, 41, 255 ),		// Lightning
		IM_COL32( 255, 128, 0, 255 ),		// Ground
		IM_COL32( 255, 200, 175, 255 ),		// Light
		IM_COL32( 255, 130, 255, 255 )		// Dark
	};

	enum enum_state : uint8_t const
	{
		invalid = 0,
		try_spawn = 1,
		error = 2,
		active = 3,
	};

	enum enum_column_type
	{
		rarity = 0,
		name = 1,
		element = 2,
		distance = 3,
		max_number_of_columns,
	};

	struct s_entity_item_data
	{
		wchar_t name[ 23 ];
		uint16_t _3;
		uint32_t _4;

		union
		{
			struct weapon
			{
				uint16_t max_pp;
				uint16_t max_attack;
				uint16_t hit_rate;

				uint16_t req_stat;
				uint16_t req_type;
				uint8_t req_sex_race;

				uint8_t element;
				uint8_t element_percent;
				uint8_t _1;
				uint8_t type_flag;
				uint8_t rank;
				uint8_t cur_grind;
				uint8_t max_grind;
				uint8_t action_label;
				uint8_t _2;
			} as_weapon;

			struct lineshield
			{
				uint16_t req_stat;
				uint16_t def;
				uint16_t ment;
				uint16_t eva;
				uint16_t end;
				uint16_t req_type;

				uint8_t element : 4;
				uint8_t rank : 4;

				uint8_t element_percent;
				uint8_t blocking : 4;
				uint8_t slot : 4;
				uint8_t req_sex_race;
				uint32_t _1;
			} as_lineshield;

			struct s_unit_item {
				struct s_head_unit {
					int16_t tech;
					int16_t ment;
					uint8_t technic_modifier_range;
					uint8_t technic_modifier_pp;
					uint8_t technic_modifier_cast;
					uint8_t _[ 9 ];
				} head;

				struct s_arm_unit {
					int16_t att;
					int16_t acc;
					uint8_t auto_damage_level;
					uint8_t striking_modifier_range;
					uint8_t striking_modifier_pp;
					uint8_t striking_modifier_speed;
					uint8_t bullet_modifier_range;
					uint8_t bullet_modifier_pp;
					uint8_t bullet_modifier_speed;
					uint8_t _[ 5 ];
				} arm;

				struct s_body_unit {
					int16_t def;
					int16_t eva;

					struct s_element {
						uint8_t boost;
						struct s_resist {
							uint8_t burn : 1;
							uint8_t poison : 1;
							uint8_t infection : 1;
							uint8_t shock : 1;
							uint8_t silence : 1;
							uint8_t freeze : 1;
							uint8_t sleep : 1;
							uint8_t unreserved : 2;
							uint8_t stun : 1;
							uint8_t confuse : 1;
							uint8_t incapacitate : 1;
							uint8_t stat_up : 1;
							uint8_t stat_dwn : 1;
						} resists;
					} element;

					uint8_t _[ 9 ];
				} body;

				struct s_extra_unit {
					int16_t end;
					int16_t _1;
					uint8_t auto_recovery_level;
					uint8_t auto_pp_recovery;
					uint8_t suv_model_id;
					uint8_t visual_effect;

					uint8_t _2[ 8 ];
				} extra;
			};

			struct grinder
			{
				uint32_t max_quantity;
				uint8_t _[ 16 ];
			} as_grinder;

			std::byte raw[ 20 ];
		} const_data;

		uint32_t unique_id;
		psu_item_id item_id;

		union
		{
			struct weapon {
				uint8_t _1[ 3 ];
				uint8_t properties;
				uint16_t current_pp, max_pp;
				uint16_t grind_bonus;
				uint8_t rank : 4;
				uint8_t grind : 4;
				uint8_t rarity : 4;
				uint8_t account_flag : 4;
				uint8_t element;
				uint8_t element_percent;
				uint8_t type_flag;
				uint8_t equip_hand : 4;
				uint8_t grind_fail : 4;

				struct pa_data {
					uint8_t pa_iindex;
					uint8_t pa_level;
				} pa[ 4 ];
			} as_weapon;

			struct lineshield {
				uint8_t _1[ 3 ];
				uint8_t properties;
				uint8_t _2[ 6 ];
				uint8_t rank;
				uint8_t rarity : 4;
				uint8_t account_flag : 4;
				uint8_t element;
				uint8_t element_percent;
				uint8_t slots : 4;
				uint8_t _3[ 9 ];
			} as_lineshield;

			struct generic 				{
				uint8_t quantity;
				uint8_t _1[ 2 ];
				uint8_t properties;
				uint8_t _2[ 6 ];
				uint8_t rank : 4;
				uint8_t _3 : 4;
				uint8_t rarity : 4;
				uint8_t account_flag : 4;
				uint8_t element;
				uint8_t element_percent;
			} generic;

			uint8_t raw[ 24 ];
		} var_data;
	};

	struct s_loot_entity
	{
		uint32_t unknown;
		s_loot_entity *next;
		enum_state state;
		uint16_t map_id;
		uint32_t _2;
		Vector3F position;
		s_entity_item_data item;

		// Something about the floor model
		uint32_t _mdl_ptr1;
		uint32_t _mdl_ptr2;
	};

	struct s_icon_text_pair {
		sptr_image icon;
		std::string text;
		ImU32 textColor;
	};

	struct s_hover_draw_param {
		ImVec2 centerPos;
		ImVec2 screenPos;
		ImVec2 textSize;
		std::string itemName;
		std::vector<s_icon_text_pair> iconTextPairs;

		ImU32 iconColor;
		ImU32 shadowColor;
		ImColor textColor;
		ImColor elementColor;
		float fontScale = 1.25f;
	};

	// Keep track of bad entities that need to be re-spawned.
	std::unordered_map< int32_t, Vector3F > m_errorItemList;
	int32_t m_errorItemPass;

private:
	void Display_Item_Weapon( s_loot_entity *entity );
	void Display_Item_Lineshield( s_loot_entity *entity );
	void Display_Item_Misc( s_loot_entity *entity );

	sptr_image GetItemIcon( const s_loot_entity *entity );
	sptr_image GetConsumableIcon( const s_loot_entity *entity );

	void DrawHoverItem( s_loot_entity *entity );
	void DrawTextShadowed(
		ImDrawList *drawList,
		const ImVec2 &pos,
		const ImColor &textColor,
		const ImColor &shadowColor,
		const std::string &text,
		float fontScale = 1.0f,
		const ImVec2 &shadowOffset = ImVec2( 1.0f, 1.0f )
	);

	void DrawIconsBelow( s_hover_draw_param &params );

	bool CheckDisplayFilter( s_loot_entity *entity );
	void TryFixErrorItem( s_loot_entity *entity );

	bool ToScreenSpace( const Vector3F &worldPos, ImVec2 &screenPos );
	bool ToScreenSpaceMinimap( const Vector3F &worldPos, ImVec2 &screenPos );	// Will get the minimap matrix one day.

	// This can definitely go into a utility file later.
	std::string WideToUTF8( const wchar_t *wideString );

public:
	LootDisplay( const LootDisplay & ) = delete;
	LootDisplay &operator=( const LootDisplay & ) = delete;

	LootDisplay();
	~LootDisplay();

	static std::shared_ptr< LootDisplay > Create()
	{
		return std::make_shared< LootDisplay >();
	}

	void SetInitialWindowPosition() override;
	void Render() override;
};