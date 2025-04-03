#pragma once

#include <d3d9.h>
#include <d3dx9.h>
#include "detours.h"

#include "imgui.h"
#include "backends/imgui_impl_dx9.h"
#include "backends/imgui_impl_win32.h"

#include <vector>
#include <memory>
#include <map>

#include "Interface/IFWinCtrl.h"

enum class WINDOW_CONTROL_ID {
	WINDOW_CONFIG_DISPLAY = 0,
	WINDOW_LOOT_DISPLAY = 1,
	WINDOW_LOG_DISPLAY = 2,
	WINDOW_CAMERA_DISPLAY = 3,
};

class PSUOverlay
{
private:
	typedef HRESULT( APIENTRY *FunctionD3DCreateDevice )(
		IDirect3D9 *pD3D,
		UINT Adapter,
		D3DDEVTYPE DeviceType,
		HWND hFocusWindow,
		DWORD BehaviorFlags,
		D3DPRESENT_PARAMETERS *pPresentationParameters,
		IDirect3DDevice9 **ppReturnedDeviceInterface
	);

	typedef HRESULT( APIENTRY *FunctionD3DEndScene )( LPDIRECT3DDEVICE9 pDevice );
	typedef HRESULT( APIENTRY *FunctionD3DDrawPrimitive )( LPDIRECT3DDEVICE9 pDevice, D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex, UINT PrimitiveCount );
	typedef HRESULT( APIENTRY *FunctionD3DDrawPrimitiveUP )( LPDIRECT3DDEVICE9 pDevice, D3DPRIMITIVETYPE PrimitiveType, UINT PrimitiveCount, CONST void *pVertexStreamZeroData, UINT VertexStreamZeroStride );
	typedef HRESULT( APIENTRY *FunctionD3DDrawIndexedPrimitive )( LPDIRECT3DDEVICE9 pDevice, D3DPRIMITIVETYPE Type, INT BaseVertexIndex, UINT MinIndex, UINT NumVertices, UINT StartIndex, UINT PrimitiveCount );
	typedef HRESULT( APIENTRY *FunctionD3DDrawIndexedPrimitiveUP )( LPDIRECT3DDEVICE9 pDevice, D3DPRIMITIVETYPE Type, UINT MinVertexIndex, UINT NumVertices, UINT PrimitiveCount, CONST void *pIndexData, D3DFORMAT IndexDataFormat, CONST void *pVertexStreamZeroData, UINT VertexStreamZeroStride );
	typedef HRESULT( APIENTRY *FunctionD3DReset )( LPDIRECT3DDEVICE9 pDevice, D3DPRESENT_PARAMETERS *pPresentationParameters );
	typedef HRESULT( APIENTRY *FunctionD3DPresent )( LPDIRECT3DDEVICE9 pDevice, CONST RECT *pSourceRect, CONST RECT *pDestRect, HWND hDestWindowOverride, CONST RGNDATA *pDirtyRegion );
	typedef HRESULT( APIENTRY *FunctionCreateAdditionalSwapChain )( LPDIRECT3DDEVICE9 pDevice, D3DPRESENT_PARAMETERS *pPresentationParameters, IDirect3DSwapChain9 **ppSwapChain );
	typedef HRESULT( APIENTRY *FunctionSwapChainRelease )( IDirect3DSwapChain9 *pSwapChain );
	typedef HRESULT( APIENTRY *FunctionSwapChainPresent )( IDirect3DSwapChain9 *pSwapChain, CONST RECT *pSourceRect, CONST RECT *pDestRect, HWND hDestWindowOverride, CONST RGNDATA *pDirtyRegion, DWORD dwFlags );

	typedef HRESULT( APIENTRY *FunctionSetTransform )( LPDIRECT3DDEVICE9 pDevice, D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX *pMatrix );
	typedef HRESULT( APIENTRY *FunctionSetViewport )( LPDIRECT3DDEVICE9 pDevice, CONST D3DVIEWPORT9 *pViewport );
	typedef HRESULT( APIENTRY *FunctionSetRenderTarget )( LPDIRECT3DDEVICE9 pDevice, DWORD RenderTargetIndex, IDirect3DSurface9 *pRenderTarget );
	typedef HRESULT( APIENTRY *FunctionSetScissorRect )( LPDIRECT3DDEVICE9 pDevice, CONST RECT *pRect );

	static PSUOverlay *instance;

	static inline bool m_bInitialized;
	static inline bool m_bDisplay;
	static inline bool m_bStyleEditor;

	static inline int m_iWidth;
	static inline int m_iHeight;

	static inline IDirect3DDevice9 *m_pDevice;
	static inline HWND m_hWnd;
	static inline WNDPROC m_pWndProc;

	static inline FunctionD3DCreateDevice m_pOriginalCreateDevice = nullptr;
	static inline FunctionD3DEndScene m_pOriginalEndScene;
	static inline FunctionD3DDrawPrimitive m_pOriginalDrawPrimitive;
	static inline FunctionD3DDrawPrimitiveUP m_pOriginalDrawPrimitiveUP;
	static inline FunctionD3DDrawIndexedPrimitive m_pOriginalDrawIndexedPrimitive;
	static inline FunctionD3DDrawIndexedPrimitiveUP m_pOriginalDrawIndexedPrimitiveUP;
	static inline FunctionD3DReset m_pOriginalReset;
	static inline FunctionD3DPresent m_pOriginalPresent;
	static inline FunctionCreateAdditionalSwapChain m_pOriginalCreateAdditionalSwapChain;

