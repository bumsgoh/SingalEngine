#pragma once

#include <memory>
#include "d3dx12.h"
#include <d3d12.h>
#include <wrl/client.h> 

using Microsoft::WRL::ComPtr;
class Cube;

class Scene
{
public:
	Scene();
	void Render(ComPtr<ID3D12GraphicsCommandList>& cmdList);
	bool Initialize(ComPtr<ID3D12Device>& device, ComPtr<ID3D12GraphicsCommandList>& cmdList);

	std::shared_ptr<Cube> cube;

};