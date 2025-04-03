#pragma once

namespace PSUWorld
{
	enum class WorldType {
		Lobby = 0,
		PPT,
		Mission,
		Counter,
		Room,
		Unknown,
	};

	static WorldType GetType( uint32_t quest_id )
	{
		switch( quest_id )
		{
			case 0: return WorldType::Unknown; break;

			/* Lobby Quest ID */
			case 1100000:
			case 1101000:
			case 1102000:
			case 1103000:
			case 1105000: return WorldType::Lobby; break;

			case 1104000: return WorldType::PPT; break;

			/* Room Quest ID */
			case 1120000: return WorldType::Room; break;

			/* Every counter... */
			case 0x7FFFFFFF: return WorldType::Counter; break;

			/* Assume it's a mission... */
			default: return WorldType::Mission; break;
		}
	}
}