/*
* This hook is responsible for the CPU Affinity loop and hardware information.
* It is used to fix the CPU Affinity issue with the game when using 32 cores or more.
*/

#pragma once

#include "../PSUEx.h"

namespace HardwareInfo
{
	typedef bool *( __cdecl *function775990 )( ULONG_PTR *ProcessAffinityMask );
	function775990 pOriginal775990 = reinterpret_cast< function775990 >( 0x00775990 );

	typedef DWORD *( __cdecl *function775870 )( int a1, int a2 );
	function775870 pOriginal775870 = reinterpret_cast< function775870 >( 0x00775870 );

	typedef DWORD *( __cdecl *function775960 )( DWORD *a1, int a2, int a3 );
	function775960 pOriginal775960 = reinterpret_cast< function775960 >( 0x00775960 );

	bool GetCPUName( ULONG_PTR *processAffinityMask )
	{
		if( PSUIni::Get().ReadInt( L"custom", L"CPU_AFFINITY_FIX", 1 ) == 0 )
		{
			return pOriginal775990( processAffinityMask );
		}

		if( processAffinityMask == NULL )
			return false;

		HANDLE hProcess = GetCurrentProcess();
		ULONG_PTR systemAffinityMask;
		SYSTEM_INFO sysInfo;
		GetSystemInfo( &sysInfo );

		int maxLogicalProcessors = sysInfo.dwNumberOfProcessors;

		if( !GetProcessAffinityMask( hProcess, processAffinityMask, &systemAffinityMask ) || !( *processAffinityMask ) )
			return false;

		ULONG_PTR originalAffinityMask = *processAffinityMask;
		int selectedCore = -1;

		for( int i = 0; i < 32; i++ )
		{
			if( ( *processAffinityMask >> i ) & 1 )
			{
				selectedCore = i;
				break;
			}
		}

		if( selectedCore == -1 )
			return false;

		DWORD *cpuInfoBuffer = NULL;
		int coreCount = 0;
		DWORD *cpuData = NULL;
		DWORD *tempPointer = NULL;

		// Temporarily set CPU affinity to a single core to gather CPU information.
		// This is necessary when gathering CPU information on systems with 32 or more cores.
		if( SetProcessAffinityMask( hProcess, 1 << selectedCore ) )
		{
			cpuData = pOriginal775870( selectedCore, coreCount );
			if( cpuData )
			{
				if( cpuInfoBuffer )
				{
					for( tempPointer = cpuInfoBuffer; *tempPointer; tempPointer++ );
					tempPointer = cpuData;
				}
				else
				{
					cpuInfoBuffer = cpuData;
				}
				coreCount++;
			}

			// Restore original CPU affinity so the client doesn't lag out c:
			//SetProcessAffinityMask( hProcess, originalAffinityMask );
		}

		if( cpuInfoBuffer == NULL )
			return false;

		// CPU Name Extraction (Using CPUID Calls)
		if( ( cpuData = pOriginal775960( cpuInfoBuffer, 0, 0x80000004 ) ) == NULL )
			return false;
		processAffinityMask[ 8 ] = cpuData[ 4 ];
		processAffinityMask[ 9 ] = cpuData[ 5 ];
		processAffinityMask[ 10 ] = cpuData[ 6 ];
		processAffinityMask[ 11 ] = cpuData[ 7 ];

		if( ( cpuData = pOriginal775960( cpuInfoBuffer, 0, 0x80000003 ) ) == NULL )
			return false;
		processAffinityMask[ 4 ] = cpuData[ 4 ];
		processAffinityMask[ 5 ] = cpuData[ 5 ];
		processAffinityMask[ 6 ] = cpuData[ 6 ];
		processAffinityMask[ 7 ] = cpuData[ 7 ];

		if( ( cpuData = pOriginal775960( cpuInfoBuffer, 0, 0x80000002 ) ) == NULL )
			return false;
		processAffinityMask[ 0 ] = cpuData[ 4 ];
		processAffinityMask[ 1 ] = cpuData[ 5 ];
		processAffinityMask[ 2 ] = cpuData[ 6 ];
		processAffinityMask[ 3 ] = cpuData[ 7 ];

		return true;
	}

	void Hook()
	{
		DetourAttach( &( PVOID & )pOriginal775990, reinterpret_cast< PVOID >( GetCPUName ) );
	}
}