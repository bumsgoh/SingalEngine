#pragma once

#include "ShaderCommon.h"
#include <array>
#include "GlobalFunctions.h"
#include <memory>
#include "Model.h"


class Cube :
    public Model
{
public:
	Cube();
	virtual bool Initialize(
		ComPtr<ID3D12Device>& device,
		ComPtr<ID3D12GraphicsCommandList>& cmdList,
		UINT numberOfItems) override;

	std::array<Vertex, 8> vertices =
	{
		Vertex({ XMFLOAT3(-1.0f, -1.0f, 0.0f) }),
		Vertex({ XMFLOAT3(-1.0f, +1.0f, 0.0f) }),
		Vertex({ XMFLOAT3(+1.0f, +1.0f, 0.0f) }),
		Vertex({ XMFLOAT3(+1.0f, -1.0f, 0.0f) }),
		Vertex({ XMFLOAT3(-1.0f, -1.0f, +1.0f) }),
		Vertex({ XMFLOAT3(-1.0f, +1.0f, +1.0f) }),
		Vertex({ XMFLOAT3(+1.0f, +1.0f, +1.0f) }),
		Vertex({ XMFLOAT3(+1.0f, -1.0f, +1.0f) })
	};

	std::array<std::uint16_t, 36> indices =
	{
		// front face
		0, 1, 2,
		0, 2, 3,

		// back face
		4, 6, 5,
		4, 7, 6,

		// left face
		4, 5, 1,
		4, 1, 0,

		// right face
		3, 2, 6,
		3, 6, 7,

		// top face
		1, 5, 6,
		1, 6, 2,

		// bottom face
		4, 0, 3,
		4, 3, 7
	};
};

