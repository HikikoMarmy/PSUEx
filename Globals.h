#pragma once

#include <Windows.h>
#include <d3d9.h>
#include "PSUStructs/Vector3F.hpp"
#include "PSUStructs/Matrix4x4.hpp"

namespace Global
{
	inline HWND hWnd;
	inline int screenWidth;
	inline int screenHeight;
	inline int windowPosX;
	inline int windowPosY;
	inline int monitorLeft;
	inline int monitorTop;
	inline int monitorRight;
	inline int monitorBottom;

	inline LPDIRECT3DDEVICE9 pD3DDevice;

	inline Vector3F cameraPosition;
	inline Vector3F cameraFocus;
	inline Vector3F cameraForward;

	inline Matrix4x4 worldMatrix;
	inline Matrix4x4 viewMatrix;
	inline Matrix4x4 projectionMatrix;
	inline Matrix4x4 minimapMatrix;
	inline bool		 minimapActive;
}