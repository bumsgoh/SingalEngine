#pragma once

#include "CommonMath.h"
using namespace DirectX;

class Camera
{
public:
    Camera() { UpdateViewDirection(); }

    XMFLOAT4X4 GetViewRow();
    XMFLOAT4X4 GetProjRow();
    XMFLOAT3 GetEyePos();

    void UpdateViewDirection();
    void UpdateKeyboard(const float dt, bool const keyPressed[256]);
    void UpdateMouse(float mouseNdcX, float mouseNdcY);
    void MoveForward(float dt);
    void MoveRight(float dt);
    void MoveUp(float dt);
    void SetAspectRatio(float aspect);

public:
    bool m_useFirstPersonView = false;

private:
    XMFLOAT3 m_position = XMFLOAT3(5, 0, 0); //XMFLOAT3(0.275514f, 0.461257f, 0.0855238f);
    XMFLOAT3 m_viewDirection = XMFLOAT3(0.0f, 0.0f, 1.0f);
    XMFLOAT3 m_upDirection = XMFLOAT3(0.0f, 1.0f, 0.0f);
    XMFLOAT3 m_rightDirection = XMFLOAT3(1.0f, 0.0f, 0.0f);

    // roll, pitch, yaw
    float m_yaw = -0.019635f, m_pitch = -0.120477f;

    float m_speed = 3.0f; // 움직이는 속도

    // 프로젝션 옵션도 카메라 클래스로 이동
    float m_projFovAngleY = 90.0f;
    float m_nearZ = 0.01f;
    float m_farZ = 100.0f;
    float m_aspect = 16.0f / 9.0f;
    bool m_usePerspectiveProjection = true;
};

