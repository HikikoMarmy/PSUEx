#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <unordered_map>

#include <imgui.h>

class Config {
private:
	struct s_window_info {
		bool m_enabled;
		ImVec2 m_position;
		ImVec2 m_size;
	};

public:
	static Config &Get()
	{
		static Config instance;
		return instance;
	}

	Config( const Config & ) = delete;
	Config &operator=( const Config & ) = delete;

	Config();
	~Config();

	void SaveToFile();
	void LoadFromFile();

public:
	ImGuiStyle m_style;

	struct s_config {
		s_window_info m_windowInfo;
	} config;

	struct s_loot {
		s_window_info m_windowInfo;

		bool m_displayMission;

		int8_t m_minimumElementPercent;
		int8_t m_minimumRarity;
		int8_t m_maximumRarity;

		bool m_colorItemByElement;
		ImColor m_elementColor[ 7 ];
		
		struct s_float_item {
			bool m_displayItem;
			bool m_colorItemByElement;
			bool m_displayElementIcon;
			bool m_displayItemIcon;
			bool m_displayItemRarity;
			bool m_displayFade;
			int m_iconDisplayType;
			float m_floatHeight;
			float m_fadeDistance;
		} hover;
		
		std::array< bool, 4 > m_columnVisibility;
		std::array< bool, 20 > m_itemTypeFilter;
		std::array< bool, 28 > m_weaponTypeFilter;
		std::array< bool, 7 > m_elementFilter;
	} loot_reader;

	struct s_camera {
		s_window_info m_windowInfo;

		float m_smoothing;
		float m_refocusSpeed;
		float m_fov;
		float m_followDistance;
		float m_followHeight;

		float m_rotationSpeedH;
		float m_rotationSpeedV;
		float m_turnAcceleration;
	} camera;
};