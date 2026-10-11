/*
* UI scaling (UI_SCALE).
*
* At high resolutions the game's UI is drawn at its native pixel size, so it
* gets small. With UI_SCALE above 1, the game lays its UI out on a screen
* UI_SCALE times smaller than the real one (by hooking the layout-size
* query), and the draw hooks scale its pre-transformed (XYZRHW) vertices
* back up, so the UI is drawn larger and stays sharp.
*
* The calls that set up render targets and the camera are pointed back at
* the real size. Left unscaled: draws that already cover the whole target
* (fades, backdrops), draws into render targets smaller than the screen, the
* bloom pass, and anything not drawn by the game itself (other mods'
* overlays). Viewports sized for the smaller screen are widened back to the
* real one so the scaled UI isn't clipped.
*
* Settings ([custom] in PsuIlluminus.ini):
*   UI_SCALE  1 (off) to 8, decimals allowed (1.5 at 1920x1080, 2 at
*             2560x1440, 3 at 3840x2160 draw the UI as large as at
*             1280x720). Limited so the UI still has at least 640x480.
*/

#pragma once

#include "../PSUEx.h"

#include <intrin.h>
#include <cfloat>
#include <cmath>
#include <string>
#include <vector>

namespace UIScaling
{
	// void (int out[2]) (cdecl), the layout-size query: the size the UI lays itself out for.
	typedef void( __cdecl *LayoutSize_t )( int *out );
	LayoutSize_t pOriginal_LayoutSize = reinterpret_cast< LayoutSize_t >( 0x00777160 );

	// bool (int *w, int *h) (cdecl), the back-buffer-size query.
	const uintptr_t BACKBUFFER_SIZE_FN = 0x0076E1D0;

	// The back buffer's size the game was started with.
	const uintptr_t BACKBUFFER_WIDTH = 0x00A89120;
	const uintptr_t BACKBUFFER_HEIGHT = 0x00A89124;

	// Calls to the back-buffer-size query that must keep seeing the real size
	// (render targets, the camera's aspect).
	const uintptr_t BACKBUFFER_SIZE_CALLS[] = { 0x0077757E, 0x00777ADF, 0x00777B64, 0x0048CA59, 0x0048CC5C, 0x0048CD19 };
	// Calls to the layout-size query that need the real size too.
	const uintptr_t LAYOUT_SIZE_CALLS[] = { 0x004FF827 };

	// The bloom pass (thiscall, no stack arguments): its full-screen draws use
	// the same pre-transformed vertices but must not be scaled.
	typedef void( __fastcall *BloomPass_t )( void *thisPtr, void *edx );
	BloomPass_t pOriginal_BloomPass = reinterpret_cast< BloomPass_t >( 0x004A12B0 );
	// sub esp,0x128; push esi; mov esi,[0xb9cbc8]
	const uint8_t BLOOM_PASS_CODE[] = { 0x81, 0xEC, 0x28, 0x01, 0x00, 0x00, 0x56, 0x8B, 0x35, 0xC8, 0xCB, 0xB9 };

	// The smallest screen the game's UI is built for.
	const int SMALLEST_LAYOUT_W = 640;
	const int SMALLEST_LAYOUT_H = 480;
	const float MAX_SCALE = 8.0f;

	// IDirect3DDevice9 methods, by their place in the interface.
	const int D3D_RESET = 16;
	const int D3D_SET_RENDER_TARGET = 37;
	const int D3D_SET_VIEWPORT = 47;
	const int D3D_DRAW_PRIMITIVE_UP = 83;
	const int D3D_DRAW_INDEXED_PRIMITIVE_UP = 84;
	const int D3D_SET_FVF = 89;

	// Largest vertex upload copied and rescaled.
	const size_t MAX_SCALED_BYTES = 0x400000;
	// Every vertex needs at least an x and a y float.
	const size_t MIN_VERTEX_BYTES = 8;

	typedef HRESULT( WINAPI *Reset_t )( IDirect3DDevice9 *, D3DPRESENT_PARAMETERS * );
	typedef HRESULT( WINAPI *SetRenderTarget_t )( IDirect3DDevice9 *, DWORD, IDirect3DSurface9 * );
	typedef HRESULT( WINAPI *SetViewport_t )( IDirect3DDevice9 *, const D3DVIEWPORT9 * );
	typedef HRESULT( WINAPI *DrawPrimitiveUP_t )( IDirect3DDevice9 *, D3DPRIMITIVETYPE, UINT, const void *, UINT );
	typedef HRESULT( WINAPI *DrawIndexedPrimitiveUP_t )( IDirect3DDevice9 *, D3DPRIMITIVETYPE, UINT, UINT, UINT, const void *, D3DFORMAT, const void *, UINT );
	typedef HRESULT( WINAPI *SetFVF_t )( IDirect3DDevice9 *, DWORD );

