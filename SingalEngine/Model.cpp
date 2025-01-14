#include "Model.h"

bool Model::Initialize(ComPtr<ID3D12Device>& device, ComPtr<ID3D12GraphicsCommandList>& cmdList)
{
	return true;
}

Model::Model() {
	
}

Model::~Model() {
   
   
   
   
}

void Model::Render(const ComPtr<ID3D12GraphicsCommandList>& commandList)
{
	commandList->IASetVertexBuffers(0, 1, &this->VertexBufferView());
	commandList->IASetIndexBuffer(&this->IndexBufferView());
	commandList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->DrawIndexedInstanced(IndexCount, 1, 0, 0, 0);
}
