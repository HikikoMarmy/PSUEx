#include "../Globals.h"

#include "../PSUExConfig.h"
#include "PSUOverlay.h"
#include "Interface/ConfigDisplay.h"
#include "Interface/LootDisplay.h"
#include "Interface/LogDisplay.h"
#include "Interface/CameraDisplay.h"
#include <intrin.h>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );

std::map<IDirect3DSwapChain9 *, int> swapChainMap;
int swapChainCounter = 0;

PSUOverlay::PSUOverlay()
{
	m_bInitialized = false;
	m_bDisplay = false;
	m_bStyleEditor = false;

	m_iWidth = 0;
	m_iHeight = 0;

	m_hWnd = nullptr;
	m_pOriginalEndScene = nullptr;
	m_pOriginalDrawIndexedPrimitive = nullptr;
	m_pOriginalReset = nullptr;
	m_pOriginalPresent = nullptr;
	m_pOriginalCreateAdditionalSwapChain = nullptr;

	m_style = ImGuiStyle();

	m_pOriginalSwapChainPresentsMap.clear();
	m_swapChainCounter = 0;
	m_swapChainMap.clear();
}

PSUOverlay::~PSUOverlay()
{
}

void PSUOverlay::RenderMenuBar()
{
	if( ImGui::BeginMainMenuBar() )
	{
		if( ImGui::BeginMenu( "Configuration" ) )
		{
			if( ImGui::MenuItem( "Style" ) )
			{
				m_bStyleEditor = !m_bStyleEditor;
			}

			if( ImGui::MenuItem( "Windows" ) )
			{
				m_Controls[ WINDOW_CONTROL_ID::WINDOW_CONFIG_DISPLAY ]->Toggle();
			}
			ImGui::EndMenu();
		}

		if( ImGui::BeginMenu( "Window" ) )
		{
			if (ImGui::MenuItem("Loot Reader" ))
			{
				m_Controls[ WINDOW_CONTROL_ID::WINDOW_LOOT_DISPLAY ]->Toggle();
			}

			//if( ImGui::MenuItem( "Log Display" ) )
			//{
			//	m_Controls[ WINDOW_CONTROL_ID::WINDOW_LOG_DISPLAY ]->Toggle();
			//}

			if( ImGui::MenuItem( "Camera Options" ) )
			{
				m_Controls[ WINDOW_CONTROL_ID::WINDOW_CAMERA_DISPLAY ]->Toggle();
			}
			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}
}

void PSUOverlay::Initialize( IDirect3D9 *pD3D )
{
	if( nullptr == pD3D )
	{
		printf( "Initialize: pD3D is null!\n" );
		return;
	}

	void **vTable = *( void *** )pD3D;

	if( nullptr == vTable )
	{
		printf( "Initialize: vTable is null!\n" );
		return;
	}

	m_pOriginalCreateDevice = ( FunctionD3DCreateDevice )vTable[ 16 ];

	DetourTransactionBegin();
	{
		DetourUpdateThread( GetCurrentThread() );
		DetourAttach( &( PVOID & )m_pOriginalCreateDevice, Hook_CreateDevice );
	}
	DetourTransactionCommit();
}

void PSUOverlay::AddControl( WINDOW_CONTROL_ID ID, WinCtrlPtr pCtrl )
{
	//printf( "Storing control in m_Controls at ID %d...\n", ( int )ID );
	m_Controls[ ID ] = pCtrl;
}

void PSUOverlay::ReleaseDirectX()
{
	if( m_bInitialized )
	{
		ReleaseWindow();
		ImGui_ImplDX9_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
	}

	m_bInitialized = false;

	DetourTransactionBegin();
	{
		DetourUpdateThread( GetCurrentThread() );
		DetourDetach( &( PVOID & )m_pOriginalEndScene, Hook_EndScene );
		DetourDetach( &( PVOID & )m_pOriginalReset, Hook_Reset );
		DetourDetach( &( PVOID & )m_pOriginalDrawIndexedPrimitive, Hook_DrawIndexedPrimitive );
		DetourDetach( &( PVOID & )m_pOriginalPresent, Hook_Present );
		//DetourDetach( &( PVOID & )m_pOriginalPresentEx, Hook_PresentEx );
		DetourDetach( &( PVOID & )m_pOriginalCreateAdditionalSwapChain, Hook_CreateAdditionalSwapChain );
		//DetourDetach( &( PVOID & )m_pOriginalSwapchainPresent, Hook_Swapchain_Present );
	}
	DetourTransactionCommit();
}

void PSUOverlay::InitializeControls()
{
	ImageManager::Get().Initialize( m_pDevice );

	AddControl( WINDOW_CONTROL_ID::WINDOW_CONFIG_DISPLAY, ConfigDisplay::Create() );
	AddControl( WINDOW_CONTROL_ID::WINDOW_LOOT_DISPLAY, LootDisplay::Create() );
	//AddControl( WINDOW_CONTROL_ID::WINDOW_LOG_DISPLAY, LogDisplay::Create() );
	AddControl( WINDOW_CONTROL_ID::WINDOW_CAMERA_DISPLAY, CameraDisplay::Create() );
}

void PSUOverlay::HookWindow()
{
	m_pWndProc = ( WNDPROC )SetWindowLongPtr( Global::hWnd, GWLP_WNDPROC, ( LONG_PTR )WndProc );
}

void PSUOverlay::ReleaseWindow()
{
	SetWindowLongPtr( m_hWnd, GWLP_WNDPROC, ( LONG_PTR )m_pWndProc );
}

LRESULT WINAPI PSUOverlay::WndProc( const HWND hWnd, const UINT msg, const WPARAM wParam, const LPARAM lParam )
{
	//Added INSERT as the key to toggle m_bDisplay
	if (GetAsyncKeyState(VK_INSERT) & 1) m_bDisplay ^= 1;

	//m_bDisplay was false always, so this wasn't working (This enables UI interaction)
	//If you don't want the toggle, you can just make m_bDisplay always true, or remove it from the below if statement
	if( /*m_bDisplay && */ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam) )
	{
		ImGui::GetIO().MouseDrawCursor = ImGui::IsWindowHovered( ImGuiHoveredFlags_AnyWindow );
		return true;
	}
	
	if( m_bInitialized )
	{
		ImGui::GetIO().MouseDrawCursor = false;// ImGui::IsWindowHovered( ImGuiHoveredFlags_AnyWindow );
	}
	
	if( msg == WM_CLOSE )
	{
		ReleaseDirectX();
		ReleaseWindow();
		TerminateProcess( GetCurrentProcess(), 0 );
	}
	
	if( ImGui::GetIO().WantCaptureMouse )
	{
		if( ImGui::IsWindowHovered( ImGuiHoveredFlags_AnyWindow ) )
			return true;
		return false;
	}

	return CallWindowProc( m_pWndProc, hWnd, msg, wParam, lParam );
}

