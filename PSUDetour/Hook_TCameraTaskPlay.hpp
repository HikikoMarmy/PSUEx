#pragma once

#include "../PSUEx.h"

namespace TCameraTaskPlay
{
	struct CameraTaskPlayData {
		void *ptr_a;               // Possibly vtable
		void *ptr_b;               // Possibly vtable

		uint32_t m_bitflags;         // Flags related to camera states or modes. Changing the first byte to 01 causes the camera to stop following.
		void *selfPtr;             // Pointer to self

		uint32_t unknown_a;        // Possibly an ID or reference index
		void *m_ptr_d;               // Another Pointer (Potentially related to task management)
		char padding_18[ 0x08 ];

		uint16_t m_state;            // Camera State (Bitflag or Enum)
		uint16_t unknown_b;        // Counter or mode indicator (Increases after first-person view)

		void *m_ptr_e;               // Unknown Pointer
		void *m_ptr_f;               // Unknown Pointer
		void *m_playerDataPtr;       // Pointer to player data

		int32_t m_playerState;       // Player's current state

		float m_leadOffset;          // Camera Lead Offset
		float m_focusHeight;         // Height above player where camera focuses
		float m_offsetAboveFocus;    // Offset above focus point
		float m_nearDeadZone;        // Minimum distance from player to focus point

		float m_followDistanceNear;           // Minimum follow distance from the player
		float m_followDistanceFar;            // Maximum follow distance from the player
		float m_followSmoothing;		// Smoothness factor for following

		float m_refocusSpeed;         // Speed of refocus adjustment
		float rotationSpeed;        // Camera rotation speed when refocusing (higher = slower);
		float fpsViewSpeed;			// Transition speed to First Person
		float unknown_c;            // Unknown float

		float m_updateDist;			// Update Distance ( 0 for super smoothing )

		float m_refocusHeight;        // Height adjustment when refocusing
		float m_refocusDistanceNear;  // COULD BE WRONG
		float m_refocusDistanceFar;   // Distance adjustment for refocusing

		float m_upperViewAngle;		// Vertical view angle
		float m_lowerViewAngle;		// Lower view angle

		float unknown_d;            // Something with refocusing behind you
		float unknown_e;            // Unknown float

		float m_rotateSpeedH;    // Horizontal refocus turning speed
		float m_rotateSpeedV;    // Vertical refocus turning speed

		float unknown_f;            // Unknown float
		float unknown_g;            // Unknown float

		int32_t unknown_h;          // Unknown int	(Is 1)
		int32_t unknown_i;          // Unknown. Raises and lowers as things appear, sometimes.

		float m_turnAcceleration;     // How fast the camera reaches maximum turning speed (1 for instant, 0.01 for slow)

		char padding_9C[ 44 ];

		Vector3F m_position;          // Actual camera position (X, Y, Z)
		Vector3F m_focusPosition;     // Focus position the camera is trying to look at (X, Y, Z)
	};

	typedef int( __thiscall *function48E4C0 )( int thisPtr );
	function48E4C0 pOriginal48E4C0 = reinterpret_cast< function48E4C0 >( 0x0048E4C0 );

	typedef int( __thiscall *function48DA90 )( int thisPtr );
	function48DA90 pOriginal48DA90 = reinterpret_cast< function48DA90 >( 0x0048DA90 );

	// Calls onCamera Rotation/Refocus
	int __fastcall Hook_48E4C0( int thisPtr )
	{
		// thisPtr may be the same as the global camera pointer
		// haven't checked it.
		
		// Call the original function
		return pOriginal48E4C0( thisPtr );
	}

	// Calls any time the camera updates.
	int __fastcall Hook_48DA90( int thisPtr )
	{
		CameraTaskPlayData *camera = reinterpret_cast< CameraTaskPlayData * >( thisPtr );

		Global::cameraPosition = camera->m_position;
		Global::cameraFocus = camera->m_focusPosition;

		auto viewMatrix = PSUMemory::ReadAbsoluteMemory< Matrix4x4 >( 0x00A85C30 );
		auto projectionMatrix = PSUMemory::ReadAbsoluteMemory< Matrix4x4 >( 0x00A85B30 );
		Global::viewProjectionMatrix = viewMatrix * projectionMatrix;

		Global::cameraForward = Global::cameraFocus - Global::cameraPosition;
		Global::cameraForward.Normalize();

		// Update the camera data with our own settings
		auto &config = Config::Get().camera;

		camera->m_followDistanceNear = Constant::Camera::FOLLOW_NEAR * config.m_followDistance;
		camera->m_followDistanceFar = Constant::Camera::FOLLOW_FAR * config.m_followDistance;
		camera->m_followSmoothing = config.m_smoothing;

		camera->m_refocusDistanceNear = Constant::Camera::REFOCUS_DISTANCE_NEAR * config.m_followDistance;
		camera->m_refocusDistanceFar = Constant::Camera::REFOCUS_DISTANCE_FAR * config.m_followDistance;

		camera->m_focusHeight = config.m_followHeight;
		camera->m_refocusHeight = config.m_followHeight;

		camera->m_refocusSpeed = config.m_refocusSpeed;
		camera->m_rotateSpeedH = Constant::Camera::ROTATION_SPEED_H * config.m_rotationSpeedH;
		camera->m_rotateSpeedV = Constant::Camera::ROTATION_SPEED_V * config.m_rotationSpeedV;

		if( camera->m_turnAcceleration < config.m_turnAcceleration )
		{
			camera->m_turnAcceleration = config.m_turnAcceleration;
		}

		camera->m_updateDist = Constant::Camera::UPDATE_DISTANCE;

		return pOriginal48DA90( thisPtr );
	}

	void Hook()
	{
		//DetourAttach( &( PVOID & )pOriginal48E4C0, reinterpret_cast< PVOID >( Hook_48E4C0 ) );
		DetourAttach( &( PVOID & )pOriginal48DA90, reinterpret_cast< PVOID >( Hook_48DA90 ) );
	}
}