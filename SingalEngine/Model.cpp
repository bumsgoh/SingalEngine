#include "Model.h"

bool Model::Initialize(ComPtr<ID3D12Device>& device, ComPtr<ID3D12GraphicsCommandList>& cmdList, UINT numberOfItems)
{
	ThrowIfFailed(device->CreateCommandAllocator(
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(this->CommandListAlloc.GetAddressOf())));

	this->NumberOfItems = numberOfItems;
	this->CBuffer = std::make_unique<UploadBuffer<ConstantBuffer>>(device.Get(), numberOfItems, true);
	return true;
}

Model::Model() {
	
}

Model::~Model() {
   
   
   
   
}

void Model::Render(const ComPtr<ID3D12GraphicsCommandList>& commandList)
{
	UINT objCBByteSize = CalcConstantBufferByteSize(sizeof(ConstantBuffer));
	commandList->IASetVertexBuffers(0, 1, &this->VertexBufferView());
	commandList->IASetIndexBuffer(&this->IndexBufferView());
	commandList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = this->CBuffer->Resource()->GetGPUVirtualAddress() + this->ConstantBufferIndex * objCBByteSize;

	commandList->SetGraphicsRootConstantBufferView(0, objCBAddress);
	commandList->DrawIndexedInstanced(IndexCount, 1, 0, 0, 0);
}