HRESULT APIENTRY PSUOverlay::Hook_CreateDevice( IDirect3D9 *pD3D, UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS *pPresentationParameters, IDirect3DDevice9 **ppReturnedDeviceInterface )
{
	static int deviceCreationCount = 0;

	deviceCreationCount++;

	auto hr = m_pOriginalCreateDevice( pD3D, Adapter, DeviceType, hFocusWindow, BehaviorFlags, pPresentationParameters, ppReturnedDeviceInterface );

	if( SUCCEEDED( hr ) && ppReturnedDeviceInterface )
	{
		IDirect3DDevice9 *pDevice = *ppReturnedDeviceInterface;

		// Log the vTable pointer
		void **vTable = *( void *** )pDevice;

		// Hook only the actual rendering device (the second one)
		if( deviceCreationCount == 2 )
		{
			Global::pD3DDevice = pDevice;

			m_pOriginalReset = ( FunctionD3DReset )vTable[ 0x10 ];
			
			m_pOriginalPresent = ( FunctionD3DPresent )vTable[ 0x42 ];
			m_pOriginalDrawPrimitive = ( FunctionD3DDrawPrimitive )vTable[ 0x51 ]; 
			m_pOriginalDrawIndexedPrimitive = ( FunctionD3DDrawIndexedPrimitive )vTable[ 0x52 ];  
			m_pOriginalDrawPrimitiveUP = ( FunctionD3DDrawPrimitiveUP )vTable[ 0x53 ];  
			m_pOriginalDrawIndexedPrimitiveUP = ( FunctionD3DDrawIndexedPrimitiveUP )vTable[ 0x54 ];  
			m_pOriginalCreateAdditionalSwapChain = ( FunctionCreateAdditionalSwapChain )vTable[ 0x56 ];

			m_pOriginalSetRenderTarget = ( FunctionSetRenderTarget )vTable[ 0x24 ];
			m_pOriginalEndScene = ( FunctionD3DEndScene )vTable[ 0x2A ];
			m_pOriginalSetTransform = ( FunctionSetTransform )vTable[ 0x2C ];
			m_pOriginalSetViewport = ( FunctionSetViewport )vTable[ 0x2F ]; 
			

			DetourTransactionBegin();
			DetourUpdateThread( GetCurrentThread() );

			if( DetourAttach( &( PVOID & )m_pOriginalPresent, Hook_Present ) != NO_ERROR )
			{
				printf( "Failed to hook Present!\n" );
			}

			if( DetourAttach( &( PVOID & )m_pOriginalEndScene, Hook_EndScene ) != NO_ERROR )
			{
				printf( "Failed to hook EndScene!\n" );
			}

			if( DetourAttach( &( PVOID & )m_pOriginalReset, Hook_Reset ) != NO_ERROR )
			{
				printf( "Failed to hook Reset!\n" );
			}

			if( DetourAttach( &( PVOID & )m_pOriginalDrawPrimitive, Hook_DrawPrimitive ) != NO_ERROR )
			{
				printf( "Failed to hook DrawPrimitive!\n" );
			}

			if( DetourAttach( &( PVOID & )m_pOriginalDrawIndexedPrimitive, Hook_DrawIndexedPrimitive ) != NO_ERROR )
			{
				printf( "Failed to hook DrawIndexedPrimitive!\n" );
			}

			if( DetourAttach( &( PVOID & )m_pOriginalDrawPrimitiveUP, Hook_DrawPrimitiveUP ) != NO_ERROR )
			{
				printf( "Failed to hook DrawPrimitiveUP!\n" );
			}

			if( DetourAttach( &( PVOID & )m_pOriginalDrawIndexedPrimitiveUP, Hook_DrawIndexedPrimitiveUP ) != NO_ERROR )
			{
				printf( "Failed to hook DrawIndexedPrimitiveUP!\n" );
			}

			if( DetourAttach( &( PVOID & )m_pOriginalCreateAdditionalSwapChain, Hook_CreateAdditionalSwapChain ) != NO_ERROR )
			{
				printf( "Failed to hook CreateAdditionalSwapChain!\n" );
			}

			if( DetourAttach( &( PVOID & )m_pOriginalSetTransform, Hook_SetTransform ) != NO_ERROR )
			{
				printf( "Failed to hook SetTransform!\n" );
			}

			if( DetourAttach( &( PVOID & )m_pOriginalSetViewport, Hook_SetViewport ) != NO_ERROR )
			{
				printf( "Failed to hook SetViewport!\n" );
			}

			if( DetourAttach( &( PVOID & )m_pOriginalSetRenderTarget, Hook_SetRenderTarget ) != NO_ERROR )
			{
				printf( "Failed to hook SetRenderTarget!\n" );
			}

			HRESULT result = DetourTransactionCommit();
			if( result != NO_ERROR )
			{
				printf( "2 DetourTransactionCommit failed with error code: %d\n", result );
			}

			printf( "Hooks successfully installed on the real device!\n" );
		}
		else
		{
			printf( "Skipping hook for temporary device.\n" );
		}
	}
	else
	{
		printf( "Failed to create device! HRESULT: 0x%08X\n", hr );
	}

	return hr;
}

