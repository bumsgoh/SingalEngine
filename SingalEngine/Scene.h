#pragma once

#include <memory>
#include "d3dx12.h"
#include <d3d12.h>
#include <wrl/client.h> 
#include "UploadBuffer.h"
#include <vector>
#include "Camera.h"

using Microsoft::WRL::ComPtr;
class Model;
class Camera;
struct ConstantBuffer;

class Scene
{
public:
	Scene(float screenAspect);
	void Render(ComPtr<ID3D12GraphicsCommandList>& cmdList);
	bool Initialize(ComPtr<ID3D12Device>& device, 
					ComPtr<ID3D12GraphicsCommandList>& cmdList);
	void Update(float DeltaTime);
	void UpdateConstantBuffers(float DeltaTime, Camera camera);

	float totalTime = 0.0f;
	float screenAspect;
	std::vector<std::shared_ptr<Model>> modelList;
	Model* m_currentResource = nullptr;
	UINT m_currentConstantBufferIndex = 0;

};