	Reset_t pOriginal_Reset = nullptr;
	SetRenderTarget_t pOriginal_SetRenderTarget = nullptr;
	SetViewport_t pOriginal_SetViewport = nullptr;
	DrawPrimitiveUP_t pOriginal_DrawPrimitiveUP = nullptr;
	DrawIndexedPrimitiveUP_t pOriginal_DrawIndexedPrimitiveUP = nullptr;
	SetFVF_t pOriginal_SetFVF = nullptr;

	// UI_SCALE as set; back to 1 if the draw hooks can't go in, so the UI
	// isn't laid out smaller without being drawn back up. Nothing is drawn
	// before the device exists, so the layout may shrink from the start.
	float g_wanted = 1.0f;
	bool g_drawHooked = false;

	// The game's current vertex format, render target 0's size (0 until the
	// game sets one) and whether its bloom pass is running. Render thread only.
	DWORD g_fvf = 0;
	UINT g_targetWidth = 0;
	UINT g_targetHeight = 0;
	int g_inBloom = 0;

	// PSUC.exe's image, to tell the game's own draws from other mods'.
	uintptr_t g_gameStart = 0;
	uintptr_t g_gameEnd = 0;

	std::vector< uint8_t > g_scaledCopy;

	bool CalledByGame( void *returnAddress )
	{
		auto at = reinterpret_cast< uintptr_t >( returnAddress );
		return at >= g_gameStart && at < g_gameEnd;
	}

	bool CodeIs( uintptr_t address, const uint8_t *code, size_t size )
	{
		return memcmp( reinterpret_cast< const void * >( address ), code, size ) == 0;
	}

	// The real back-buffer size the game set, if it's sane.
	bool BackbufferSize( int &w, int &h )
	{
		w = *reinterpret_cast< int * >( BACKBUFFER_WIDTH );
		h = *reinterpret_cast< int * >( BACKBUFFER_HEIGHT );
		return w > 0 && h > 0;
	}

	// The scale in effect, limited so the UI is still laid out on at least 640x480.
	float Scale()
	{
		if( g_wanted <= 1.0f ) return 1.0f;
		int w, h;
		if( !BackbufferSize( w, h ) ) return g_wanted;
		float most = std::fmin( static_cast< float >( w ) / SMALLEST_LAYOUT_W, static_cast< float >( h ) / SMALLEST_LAYOUT_H );
		return std::fmax( 1.0f, std::fmin( g_wanted, most ) );
	}

	// Stands in for the back-buffer-size query at the calls that need the real size.
	int __cdecl BackbufferSizeForCall( int *w, int *h )
	{
		int rw, rh;
		if( !BackbufferSize( rw, rh ) )
		{
			rw = 640;
			rh = 480;
		}
		if( w ) *w = rw;
		if( h ) *h = rh;
		return 1;
	}

	// Stands in for the layout-size query at the calls that need the real size.
	void __cdecl LayoutSizeForCall( int *out )
	{
		if( !out ) return;
		if( !BackbufferSize( out[ 0 ], out[ 1 ] ) )
		{
			out[ 0 ] = 640;
			out[ 1 ] = 480;
		}
	}

	// The layout-size query: the virtual screen, real size / the scale.
	void __cdecl Hook_LayoutSize( int *out )
	{
		pOriginal_LayoutSize( out );
		float scale = Scale();
		if( !out || scale <= 1.0f ) return;
		for( int i = 0; i < 2; i++ )
		{
			int v = static_cast< int >( std::lround( out[ i ] / scale ) );
			out[ i ] = v < 1 ? 1 : v;
		}
	}

	void __fastcall Hook_BloomPass( void *thisPtr, void *edx )
	{
		g_inBloom++;
		pOriginal_BloomPass( thisPtr, edx );
		g_inBloom--;
	}

	// Drawing into a render target smaller than the screen (an effect's off-screen buffer).
	bool IntoSmallTarget()
	{
		int w, h;
		if( !BackbufferSize( w, h ) ) return false;
		return g_targetWidth != 0 && g_targetHeight != 0
			&& static_cast< int >( g_targetWidth ) < w && static_cast< int >( g_targetHeight ) < h;
	}