HRESULT APIENTRY PSUOverlay::Hook_Reset( LPDIRECT3DDEVICE9 pDevice, D3DPRESENT_PARAMETERS *pPresentationParameters )
{
	ImGui_ImplDX9_InvalidateDeviceObjects();
	HRESULT tmpReset = m_pOriginalReset( pDevice, pPresentationParameters );
	ImGui_ImplDX9_CreateDeviceObjects();
	return tmpReset;
}

HRESULT APIENTRY PSUOverlay::Hook_DrawPrimitive( LPDIRECT3DDEVICE9 pDevice, D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex, UINT PrimitiveCount )
{
	return m_pOriginalDrawPrimitive( pDevice, PrimitiveType, StartVertex, PrimitiveCount );
}

HRESULT APIENTRY PSUOverlay::Hook_DrawIndexedPrimitive( LPDIRECT3DDEVICE9 pDevice, D3DPRIMITIVETYPE PrimitiveType, INT BaseVertexIndex, UINT MinIndex, UINT NumVertices, UINT StartIndex, UINT PrimitiveCount )
{
	HRESULT hr = m_pOriginalDrawIndexedPrimitive(
		pDevice, PrimitiveType, BaseVertexIndex, MinIndex, NumVertices, StartIndex, PrimitiveCount
	);

	return hr;
}

