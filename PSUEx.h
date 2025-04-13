#pragma once

#include <cstdio>
#include <d3d9.h>
#include <d3dx9.h>

#include "Windows.h"
#include "WinBase.h"
#include "detours.h"
#include "resource.h"

#include "PSUIni.hpp"
#include "PSUExConfig.h"

#include "Misc/Globals.h"
#include "Misc/Constants.h"

#include "PSUStructs/Item.h"
#include "PSUStructs/PSUWorld.hpp"
#include "PSUStructs/Vector2F.hpp"
#include "PSUStructs/Vector3F.hpp"
#include "PSUStructs/Matrix4x4.hpp"

#include "PSUMemory/Memory.hpp"
#include "PSUMemory/PatchRegistry.h"

#include "PSUOverlay/PSUOverlay.h"

#include "PSUDetour/Hook_EntryPoint.hpp"
#include "PSUDetour/Hook_CreateWindow.hpp"
#include "PSUDetour/Hook_InitializeD3D9.hpp"
#include "PSUDetour/Hook_GetCPUName.hpp"
#include "PSUDetour/Hook_GameFlags.hpp"
#include "PSUDetour/Hook_UIScale.hpp"
#include "PSUDetour/Hook_TItemBulletSlicer.hpp"
#include "PSUDetour/Hook_TCameraTaskPlay.hpp"

#include "PSUDetour/Hook_00774F90.hpp"
#include "PSUDetour/Hook_004CF9A0.hpp"
#include "PSUDetour/Hook_0050B790_LogChatMessage.hpp"
#include "PSUDetour/Hook_0074B490_memset32.hpp"
#include "PSUDetour/Hook_0076E330_Audio_PlayADX.hpp"
#include "PSUDetour/Hook_00814040.hpp"