	// Whether the vertices' bounding box already spans (almost) the whole
	// width x height target: a full-screen draw that must not be scaled.
	bool SpansTarget( const uint8_t *data, size_t count, size_t stride, float width, float height )
	{
		float minX = FLT_MAX, maxX = -FLT_MAX, minY = FLT_MAX, maxY = -FLT_MAX;
		for( size_t i = 0; i < count; i++ )
		{
			float x, y;
			memcpy( &x, data + i * stride, 4 );
			memcpy( &y, data + i * stride + 4, 4 );
			if( !std::isfinite( x ) || !std::isfinite( y ) ) continue;
			minX = std::fmin( minX, x );
			maxX = std::fmax( maxX, x );
			minY = std::fmin( minY, y );
			maxY = std::fmax( maxY, y );
		}
		return minX <= maxX && maxX - minX >= width * 0.98f && maxY - minY >= height * 0.98f;
	}

	// Vertices a DrawPrimitiveUP call consumes, or 0 for an unknown type.
	size_t VerticesUsed( D3DPRIMITIVETYPE type, UINT count )
	{
		switch( type )
		{
			case D3DPT_POINTLIST: return count;
			case D3DPT_LINELIST: return static_cast< size_t >( count ) * 2;
			case D3DPT_LINESTRIP: return static_cast< size_t >( count ) + 1;
			case D3DPT_TRIANGLELIST: return static_cast< size_t >( count ) * 3;
			case D3DPT_TRIANGLESTRIP:
			case D3DPT_TRIANGLEFAN: return static_cast< size_t >( count ) + 2;
			default: return 0;
		}
	}

	// A scaled-up copy of the `count` vertices at `data`, or nullptr to draw
	// them as they are. The copy lives until the next call.
	const void *Scaled( const void *data, size_t count, UINT stride )
	{
		float scale = Scale();
		if( !data || stride < MIN_VERTEX_BYTES || scale <= 1.0f || g_inBloom
			|| !( g_fvf & D3DFVF_XYZRHW ) || IntoSmallTarget() )
			return nullptr;

		if( count == 0 || count > MAX_SCALED_BYTES / stride ) return nullptr;
		size_t bytes = count * stride;

		int w, h;
		float fw = 640.0f, fh = 480.0f;
		if( BackbufferSize( w, h ) )
		{
			fw = static_cast< float >( w );
			fh = static_cast< float >( h );
		}
		if( g_targetWidth && g_targetHeight )
		{
			fw = std::fmin( fw, static_cast< float >( g_targetWidth ) );
			fh = std::fmin( fh, static_cast< float >( g_targetHeight ) );
		}
		auto src = static_cast< const uint8_t * >( data );
		if( SpansTarget( src, count, stride, fw, fh ) ) return nullptr;

		g_scaledCopy.assign( src, src + bytes );
		// Pre-transformed vertices sit half a pixel before the pixel centres
		// they mean, so the scaling is about pixel centres: scaling the raw
		// value would leave the UI up to a pixel off its texels and blur it.
		for( size_t i = 0; i < count; i++ )
		{
			uint8_t *v = g_scaledCopy.data() + i * stride;
			float x, y;
			memcpy( &x, v, 4 );
			memcpy( &y, v + 4, 4 );
			if( std::isfinite( x ) && std::isfinite( y ) )
			{
				x = ( x + 0.5f ) * scale - 0.5f;
				y = ( y + 0.5f ) * scale - 0.5f;
				memcpy( v, &x, 4 );
				memcpy( v + 4, &y, 4 );
			}
		}
		return g_scaledCopy.data();
	}

	HRESULT WINAPI Hook_SetFVF( IDirect3DDevice9 *device, DWORD fvf )
	{
		if( CalledByGame( _ReturnAddress() ) ) g_fvf = fvf;
		return pOriginal_SetFVF( device, fvf );
	}

	HRESULT WINAPI Hook_DrawPrimitiveUP( IDirect3DDevice9 *device, D3DPRIMITIVETYPE type, UINT count, const void *data, UINT stride )
	{
		const void *copy = nullptr;
		if( CalledByGame( _ReturnAddress() ) )
		{
			size_t vertices = VerticesUsed( type, count );
			if( vertices ) copy = Scaled( data, vertices, stride );
		}
		return pOriginal_DrawPrimitiveUP( device, type, count, copy ? copy : data, stride );
	}

