#pragma once
#include "ShaderCommon.h"

class Transform
{

public:
    XMMATRIX ModelMatrix()
    {
        XMMATRIX scale = XMLoadFloat4x4(&Scale);
        XMMATRIX rotation = XMLoadFloat4x4(&Rotation);
        XMMATRIX translation = XMLoadFloat4x4(&Translation);

        return XMMatrixMultiply(XMMatrixMultiply(scale, rotation), translation);
    }

	XMFLOAT4X4 Scale = Identity4x4();
	XMFLOAT4X4 Rotation = Identity4x4();
	XMFLOAT4X4 Translation = Identity4x4();
};