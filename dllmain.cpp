#include "PSUEx.h"
#include "ExceptionHandler.h"

static void
DetoursHookAll()
{
	DetourTransactionBegin();
	{
		DetourUpdateThread( GetCurrentThread() );
		DetourAttach( &( PVOID & )Original_WinMain, Hook_WinMain );
		DetourAttach( &( PVOID & )Original_CreateWindowExA, Hook_CreateWindowExA );

		sub_774F90::Hook();			// Initialize DirectInput	
		sub_776590::Hook();			// Initialize Direct3D

		sub_50B790::Hook();			// Chat Logger
		sub_74B490::Hook();			// memset Crash
		sub_814040::Hook();			// File Processing

		HardwareInfo::Hook();		// CPU Affinity Fix
		TItemBulletSlicer::Hook();	// Slicer Crash
		TFlagManager::Hook();		// Flag Manager
		TCameraTaskPlay::Hook();	// Camera Position
		UIScaling::Hook();			// UI Scaling

		//sub_76E330::Hook();	// ADX Audio Player
		//sub_777160::Hook();	// Unknown Function
		//sub_770AF0::Hook();	// Something with File Processing
		//sub_783120::Hook();
	}
	DetourTransactionCommit();
}

static void
DetoursUnhookAll()
{
	DetourTransactionBegin();
	{
		DetourUpdateThread( GetCurrentThread() );
		DetourDetach( &( PVOID & )Original_CreateWindowExA, Hook_CreateWindowExA );
		DetourDetach( &( PVOID & )Original_WinMain, Hook_WinMain );
	}
	DetourTransactionCommit();
}

BOOL
APIENTRY DllMain( HMODULE hModule,
					   DWORD  ul_reason_for_call,
					   LPVOID lpReserved
)
{
	switch( ul_reason_for_call )
	{
		case DLL_PROCESS_ATTACH:
		{
			DisableThreadLibraryCalls( hModule );
			SetupCrashHandler();

		#ifdef _DEBUG
			AllocConsole();
			freopen_s( ( FILE ** )stdout, "CONOUT$", "w", stdout );
		#endif
			
			DetoursHookAll();
		}
		break;

		case DLL_PROCESS_DETACH:
		{
		#ifdef _DEBUG
			fclose( stdout );
			FreeConsole();
		#endif

			Config().Get().SaveToFile();
			DetoursUnhookAll();
		}
		break;
	}
	return TRUE;
}
