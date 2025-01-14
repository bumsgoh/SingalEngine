#include "Scene.h"
#include "Cube.h"

Scene::Scene()
{
	cube = std::make_shared<Cube>();
	
}

void Scene::Render(ComPtr<ID3D12GraphicsCommandList>& cmdList)
{
	cube->Render(cmdList);
}


bool Scene::Initialize(
	ComPtr<ID3D12Device>& device,
	ComPtr<ID3D12GraphicsCommandList>& cmdList)
{

	cube->Initialize(device, cmdList);
	return true;
}
