#pragma once

#include <DirectXMath.h>

using namespace DirectX;

static XMFLOAT4X4 Identity4x4()
{
    static XMFLOAT4X4 I(
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f);

    return I;
}

static XMFLOAT4X4 HalfIdentity4x4()
{
    static XMFLOAT4X4 I(
        0.5f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.5f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.5f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f);

    return I;
}

static XMFLOAT4X4 Scale4x4(float x, float y, float z, float weight = 1.0f)
{
    static XMFLOAT4X4 I(
        x * weight, 0.0f, 0.0f, 0.0f,
        0.0f, y * weight, 0.0f, 0.0f,
        0.0f, 0.0f, z * weight, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f);

    return I;
}
