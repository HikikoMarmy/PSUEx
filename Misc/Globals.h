#pragma once

#include <d3d9.h>
#include <Windows.h>
#include "../PSUStructs/Vector3F.hpp"
#include "../PSUStructs/Matrix4x4.hpp"

namespace Global
{
	inline HWND hWnd;
	inline float screenWidth;
	inline float screenHeight;
	inline int windowPosX;
	inline int windowPosY;
	inline int monitorLeft;
	inline int monitorTop;
	inline int monitorRight;
	inline int monitorBottom;

	inline LPDIRECT3DDEVICE9 pD3DDevice;
	// Called with each Direct3D device the game creates (UI scaling).
	inline void ( *OnDeviceCreated )( LPDIRECT3DDEVICE9 pDevice ) = nullptr;

	inline Vector3F cameraPosition;
	inline Vector3F cameraFocus;
	inline Vector3F cameraForward;

	inline Matrix4x4 viewProjectionMatrix;
	inline Matrix4x4 minimapMatrix;
	inline bool		 minimapActive;
}