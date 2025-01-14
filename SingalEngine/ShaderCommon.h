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
    XMFLOAT4X4 worldMatrix = Identity4x4();
};