	HRESULT WINAPI Hook_DrawIndexedPrimitiveUP( IDirect3DDevice9 *device, D3DPRIMITIVETYPE type, UINT minIndex, UINT vertices, UINT count,
		const void *indices, D3DFORMAT indexFormat, const void *data, UINT stride )
	{
		// The indices address vertices minIndex .. minIndex + vertices, all of
		// which must be in the copy.
		const void *copy = nullptr;
		if( CalledByGame( _ReturnAddress() ) )
			copy = Scaled( data, static_cast< size_t >( minIndex ) + vertices, stride );
		return pOriginal_DrawIndexedPrimitiveUP( device, type, minIndex, vertices, count, indices, indexFormat, copy ? copy : data, stride );
	}

	HRESULT WINAPI Hook_SetViewport( IDirect3DDevice9 *device, const D3DVIEWPORT9 *viewport )
	{
		float scale = Scale();
		int w, h;
		if( viewport && scale > 1.0f && CalledByGame( _ReturnAddress() ) && !IntoSmallTarget() && BackbufferSize( w, h ) )
		{
			// A viewport sized for the virtual screen, widened back to the real screen.
			int vw = static_cast< int >( std::lround( w / scale ) );
			int vh = static_cast< int >( std::lround( h / scale ) );
			bool real = viewport->Width == static_cast< DWORD >( w ) && viewport->Height == static_cast< DWORD >( h );
			if( !real && std::abs( vw - static_cast< int >( viewport->Width ) ) < 2 && std::abs( vh - static_cast< int >( viewport->Height ) ) < 2 )
			{
				D3DVIEWPORT9 fixed = *viewport;
				fixed.Width = static_cast< DWORD >( w );
				fixed.Height = static_cast< DWORD >( h );
				return pOriginal_SetViewport( device, &fixed );
			}
		}
		return pOriginal_SetViewport( device, viewport );
	}

	HRESULT WINAPI Hook_SetRenderTarget( IDirect3DDevice9 *device, DWORD index, IDirect3DSurface9 *surface )
	{
		HRESULT hr = pOriginal_SetRenderTarget( device, index, surface );
		if( SUCCEEDED( hr ) && index == 0 )
		{
			D3DSURFACE_DESC desc = {};
			if( surface && SUCCEEDED( surface->GetDesc( &desc ) ) )
			{
				g_targetWidth = desc.Width;
				g_targetHeight = desc.Height;
			}
			else
			{
				g_targetWidth = g_targetHeight = 0;
			}
		}
		return hr;
	}

	// After a device Reset the game sets its render target again.
	HRESULT WINAPI Hook_Reset( IDirect3DDevice9 *device, D3DPRESENT_PARAMETERS *params )
	{
		g_targetWidth = g_targetHeight = 0;
		return pOriginal_Reset( device, params );
	}

	// Whether the call at `site` goes to `expected`.
	bool CallsTo( uintptr_t site, uintptr_t expected )
	{
		auto code = reinterpret_cast< const uint8_t * >( site );
		if( code[ 0 ] != 0xE8 ) return false;
		int32_t rel;
		memcpy( &rel, code + 1, 4 );
		return site + 5 + rel == expected;
	}

	// Points the call at `site` at `to`.
	void RetargetCall( uintptr_t site, void *to )
	{
		auto code = reinterpret_cast< uint8_t * >( site );
		int32_t rel = static_cast< int32_t >( reinterpret_cast< uintptr_t >( to ) - ( site + 5 ) );
		DWORD old;
		if( !VirtualProtect( code + 1, 4, PAGE_EXECUTE_READWRITE, &old ) ) return;
		memcpy( code + 1, &rel, 4 );
		VirtualProtect( code + 1, 4, old, &old );
		FlushInstructionCache( GetCurrentProcess(), code, 5 );
	}