HRESULT APIENTRY PSUOverlay::Hook_DrawPrimitiveUP( LPDIRECT3DDEVICE9 pDevice, D3DPRIMITIVETYPE PrimitiveType, UINT PrimitiveCount, const void *pVertexStreamZeroData, UINT VertexStreamZeroStride )
{
	return m_pOriginalDrawPrimitiveUP( pDevice, PrimitiveType, PrimitiveCount, pVertexStreamZeroData, VertexStreamZeroStride );
}

HRESULT APIENTRY PSUOverlay::Hook_DrawIndexedPrimitiveUP( LPDIRECT3DDEVICE9 pDevice, D3DPRIMITIVETYPE Type, UINT MinVertexIndex, UINT NumVertices, UINT PrimitiveCount, const void *pIndexData, D3DFORMAT IndexDataFormat, const void *pVertexStreamZeroData, UINT VertexStreamZeroStride )
{
	return m_pOriginalDrawIndexedPrimitiveUP(
		pDevice,
		Type,
		MinVertexIndex,
		NumVertices,
		PrimitiveCount,
		pIndexData,
		IndexDataFormat,
		pVertexStreamZeroData,
		VertexStreamZeroStride
	);
}

void PrintMatrix( const Matrix4x4 &matrix, const char *name )
{
	printf( "%s Matrix:\n", name );
	for( int row = 0; row < 4; ++row )
	{
		printf( "[ %.2f, %.2f, %.2f, %.2f ]\n",
				matrix.m[ row ][ 0 ], matrix.m[ row ][ 1 ], matrix.m[ row ][ 2 ], matrix.m[ row ][ 3 ] );
	}
	printf( "\n" );
}

HRESULT APIENTRY PSUOverlay::Hook_EndScene( LPDIRECT3DDEVICE9 pDevice )
{
	if( !m_bInitialized )
	{
		m_pDevice = pDevice;
		InitializeImGui( pDevice );
		InitializeControls();
		HookWindow();
	}

	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	{
		if( m_bDisplay )
		{
			RenderMenuBar();
			if( m_bStyleEditor )
				ImGui::ShowStyleEditor( &m_style );
		}

		for( auto &i : m_Controls )
		{
			i.second->Render();
		}
	}
	ImGui::EndFrame();
	ImGui::Render();
	ImGui_ImplDX9_RenderDrawData( ImGui::GetDrawData() );

	return m_pOriginalEndScene( pDevice );
}

HRESULT APIENTRY PSUOverlay::Hook_Present( LPDIRECT3DDEVICE9 pDevice, const RECT *pSourceRect, const RECT *pDestRect, HWND hDestWindowOverride, const RGNDATA *pDirtyRegion )
{
	return m_pOriginalPresent( pDevice, pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion );
}

