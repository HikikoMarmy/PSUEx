/*
* This hook is responsible for fixing a crash that occurs with Slicers
* when the game tries to access a nullptr.
*/

#pragma once

#include "../PSUEx.h"

namespace TItemBulletSlicer
{
	typedef void( __thiscall *TItemBulletSlicer_EffectFrame )( void* thisPtr );
	TItemBulletSlicer_EffectFrame pOriginal_EffectFrame = reinterpret_cast< TItemBulletSlicer_EffectFrame >( 0x004E5700 );

	typedef int( __thiscall *TItemBulletSlicer_sub_4E58F0 )( void *thisPtr );
	TItemBulletSlicer_sub_4E58F0 pOriginal_sub_4E58F0 = reinterpret_cast< TItemBulletSlicer_sub_4E58F0 >( 0x004E58F0 );

	void __fastcall Hook_EffectFrame( void *thisPtr )
	{
		if( !thisPtr ) return;

		int *ptr_19C = *( int ** )( ( uintptr_t )thisPtr + 0x19C );
		if( !ptr_19C )
		{
			printf( "[TItemBulletSlicer::EffectFrame] Skipping function due to NULL *(this + 0x19C)!\n" );
			return;
		}

		int *ptr_28 = *( int ** )( ( uintptr_t )thisPtr + 0x28 );
		if( !ptr_28 )
		{
			printf( "[TItemBulletSlicer::EffectFrame] Skipping function due to NULL *(this + 0x28)!\n" );
			return;
		}

		pOriginal_EffectFrame( thisPtr );
	}

	// Hooked function
	int __fastcall Hook_sub_4E58F0( void *thisPtr )
	{
		printf( "[Hook] SLICER_CRASH_sub_4E58F0 called! thisPtr: %p\n", thisPtr );

		if( !thisPtr ) return 0;

		int *ptr_19C = *( int ** )( ( uintptr_t )thisPtr + 0x19C );
		if( !ptr_19C )
		{
			printf( "[TItemBulletSlicer::4E58F0] Skipping function due to NULL *(this + 0x19C)!\n" );
			return 0;
		}

		int *ptr_28 = *( int ** )( ( uintptr_t )thisPtr + 0x28 );
		if( !ptr_28 )
		{
			printf( "[TItemBulletSlicer::EffectFrame] Skipping function due to NULL *(this + 0x28)!\n" );
			return 0;
		}

		return pOriginal_sub_4E58F0( thisPtr );
	}

	void Hook()
	{
		DetourAttach( &( PVOID & )pOriginal_EffectFrame, reinterpret_cast< PVOID >( Hook_EffectFrame ) );
		DetourAttach( &( PVOID & )pOriginal_sub_4E58F0, reinterpret_cast< PVOID >( Hook_sub_4E58F0 ) );
	}
}