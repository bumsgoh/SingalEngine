#pragma once
#include <DirectXMath.h>
#include "CommonMath.h"

using namespace DirectX;

struct Vertex
{
    XMFLOAT3 Position;
};

struct ConstantBuffer
{
    XMFLOAT4X4 modelMatrix = Identity4x4();
    XMFLOAT4X4 viewMatrix = Identity4x4();
    XMFLOAT4X4 projectionMatrix = Identity4x4();
};