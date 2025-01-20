#pragma once

#include <array>
#include <wrl/client.h> 
#include <d3d12.h>
#include "d3dx12.h"
#include <dxgi1_4.h>
#include <vector>
#include <windows.h>
#include <memory>
#include "ShaderCommon.h"
#include "UploadBuffer.h"
#include <DirectXMath.h>

using namespace Microsoft::WRL;
using std::wstring;

class Scene;
class Model;

class Renderer
{
public:
	static const int SwapChainBufferCount = 2;

	ComPtr<ID3D12Fence> m_fence;
	ComPtr<IDXGIFactory> m_dxgiFactory;
	ComPtr<ID3D12GraphicsCommandList> m_commandList;
	ComPtr<ID3D12CommandQueue> m_commandQueue;
	ComPtr<ID3D12CommandAllocator> m_commandListAllocator;

	ComPtr<ID3D12Device> m_device;
	UINT64 m_currentFenceValue = 0;
	int m_screenWidth;
	int m_screenHeight;

	D3D12_VIEWPORT m_screenViewport = D3D12_VIEWPORT();
	D3D12_RECT m_scissorRect;

	ComPtr<IDXGISwapChain> m_swapChain;

	//heaps
	ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
	ComPtr<ID3D12DescriptorHeap> m_dsvHeap;
	UINT m_rtvDescriptorSize = 0;
	UINT m_dsvDescriptorSize = 0;

	ComPtr<ID3DBlob> m_vsByteCode = nullptr;
	ComPtr<ID3DBlob> m_psByteCode = nullptr;
	ComPtr<ID3D12RootSignature> m_rootSignature = nullptr;
	ComPtr<ID3D12DescriptorHeap> m_cbvHeap = nullptr;
	std::unique_ptr<UploadBuffer<ConstantBuffer>> gpuConstantBuffer = nullptr;

	DXGI_FORMAT m_backBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	DXGI_FORMAT m_depthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	ComPtr<ID3D12Resource> m_swapChainBuffer[2];
	ComPtr<ID3D12Resource> m_depthStencilBuffer;
	int m_currBackBufferIndex = 0;

	bool m_4xMsaaState = false;
	UINT m_4xMsaaQuality = 0;

	std::vector<D3D12_INPUT_ELEMENT_DESC> m_inputLayout;

	ComPtr<ID3D12PipelineState> m_pso = nullptr;

	Renderer(int screenWidth,
			 int screenHeight);

	float GetScreenRatio() const;
	void SetViewport();
	void DidResizeWindow();

	void Initialize(Scene& scene);
	bool InitDirect3D(HWND windowHandle, Scene& scene);
	void InitCommandList();
	void InitDSHeaps();
	void InitSwapChain(HWND windowHandle,
					   ComPtr<IDXGIFactory>& dxgiFactory,
					   ComPtr<ID3D12CommandQueue>& commandQueue);
	void BuildConstantBuffers();
	void BuildDescriptorHeaps();
	void BuildDefaultPSO();
	void BuildShadersAndInputLayout();
	void BuildRootSignature();

	void ResetSwapChain();

	void Render(Scene& scene, float DeltaTime);
	void Update(Scene& scene, float DeltaTime);
	void PresentCurrentBuffer();

	void FlushCommandQueue();

	ID3D12Resource* CurrentBackBuffer() const;
	D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferView() const;
	D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView() const;
};

