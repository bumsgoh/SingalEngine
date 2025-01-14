#pragma once

#include <array>
#include <wrl/client.h> 
#include <d3d12.h>
#include "d3dx12.h"
#include <vector>
#include <memory>
#include "ShaderCommon.h"
#include "UploadBuffer.h"
using namespace Microsoft::WRL;

class Scene;

class Renderer
{
public:
	ComPtr<ID3D12GraphicsCommandList> commandList;
	ComPtr<ID3D12Device> device;
	ComPtr<ID3DBlob> m_vsByteCode = nullptr;
	ComPtr<ID3DBlob> m_psByteCode = nullptr;
	ComPtr<ID3D12RootSignature> m_rootSignature = nullptr;
	ComPtr<ID3D12DescriptorHeap> m_cbvHeap = nullptr;
	std::unique_ptr<UploadBuffer<ConstantBuffer>> mObjectCB = nullptr;

	DXGI_FORMAT m_backBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

	std::vector<D3D12_INPUT_ELEMENT_DESC> m_inputLayout;

	ComPtr<ID3D12PipelineState> m_pso = nullptr;

	Renderer(const ComPtr<ID3D12Device>& device,
			 const ComPtr<ID3D12GraphicsCommandList>& commandList);

	void Initialize(Scene& scene);
	void BuildConstantBuffers();
	void BuildDescriptorHeaps();
	void BuildDefaultPSO();
	void BuildShadersAndInputLayout();
	void BuildRootSignature();
	void Render(Scene& scene);
};