HRESULT APIENTRY PSUOverlay::Hook_CreateAdditionalSwapChain( LPDIRECT3DDEVICE9 pDevice, D3DPRESENT_PARAMETERS *pPresentationParameters, IDirect3DSwapChain9 **ppSwapChain )
{
	if( !pPresentationParameters || !ppSwapChain )
	{
		return D3DERR_INVALIDCALL;
	}

	HRESULT hr = m_pOriginalCreateAdditionalSwapChain( pDevice, pPresentationParameters, ppSwapChain );

	// The game does some aggressive swap chain creation at times
	// but I'm not sure if we'll need it at all.

	//if( SUCCEEDED( hr ) && ppSwapChain && *ppSwapChain )
	//{
	//	// Assign a unique ID to this swap chain
	//	swapChainCounter = swapChainMap.size();
	//	swapChainMap[ *ppSwapChain ] = swapChainCounter;
	//
	//	// Grab and save the original swap chain functions
	//	void **vTable = *( void *** )*ppSwapChain;
	//	auto originalSwapChainPresent = ( FunctionSwapChainPresent )vTable[ 3 ];
	//	auto originalSwapChainRelease = ( FunctionSwapChainRelease )vTable[ 2 ];
	//	auto originalSwapChainDrawIndexedPrimitiveUP = ( FunctionD3DDrawIndexedPrimitiveUP )vTable[ 53 ];
	//
	//	m_pOriginalSwapChainPresentsMap[ *ppSwapChain ] = originalSwapChainPresent;
	//	m_pOriginalSwapChainReleasesMap[ *ppSwapChain ] = originalSwapChainRelease;
	//	m_pOriginalSwapChainDrawIndexedPrimitiveUPsMap[ *ppSwapChain ] = originalSwapChainDrawIndexedPrimitiveUP;
	//
	//	// Hook the function
	//	DWORD oldProtect;
	//	VirtualProtect( &vTable[ 3 ], sizeof( void * ), PAGE_EXECUTE_READWRITE, &oldProtect );
	//	vTable[ 3 ] = ( void * )Hook_Swapchain_Present;
	//	VirtualProtect( &vTable[ 3 ], sizeof( void * ), oldProtect, &oldProtect );
	//
	//	VirtualProtect( &vTable[ 2 ], sizeof( void * ), PAGE_EXECUTE_READWRITE, &oldProtect );
	//	vTable[ 2 ] = ( void * )Hook_Swapchain_Release;
	//	VirtualProtect( &vTable[ 2 ], sizeof( void * ), oldProtect, &oldProtect );
	//
	//	VirtualProtect( &vTable[ 53 ], sizeof( void * ), PAGE_EXECUTE_READWRITE, &oldProtect );
	//	vTable[ 53 ] = ( void * )Hook_DrawIndexedPrimitiveUP;
	//	VirtualProtect( &vTable[ 53 ], sizeof( void * ), oldProtect, &oldProtect );
	//}

	return hr;
}

HRESULT APIENTRY PSUOverlay::Hook_Swapchain_Present( IDirect3DSwapChain9 *pSwapChain, const RECT *pSourceRect, const RECT *pDestRect, HWND hDestWindowOverride, const RGNDATA *pDirtyRegion, DWORD dwFlags )
{
	int swapChainID = swapChainMap.count( pSwapChain ) ? swapChainMap[ pSwapChain ] : -1;
	
	if( m_pOriginalSwapChainPresentsMap.count( pSwapChain ) )
	{
		auto originalPresent = m_pOriginalSwapChainPresentsMap[ pSwapChain ];
		return originalPresent( pSwapChain, pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion, dwFlags );
	}

	return D3DERR_INVALIDCALL;
}

