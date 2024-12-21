#pragma once

#include "resource.h"
#include "GlobalFunctions.h"
//vcpkg install imgui[core,dx12-binding,win32-binding]:x64-windows
#include <imgui.h>
#include <imgui_impl_dx12.h>
#include <imgui_impl_win32.h>

#include <D3Dcompiler.h>
#include <dxgi1_4.h>
#include <d3d12.h>
#include "d3dx12.h"
#include <d3dcompiler.h>
#include <DirectXMath.h>

#include <iostream>
#include <vector>
#include <directxtk/SimpleMath.h>
#include <windows.h>
#include <wrl/client.h> 

// Link necessary d3d12 libraries.
#pragma comment(lib,"d3dcompiler.lib")
#pragma comment(lib, "D3D12.lib")
#pragma comment(lib, "dxgi.lib")

using Microsoft::WRL::ComPtr;
using std::shared_ptr;
using std::vector;
using std::wstring;

class SingalEngine {

public:
	SingalEngine();
	virtual ~SingalEngine();
	virtual LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

	bool Initialize();
	bool InitWindow(HINSTANCE hInstance, int nCmdShow);


	bool InitDirect3D();
	void InitCommandList();
	void InitDSHeaps();
	void InitSwapChain();
	int Run();

	float GetScreenRatio() const;
	void SetViewport();

	void UpdateEnginFrame(float dt);
	void UpdateGUI();

	void Render();

	//Buffer
	ID3D12Resource* CurrentBackBuffer() const;
	D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferView() const;
	D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView() const;
	void FlushCommandQueue();

public:
	int m_screenWidth;
	int m_screenHeight;

	HWND m_windowHandle = nullptr;

	ComPtr<IDXGIFactory> m_dxgiFactory;
	ComPtr<ID3D12Device> m_device;
	ComPtr<ID3D12Fence> m_fence;
	UINT64 m_currentFenceValue = 0;

	ComPtr<ID3D12CommandQueue> m_commandQueue;
	ComPtr<ID3D12CommandAllocator> m_commandListAllocator;
	ComPtr<ID3D12GraphicsCommandList> m_commandList;
	ComPtr<IDXGISwapChain> m_swapChain;

	//heaps
	ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
	ComPtr<ID3D12DescriptorHeap> m_dsvHeap;
	UINT m_rtvDescriptorSize = 0;
	UINT m_dsvDescriptorSize = 0;

	//Render
	DXGI_FORMAT m_backBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	DXGI_FORMAT m_depthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	ComPtr<ID3D12Resource> m_swapChainBuffer[2];
	ComPtr<ID3D12Resource> m_depthStencilBuffer;
	int m_currBackBufferIndex = 0;

	D3D12_VIEWPORT m_screenViewport;
};