	static inline FunctionSetTransform m_pOriginalSetTransform;
	static inline FunctionSetViewport m_pOriginalSetViewport;
	static inline FunctionSetRenderTarget m_pOriginalSetRenderTarget;
	static inline FunctionSetScissorRect m_pOriginalSetScissorRect;

	static inline std::map<IDirect3DSwapChain9 *, FunctionSwapChainPresent > m_pOriginalSwapChainPresentsMap;
	static inline std::map<IDirect3DSwapChain9 *, FunctionSwapChainRelease > m_pOriginalSwapChainReleasesMap;
	static inline std::map< IDirect3DSwapChain9 *, FunctionD3DDrawIndexedPrimitiveUP > m_pOriginalSwapChainDrawIndexedPrimitiveUPsMap;

	static inline std::map<IDirect3DSwapChain9 *, int> m_swapChainMap;
	static inline int m_swapChainCounter;

	static inline ImGuiStyle m_style;
	static inline std::map< WINDOW_CONTROL_ID, WinCtrlPtr > m_Controls;

public:
	static PSUOverlay &Get()
	{
		static PSUOverlay instance;
		return instance;
	}

	PSUOverlay( const PSUOverlay & ) = delete;
	PSUOverlay &operator=( const PSUOverlay & ) = delete;

	PSUOverlay();
	~PSUOverlay();

	static void RenderMenuBar();

	static void Initialize( IDirect3D9 *pD3D );
	static void AddControl( WINDOW_CONTROL_ID ID, WinCtrlPtr pCtrl );

	template <typename T>
	static std::shared_ptr<T> GetControl( WINDOW_CONTROL_ID ID )
	{
		auto it = m_Controls.find( ID );
		if( it == m_Controls.end() )
		{
			printf( "GetControl: ID not found!\n" );
			return nullptr;
		}

		if( !it->second )
		{
			printf( "GetControl: Found ID, but pointer is null!\n" );
			return nullptr;
		}

		auto castedPtr = std::dynamic_pointer_cast< T >( it->second );
		if( !castedPtr )
		{
			printf( "GetControl: dynamic_pointer_cast failed!\n" );
			return nullptr;
		}

		return castedPtr;
	}

private:
	static void ReleaseDirectX();
	static void InitializeControls();
	static void HookWindow();
	static void ReleaseWindow();

	static HRESULT APIENTRY Hook_CreateDevice( IDirect3D9 *pD3D, UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS *pPresentationParameters, IDirect3DDevice9 **ppReturnedDeviceInterface );
	static HRESULT APIENTRY Hook_Reset( LPDIRECT3DDEVICE9 pDevice, D3DPRESENT_PARAMETERS *pPresentationParameters );
	static HRESULT APIENTRY Hook_DrawPrimitive( LPDIRECT3DDEVICE9 pDevice, D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex, UINT PrimitiveCount );
	static HRESULT APIENTRY Hook_DrawIndexedPrimitive( LPDIRECT3DDEVICE9 pDevice, D3DPRIMITIVETYPE PrimitiveType, INT BaseVertexIndex, UINT MinIndex, UINT NumVertices, UINT StartIndex, UINT PrimitiveCount );
	static HRESULT APIENTRY Hook_DrawPrimitiveUP( LPDIRECT3DDEVICE9 pDevice, D3DPRIMITIVETYPE PrimitiveType, UINT PrimitiveCount, CONST void *pVertexStreamZeroData, UINT VertexStreamZeroStride );
	static HRESULT APIENTRY Hook_DrawIndexedPrimitiveUP( LPDIRECT3DDEVICE9 pDevice, D3DPRIMITIVETYPE PrimitiveType, UINT MinVertexIndex, UINT NumVertices, UINT PrimitiveCount, CONST void *pIndexData, D3DFORMAT IndexDataFormat, CONST void *pVertexStreamZeroData, UINT VertexStreamZeroStride );
	static HRESULT APIENTRY Hook_EndScene( LPDIRECT3DDEVICE9 D3D9Device );
	static HRESULT APIENTRY Hook_Present( LPDIRECT3DDEVICE9 pDevice, CONST RECT *pSourceRect, CONST RECT *pDestRect, HWND hDestWindowOverride, CONST RGNDATA *pDirtyRegion );
	static HRESULT APIENTRY Hook_CreateAdditionalSwapChain( LPDIRECT3DDEVICE9 pDevice, D3DPRESENT_PARAMETERS *pPresentationParameters, IDirect3DSwapChain9 **ppSwapChain );
	static HRESULT APIENTRY Hook_Swapchain_Present( IDirect3DSwapChain9 *pSwapChain, CONST RECT *pSourceRect, CONST RECT *pDestRect, HWND hDestWindowOverride, CONST RGNDATA *pDirtyRegion, DWORD dwFlags );
	static HRESULT APIENTRY Hook_Swapchain_Release( IDirect3DSwapChain9 *pSwapChain );
	static HRESULT APIENTRY Hook_SetTransform( LPDIRECT3DDEVICE9 pDevice, D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX *pMatrix );
	static HRESULT APIENTRY Hook_SetViewport( LPDIRECT3DDEVICE9 pDevice, const D3DVIEWPORT9 *pViewport );
	static HRESULT APIENTRY Hook_SetRenderTarget( LPDIRECT3DDEVICE9 pDevice, DWORD RenderTargetIndex, IDirect3DSurface9 *pRenderTarget );
	static HRESULT APIENTRY Hook_SetScissorRect( LPDIRECT3DDEVICE9 pDevice, const RECT *pRect );

	static void InitializeImGui( const LPDIRECT3DDEVICE9 pDevice );
	
	static LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
};