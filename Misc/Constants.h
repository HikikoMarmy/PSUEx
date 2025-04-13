#pragma once

namespace Constant
{
	namespace Camera
	{
		constexpr float FOLLOW_LEAD_OFFSET = 5.0f;
		constexpr float FOLLOW_HEIGHT = 15.0f;
		constexpr float FOLLOW_FOCUS_HEIGHT_OFFSET = 7.5f;
		constexpr float FOLLOW_NEAR_DEAD_ZONE = 10.0f;
		constexpr float FOLLOW_NEAR = 40.0f;
		constexpr float FOLLOW_FAR = 70.0f;
		constexpr float FOLLOW_SMOOTHING = 0.15f;
		constexpr float ROTATION_SPEED = 0.4f;
		constexpr float FPS_VIEW_SPEED = 0.4f;
		constexpr float UPDATE_DISTANCE = 0.01f;
		constexpr float REFOCUS_SPEED = 0.1f;
		constexpr float REFOCUS_HEIGHT = 15.0f;
		constexpr float REFOCUS_DISTANCE_NEAR = 5.0f;
		constexpr float REFOCUS_DISTANCE_FAR = 55.0f;
		constexpr float UPPER_VIEW_ANGLE = 0.785f;  // Approximately 45 degrees in radians
		constexpr float LOWER_VIEW_ANGLE = -0.35f; // Approximately -20 degrees in radians
		constexpr float ROTATION_SPEED_H = 0.04f;
		constexpr float ROTATION_SPEED_V = 0.04f;
		constexpr float TURN_ACCELERATION = 0.1f;
	}
}