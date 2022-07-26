
#include "RotateTool.h"
#include "Math/MathFunctions.h"
#include "Math/Vector.h"
#include "Math/Quad.h"
#include "Entity/Component.h"
#include "EntitySystem/Entity.h"

#include "Editor.h"


using namespace editor;

RotateTool::RotateTool(Editor* editor)
    : m_editor(editor)
    , m_entity_id(mono::INVALID_ID)
    , m_rotation_diff(0.0f)
{ }

void RotateTool::Begin()
{ }

void RotateTool::End()
{
    m_entity_id = mono::INVALID_ID;
}

bool RotateTool::IsActive() const
{
    return m_entity_id != mono::INVALID_ID;
}

void RotateTool::HandleContextMenu(int menu_index)
{ }

void RotateTool::HandleMouseDown(const math::Vector& world_pos, uint32_t entity_id)
{
    m_entity_id = entity_id;

    if(m_entity_id == mono::INVALID_ID)
        return;

    IObjectProxy* proxy = m_editor->FindProxyObject(entity_id);
    const math::Vector position = proxy->GetPosition();
    const float rotation = proxy->GetRotation();

    m_rotation_diff = rotation - math::AngleBetweenPointsSimple(position, world_pos);
}

void RotateTool::HandleMouseUp(const math::Vector& world_pos)
{
    m_entity_id = mono::INVALID_ID;
}

void RotateTool::HandleMousePosition(const math::Vector& world_pos)
{
    if(m_entity_id == mono::INVALID_ID)
        return;

    IObjectProxy* proxy = m_editor->FindProxyObject(m_entity_id);

    const math::Vector position = proxy->GetPosition();
    const float angle = math::AngleBetweenPointsSimple(position, world_pos);
    proxy->SetRotation(angle + m_rotation_diff);

    m_editor->UpdateGrabbers();
    m_editor->UpdateSnappers();
}

void RotateTool::UpdateModifierState(bool ctrl, bool shift, bool alt)
{ }