HRESULT APIENTRY PSUOverlay::Hook_Swapchain_Release( IDirect3DSwapChain9 *pSwapChain )
{
	int swapChainID = swapChainMap.count( pSwapChain ) ? swapChainMap[ pSwapChain ] : -1;

	// Retrieve the original Release function before deleting the entry
	auto originalReleaseIterator = m_pOriginalSwapChainReleasesMap.find( pSwapChain );

	// Remove the swap chain from the maps to avoid memory leaks
	m_pOriginalSwapChainPresentsMap.erase( pSwapChain );
	swapChainMap.erase( pSwapChain );

	// Check if the original Release function exists
	if( originalReleaseIterator != m_pOriginalSwapChainReleasesMap.end() )
	{
		// Store the original function pointer and erase it from the map AFTER retrieval
		auto originalRelease = originalReleaseIterator->second;
		m_pOriginalSwapChainReleasesMap.erase( originalReleaseIterator );

		// Call the original Release function
		return originalRelease( pSwapChain );
	}

	return 0;
}

HRESULT APIENTRY PSUOverlay::Hook_SetTransform( LPDIRECT3DDEVICE9 pDevice, D3DTRANSFORMSTATETYPE State, const D3DMATRIX *pMatrix )
{
	return m_pOriginalSetTransform( pDevice, State, pMatrix );
}

HRESULT APIENTRY PSUOverlay::Hook_SetViewport( LPDIRECT3DDEVICE9 pDevice, CONST D3DVIEWPORT9 *pViewport )
{
	// Call the original function with unmodified data if conditions are not met
	return m_pOriginalSetViewport( pDevice, pViewport );
}

HRESULT APIENTRY PSUOverlay::Hook_SetRenderTarget( LPDIRECT3DDEVICE9 pDevice, DWORD RenderTargetIndex, IDirect3DSurface9 *pRenderTarget )
{
	return m_pOriginalSetRenderTarget( pDevice, RenderTargetIndex, pRenderTarget );
}

HRESULT APIENTRY PSUOverlay::Hook_SetScissorRect( LPDIRECT3DDEVICE9 pDevice, CONST RECT *pRect )
{
	// Call the original function safely
	return m_pOriginalSetScissorRect( pDevice, pRect );
}

void PSUOverlay::InitializeImGui( const LPDIRECT3DDEVICE9 pDevice )
{
	ImGui::CreateContext();
	ImGuiIO &io = ImGui::GetIO();
	auto &config = Config::Get();

	io.ConfigFlags = ImGuiConfigFlags_NoMouseCursorChange;
	io.IniFilename = nullptr;
	io.Fonts->AddFontDefault();

	ImGui::StyleColorsDark( &m_style );

	auto &style = ImGui::GetStyle();

	printf( "ImGui Style:\n" );
	printf( "Alpha: %f\n", config.m_style.Alpha );
	printf( "WindowPadding: %f %f\n", config.m_style.WindowPadding.x, config.m_style.WindowPadding.y );
	printf( "FramePadding: %f %f\n", config.m_style.FramePadding.x, config.m_style.FramePadding.y );
	printf( "ItemSpacing: %f %f\n", config.m_style.ItemSpacing.x, config.m_style.ItemSpacing.y );
	printf( "WindowRounding: %f\n", config.m_style.WindowRounding );

	style.Alpha = config.m_style.Alpha;
	style.WindowPadding = config.m_style.WindowPadding;
	style.FramePadding = config.m_style.FramePadding;
	style.ItemSpacing = config.m_style.ItemSpacing;

	style.WindowRounding = config.m_style.WindowRounding;
	style.FrameRounding = config.m_style.FrameRounding;
	style.GrabRounding = config.m_style.GrabRounding;

	for( int i = 0; i < ImGuiCol_COUNT; ++i )
	{
		const ImVec4 &col = config.m_style.Colors[ i ];
		style.Colors[ i ] = col;
	}

	//style.WindowRounding = 0.0f;
	//style.FrameRounding = 0.0f;
	//style.Colors[ ImGuiCol_WindowBg ] = ImVec4( 0.1f, 0.1f, 0.1f, 0.2f );
	//style.Colors[ ImGuiCol_ChildBg ] = ImVec4( 0.1f, 0.1f, 0.1f, 0.2f );
	//style.Colors[ ImGuiCol_PopupBg ] = ImVec4( 0.1f, 0.1f, 0.1f, 0.2f );

	ImGui_ImplWin32_Init( Global::hWnd );
	ImGui_ImplDX9_Init( pDevice );

	m_bInitialized = true;
}
