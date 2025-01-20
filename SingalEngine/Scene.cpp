#include "Scene.h"
#include "Model.h"
#include "Cube.h"
#include "ShaderCommon.h"

Scene::Scene(float screenAspect)
{
	this->screenAspect = screenAspect;
	auto cube = std::make_shared<Cube>();
	cube->ConstantBufferIndex = 0;
	modelList.push_back(cube);

	//ConstantBufferIndex ´Â °´Ã¼º°·Î ´Ã¾î³ª¾ßÇÔ
}

void Scene::Render(ComPtr<ID3D12GraphicsCommandList>& cmdList)
{
	for (auto& m : modelList) {
		m->Render(cmdList);
	}
	
}

bool Scene::Initialize(
	ComPtr<ID3D12Device>& device,
	ComPtr<ID3D12GraphicsCommandList>& cmdList)
{
	for (auto& m : modelList) {
		m->Initialize(device, cmdList, modelList.size());
	}
	return true;
}

void Scene::Update(float DeltaTime)
{
	m_currentConstantBufferIndex = (m_currentConstantBufferIndex + 1) % modelList.size();
	m_currentResource = modelList[m_currentConstantBufferIndex].get();

	for (auto& m : modelList) {
		
	}
}

void Scene::UpdateConstantBuffers(float DeltaTime)
{
	totalTime += DeltaTime;
	XMFLOAT4X4 rotation;
	XMStoreFloat4x4(&rotation, XMMatrixRotationY(sinf(totalTime)));
	XMFLOAT4X4 translation;
	XMStoreFloat4x4(&translation, XMMatrixTranslation(0, 0, 0.5));
	auto currentConstantBuffer = m_currentResource->CBuffer.get();
	m_currentResource->transform.Scale = Scale4x4(screenAspect, 1.0f, 1.0f, 0.1f);
	m_currentResource->transform.Rotation = rotation;
	m_currentResource->transform.Translation = translation;
	ConstantBuffer constantBuffer;
	XMMATRIX model = m_currentResource->transform.ModelMatrix();
	
	XMStoreFloat4x4(&constantBuffer.modelMatrix, XMMatrixTranspose(model));
	currentConstantBuffer->CopyData(m_currentResource->ConstantBufferIndex, constantBuffer);
}
