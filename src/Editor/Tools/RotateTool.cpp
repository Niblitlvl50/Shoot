
#include "RotateTool.h"
#include "Math/MathFunctions.h"
#include "Math/Vector.h"
#include "Math/Quad.h"
#include "Component.h"

#include "Editor.h"

#include <limits>

using namespace editor;

RotateTool::RotateTool(Editor* editor)
    : m_editor(editor)
    , m_entity_id(std::numeric_limits<uint32_t>::max())
    , m_rotation_diff(0.0f)
{ }

void RotateTool::Begin()
{ }

void RotateTool::End()
{
    m_entity_id = std::numeric_limits<uint32_t>::max();
}

bool RotateTool::IsActive() const
{
    return m_entity_id != std::numeric_limits<uint32_t>::max();
}

void RotateTool::HandleContextMenu(int menu_index)
{ }

void RotateTool::HandleMouseDown(const math::Vector& world_pos, uint32_t entity_id)
{
    m_entity_id = entity_id;
    if(m_entity_id == std::numeric_limits<uint32_t>::max())
        return;

    IObjectProxy* proxy = m_editor->FindProxyObject(entity_id);
    const math::Vector position = proxy->GetPosition();

    const float rotation = 0.0f; //m_entity->Rotation();

    m_rotation_diff = rotation - math::AngleBetweenPoints(position, world_pos);
}

void RotateTool::HandleMouseUp(const math::Vector& world_pos)
{
    End();
}

void RotateTool::HandleMousePosition(const math::Vector& world_pos)
{
    if(m_entity_id == std::numeric_limits<uint32_t>::max())
        return;

    IObjectProxy* proxy = m_editor->FindProxyObject(m_entity_id);

    const math::Vector position = proxy->GetPosition();
    const float angle = math::AngleBetweenPoints(position, world_pos);
    proxy->SetRotation(angle + m_rotation_diff);

    m_editor->EntityComponentUpdated(m_entity_id, TRANSFORM_COMPONENT);
    m_editor->UpdateGrabbers();
    m_editor->UpdateSnappers();
}

void RotateTool::UpdateModifierState(bool ctrl, bool shift, bool alt)
{ }