	// Called with each Direct3D device the game creates: hooks the draws
	// (once; every device runs the same code).
	void OnDevice( IDirect3DDevice9 *device )
	{
		if( g_drawHooked || g_wanted <= 1.0f || !device ) return;
		void **vTable = *reinterpret_cast< void *** >( device );
		pOriginal_Reset = reinterpret_cast< Reset_t >( vTable[ D3D_RESET ] );
		pOriginal_SetRenderTarget = reinterpret_cast< SetRenderTarget_t >( vTable[ D3D_SET_RENDER_TARGET ] );
		pOriginal_SetViewport = reinterpret_cast< SetViewport_t >( vTable[ D3D_SET_VIEWPORT ] );
		pOriginal_DrawPrimitiveUP = reinterpret_cast< DrawPrimitiveUP_t >( vTable[ D3D_DRAW_PRIMITIVE_UP ] );
		pOriginal_DrawIndexedPrimitiveUP = reinterpret_cast< DrawIndexedPrimitiveUP_t >( vTable[ D3D_DRAW_INDEXED_PRIMITIVE_UP ] );
		pOriginal_SetFVF = reinterpret_cast< SetFVF_t >( vTable[ D3D_SET_FVF ] );

		DetourTransactionBegin();
		DetourUpdateThread( GetCurrentThread() );
		DetourAttach( &( PVOID & )pOriginal_Reset, reinterpret_cast< PVOID >( Hook_Reset ) );
		DetourAttach( &( PVOID & )pOriginal_SetRenderTarget, reinterpret_cast< PVOID >( Hook_SetRenderTarget ) );
		DetourAttach( &( PVOID & )pOriginal_SetViewport, reinterpret_cast< PVOID >( Hook_SetViewport ) );
		DetourAttach( &( PVOID & )pOriginal_DrawPrimitiveUP, reinterpret_cast< PVOID >( Hook_DrawPrimitiveUP ) );
		DetourAttach( &( PVOID & )pOriginal_DrawIndexedPrimitiveUP, reinterpret_cast< PVOID >( Hook_DrawIndexedPrimitiveUP ) );
		DetourAttach( &( PVOID & )pOriginal_SetFVF, reinterpret_cast< PVOID >( Hook_SetFVF ) );
		if( DetourTransactionCommit() != NO_ERROR )
		{
			g_wanted = 1.0f;
			printf( "UIScaling: couldn't hook the draws; UI scaling off\n" );
			return;
		}
		g_drawHooked = true;
		printf( "UIScaling: on, UI_SCALE = %.2f\n", g_wanted );
	}

	// Called inside DetoursHookAll's transaction.
	void Hook()
	{
		std::wstring value = PSUIni::Get().ReadString( L"custom", L"UI_SCALE", L"1" );
		float wanted = wcstof( value.c_str(), nullptr );
		if( !std::isfinite( wanted ) || wanted <= 1.0f ) return;

		HMODULE game = GetModuleHandleW( nullptr );
		auto dos = reinterpret_cast< const IMAGE_DOS_HEADER * >( game );
		auto nt = reinterpret_cast< const IMAGE_NT_HEADERS * >( reinterpret_cast< const uint8_t * >( game ) + dos->e_lfanew );
		g_gameStart = reinterpret_cast< uintptr_t >( game );
		g_gameEnd = g_gameStart + nt->OptionalHeader.SizeOfImage;

		// All or nothing: the code must be what's expected everywhere first.
		if( !CodeIs( reinterpret_cast< uintptr_t >( pOriginal_BloomPass ), BLOOM_PASS_CODE, sizeof( BLOOM_PASS_CODE ) ) )
		{
			printf( "UIScaling: the bloom pass isn't the expected code; UI scaling off\n" );
			return;
		}
		for( uintptr_t site : BACKBUFFER_SIZE_CALLS )
		{
			if( !CallsTo( site, BACKBUFFER_SIZE_FN ) )
			{
				printf( "UIScaling: 0x%08X isn't the expected call; UI scaling off\n", static_cast< unsigned >( site ) );
				return;
			}
		}
		for( uintptr_t site : LAYOUT_SIZE_CALLS )
		{
			if( !CallsTo( site, reinterpret_cast< uintptr_t >( pOriginal_LayoutSize ) ) )
			{
				printf( "UIScaling: 0x%08X isn't the expected call; UI scaling off\n", static_cast< unsigned >( site ) );
				return;
			}
		}

		for( uintptr_t site : BACKBUFFER_SIZE_CALLS )
			RetargetCall( site, reinterpret_cast< void * >( BackbufferSizeForCall ) );
		for( uintptr_t site : LAYOUT_SIZE_CALLS )
			RetargetCall( site, reinterpret_cast< void * >( LayoutSizeForCall ) );

		DetourAttach( &( PVOID & )pOriginal_LayoutSize, reinterpret_cast< PVOID >( Hook_LayoutSize ) );
		DetourAttach( &( PVOID & )pOriginal_BloomPass, reinterpret_cast< PVOID >( Hook_BloomPass ) );
		Global::OnDeviceCreated = OnDevice;
		g_wanted = std::fmin( wanted, MAX_SCALE );
	}
}
