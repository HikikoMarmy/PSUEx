#pragma once

#include "../PSUEx.h"

//	0076E330
//	Audio_PlayADX( char *adxName, s_param *param )
namespace sub_76E330
{
	struct s_param
	{
		int a1;
		float a2;
		float a3;
		float a4;
	};

	typedef bool( __cdecl *functionType )( char *adxName, s_param *param );

	functionType pOriginalFunction = reinterpret_cast< functionType >( 0x0076E330 );

	bool Audio_PlayADX( char *adxName, s_param *param )
	{
		printf( "Hook_sub_36E330\n" );
		printf( "ADX Name: %s\n", adxName );

		printf( "A1: %u\n", param->a1 );
		printf( "A2: %f\n", param->a2 );
		printf( "A3: %f\n", param->a3 );
		printf( "A4: %f\n", param->a4 );

		return pOriginalFunction( adxName, param );
	}

	void Hook()
	{
		DetourAttach( &( PVOID & )pOriginalFunction, reinterpret_cast< PVOID >( Audio_PlayADX ) );
	}
}