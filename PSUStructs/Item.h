#pragma once
#include <bitset>

constexpr int max_item_name_length = 23;
constexpr int max_item_desc_length = 196;
constexpr int max_element_percent = 50;

enum enum_item_type : uint8_t
{
	weapon = 1,
	line_shield,
	consumable,
	striking_pa,
	bullet_pa,
	technic_pa,
	material,
	shield_unit,
	clothing,
	part,
	decor,
	trap,
	board,
	pm_device,
	grinder,
	boost,
	tool,
	meseta,
	max_number_of_item_types
};

enum enum_element_type : uint8_t
{
	neutral = 0,
	fire = 1,
	ice = 2,
	lightning = 3,
	ground = 4,
	light = 5,
	dark = 6,
	max_number_of_elements
};

enum enum_weapon_category : uint8_t
{
	sword = 1,
	knuckle,
	spear,
	double_Saber,
	axe,
	twin_sabers,
	twin_daggers,
	twin_claws,
	saber,
	dagger,
	claw,
	whip,
	slicer,
	rifle,
	shotgun,
	longbow,
	grenade_launcher,
	laser_cannon,
	twin_handgun,
	handgun,
	crossbow,
	card,
	machinegun,
	rod,
	wand,
	tcsm,
	rcsm,
	max_number_of_weapon_categories
};

static const char* item_weapon_name[] = {
	"Sword",
	"Knuckle",
	"Spear",
	"Double Saber",
	"Axe",
	"Twin Sabers",
	"Twin Daggers",
	"Twin Claws",
	"Saber",
	"Dagger",
	"Claw",
	"Whip",
	"Slicer",
	"Rifle",
	"Shotgun",
	"Longbow",
	"Grenade Launcher",
	"Laser Cannon",
	"Twin Handgun",
	"Handgun",
	"Crossbow",
	"Card",
	"Machinegun",
	"Rod",
	"Wand",
	"TCSM",
	"RCSM"
};

enum enum_equip_hand : uint8_t
{
	two_handed = 0,
	right_handed,
	left_handed
};

enum enum_item_trade_flag
{
	item_unknown_01 = 0,
	item_unknown_02,
	item_unknown_04,
	item_unknown_08,
	item_unknown_10,
	item_unknown_20,
	item_is_locked,
	item_is_unsellable,
};

enum enum_item_trap_flag : uint8_t
{
	damage_trap_low = 0,
	status_trap_low = 1,
	unknown_trap = 1 << 1,
	can_place_1 = 1 << 2,
	can_place_2 = 1 << 3,
	can_place_4 = 1 << 4,
	damage_trap_high = 1 << 5,
	status_trap_high = 1 << 6,
	firework_trap_high = ( damage_trap_high | status_trap_high ),
	player_targeting = 1 << 7
};

struct s_grinder_item
{
	uint8_t type;
	uint8_t rank;
	uint8_t quality;
};

struct s_grinder_const
{
	uint32_t max_quantity;
	uint8_t _[ 16 ];
};

struct s_grinder_variable
{
	uint8_t quantity;
	uint8_t _1[ 2 ];
	uint8_t properties;
	uint32_t max_quantity;
	uint16_t _2;
	uint8_t rank;
	uint8_t rarity : 4;
	uint8_t account_flag : 4;
	uint32_t _3;
	uint32_t _4;
	uint32_t _5;
};

union psu_item_id {
	uint32_t as_u32;

	struct s_var {
		enum_item_type item_type;
		uint8_t item_category;
		uint8_t item_index;
		uint8_t modifier : 4;
		uint8_t param : 4;
	} var;
};

struct s_item
{
	psu_item_id item_id;

	uint32_t unique_id;		//Can only be one in the zone
	uint32_t gift_owner_id;

	wchar_t name[ max_item_name_length + 1 ];
	wchar_t description[ max_item_desc_length + 1 ];
	uint8_t rarity;
	uint8_t rank;
	uint8_t manufacturer;

	uint8_t quantity;
	uint8_t max_quantity;

	uint8_t element;
	uint8_t element_percent;

	struct s_properties
	{
		uint8_t account_bound;
		bool can_use_gc;
		std::bitset< 8 > flag;
	} properties;

	union
	{
		//s_weapon_item as_weapon;
		//s_lineshield_item as_lineshield;
		//s_consumable_item as_consumable;
		//s_pa_disc as_disc;
		//s_clothing_item as_clothing;
		//s_unit_item as_unit;
		s_grinder_item as_grinder;
		//s_decor_item as_decoration;
		//s_music_disc_item as_music_disc;
		//s_room_ticket as_ticket;
		//s_trap_item as_trap;
		//s_board_item as_board;
		//s_boost_item as_boost;
	};
};
