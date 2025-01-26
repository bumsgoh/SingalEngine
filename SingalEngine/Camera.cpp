#include "Camera.h"

XMFLOAT4X4 Camera::GetViewRow()
{
    //카메라는 반대로 이동하면된다.
    XMMATRIX view = XMMatrixMultiply(XMMatrixTranslation(-m_position.x, -m_position.y, -m_position.z), 
                    XMMatrixMultiply(XMMatrixRotationY(-m_yaw),
                                     XMMatrixRotationY(-m_yaw)));
    XMFLOAT4X4 viewDestination;
    XMStoreFloat4x4(&viewDestination, view);
    return viewDestination;
}

XMFLOAT4X4 Camera::GetProjRow()
{
    return XMFLOAT4X4();
}

XMFLOAT3 Camera::GetEyePos()
{
    return XMFLOAT3(m_position.x, m_position.y, m_position.z);
}

void Camera::UpdateViewDirection()
{
}

void Camera::UpdateKeyboard(const float dt, bool const keyPressed[256])
{
    if (keyPressed['W'])
        MoveForward(dt);
    if (keyPressed['S'])
        MoveForward(-dt);
    if (keyPressed['D'])
        MoveRight(dt);
    if (keyPressed['A'])
        MoveRight(-dt);
    if (keyPressed['E'])
        MoveUp(dt);
    if (keyPressed['Q'])
        MoveUp(-dt);
}

void Camera::UpdateMouse(float mouseNdcX, float mouseNdcY)
{
}

void Camera::MoveForward(float dt)
{
    XMFLOAT3 newPosition = XMFLOAT3(m_position.x + m_viewDirection.x * m_speed * dt,
        m_position.y + m_viewDirection.y * m_speed * dt,
        m_position.z + m_viewDirection.z * m_speed * dt);

    m_position = newPosition;
}

void Camera::MoveRight(float dt)
{
    XMFLOAT3 newPosition = XMFLOAT3(m_position.x + m_rightDirection.x * m_speed * dt,
        m_position.y + m_rightDirection.y * m_speed * dt,
        m_position.z + m_rightDirection.z * m_speed * dt);

    m_position = newPosition;
}

void Camera::MoveUp(float dt)
{
    XMFLOAT3 newPosition = XMFLOAT3(m_position.x + m_upDirection.x * m_speed * dt,
        m_position.y + m_upDirection.y * m_speed * dt,
        m_position.z + m_upDirection.z * m_speed * dt);

    m_position = newPosition;
}

void Camera::SetAspectRatio(float aspect)
{
    m_aspect = aspect;
}
