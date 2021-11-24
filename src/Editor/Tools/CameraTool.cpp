
#include "CameraTool.h"

#include "Camera/ICamera.h"
#include "Math/Quad.h"
#include "Math/MathFunctions.h"
#include "System/System.h"

#include <cmath>

using namespace editor;

CameraTool::CameraTool(mono::ICamera* camera, const System::IWindow* window)
    : m_camera(camera),
      m_window(window),
      m_translate(false)
{ }

bool CameraTool::IsActive() const
{
    return m_translate;
}

void CameraTool::HandleMouseDown(const math::Vector& screen_position)
{
    m_translate = true;
    m_last_position = screen_position;
}

void CameraTool::HandleMouseUp(const math::Vector& screen_position)
{
    m_translate = false;
}

void CameraTool::HandleMousePosition(const math::Vector& screen_position)
{
    if(!m_translate)
        return;

    const System::Size& size = m_window->Size();
    const math::Vector window_size(size.width, size.height);
    const float ratio = window_size.x / window_size.y;

    math::Quad viewport = m_camera->GetViewport();
    viewport.mB.y = viewport.mB.x / ratio;

    const math::Vector& scale = viewport.mB / window_size;

    math::Vector delta = (screen_position - m_last_position);
    delta.y = -delta.y;
    delta *= scale;

    const math::Vector& cam_pos = m_camera->GetPosition();
    const math::Vector& new_pos = cam_pos - delta;

    m_camera->SetPosition(new_pos);

    m_last_position = screen_position;
}

void CameraTool::HandleMouseWheel(float x, float y)
{
    math::Vector viewport_size = m_camera->GetViewportSize();

    const float multiplier = (y < 0.0f) ? 1.0f : -1.0f;
    const float resize_value = viewport_size.x * 0.15f * multiplier;
    const float aspect = viewport_size.x / viewport_size.y;

    viewport_size.x += resize_value * aspect;
    viewport_size.y += resize_value;

    m_camera->SetTargetViewportSize(viewport_size);
}

void CameraTool::HandleMultiGesture(const math::Vector& screen_position, float distance)
{
    if(std::fabs(distance) < 1e-3)
        return;

    const float multiplier = (distance < 0.0f) ? -1.0f : 1.0f;
    HandleMouseWheel(0.0f, multiplier);
}

void CameraTool::MoveCamera(const math::Vector& delta)
{
    m_camera->Move(delta);
}
