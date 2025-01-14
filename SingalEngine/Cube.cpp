#include "Cube.h"
#include "Utils.h"
Cube::Cube()
{
	IndexCount = indices.size();
	const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	ThrowIfFailed(D3DCreateBlob(vbByteSize,
		&this->m_vertexBufferCPU));
	CopyMemory(this->m_vertexBufferCPU->GetBufferPointer(),
		vertices.data(),
		vbByteSize);

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &this->m_indexBufferCPU));
	CopyMemory(this->m_indexBufferCPU->GetBufferPointer(),
		indices.data(),
		ibByteSize);
		
	this->VertexByteStride = sizeof(Vertex);
	this->VertexBufferByteSize = vbByteSize;
	this->IndexFormat = DXGI_FORMAT_R16_UINT;
	this->IndexBufferByteSize = ibByteSize;
}

bool Cube::Initialize(
	ComPtr<ID3D12Device>& device,
	ComPtr<ID3D12GraphicsCommandList>& cmdList)
{
	m_vertexBufferGPU = CreateDefaultBuffer(
		device.Get(),
		cmdList.Get(),
		vertices.data(),
		VertexBufferByteSize,
		m_vertexBufferUploader);

	m_indexBufferGPU = CreateDefaultBuffer(
		device.Get(),
		cmdList.Get(),
		indices.data(),
		IndexBufferByteSize,
		m_indexBufferUploader);

	return true;
}

