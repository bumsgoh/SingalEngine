#pragma once

#include "Transform.h"
#include <D3Dcompiler.h>
#include <dxgi1_4.h>
#include <d3d12.h>
#include "d3dx12.h"
#include <vector>
#include <wrl/client.h> 
#include "UploadBuffer.h"
#include "ShaderCommon.h"
#include <memory>

using namespace Microsoft::WRL;
class Model
{

public:

	virtual bool Initialize(
		ComPtr<ID3D12Device>& device,
		ComPtr<ID3D12GraphicsCommandList>& cmdList,
		UINT numberOfItems);

	UINT VertexByteStride = 0;
	UINT VertexBufferByteSize = 0;
	DXGI_FORMAT IndexFormat = DXGI_FORMAT_R16_UINT;
	UINT IndexBufferByteSize = 0;
	UINT IndexCount = 0;
	UINT NumberOfItems = 0;
	UINT ConstantBufferIndex = -1;

	ComPtr<ID3D12CommandAllocator> CommandListAlloc;
	std::unique_ptr<UploadBuffer<ConstantBuffer>> CBuffer = nullptr;
	UINT64 Fence = 0;

public:
	ComPtr<ID3DBlob> m_vertexBufferCPU = nullptr;
	ComPtr<ID3DBlob> m_indexBufferCPU = nullptr;

	ComPtr<ID3D12Resource> m_vertexBufferGPU = nullptr;
	ComPtr<ID3D12Resource> m_indexBufferGPU = nullptr;

	ComPtr<ID3D12Resource> m_vertexBufferUploader = nullptr;
	ComPtr<ID3D12Resource> m_indexBufferUploader = nullptr;

	D3D12_VERTEX_BUFFER_VIEW VertexBufferView()const
	{
		D3D12_VERTEX_BUFFER_VIEW vbv;
		vbv.BufferLocation = m_vertexBufferGPU->GetGPUVirtualAddress();
		vbv.StrideInBytes = VertexByteStride;
		vbv.SizeInBytes = VertexBufferByteSize;

		return vbv;
	}

	D3D12_INDEX_BUFFER_VIEW IndexBufferView()const
	{
		D3D12_INDEX_BUFFER_VIEW ibv;
		ibv.BufferLocation = m_indexBufferGPU->GetGPUVirtualAddress();
		ibv.Format = IndexFormat;
		ibv.SizeInBytes = IndexBufferByteSize;

		return ibv;
	}
public:
	Model();
	~Model();
	Transform transform = Transform();
	void Render(const ComPtr<ID3D12GraphicsCommandList>& commandList);
};

