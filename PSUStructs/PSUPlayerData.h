#pragma once

#include "../PSUStructs/Vector3F.hpp"

class PSUPlayer {
	static const uintptr_t ptr_to_ptr_table = 0x005CA8AC;
	static inline uintptr_t ptr_to_player = 0x00000000;

	static bool GetPlayerPtr();

public:
	static PSUPlayer &Get();

	static bool IsReady()
	{
		return ptr_to_player != 0x00000000;
	}

	PSUPlayer( const PSUPlayer & ) = delete;
	PSUPlayer &operator=( const PSUPlayer & ) = delete;

	PSUPlayer()
	{
	}

		uint8_t _pad[ 276 ];
	Vector3F position;
		uint32_t _pad1;
	uint32_t angle;
		uint8_t _pad2[ 280 ];
	uint32_t quest_id;
	uint32_t zone_id;
	uint32_t map_id;
};

