
#include "RotateTool.h"
#include "Math/MathFunctions.h"
#include "Math/Vector.h"
#include "Math/Quad.h"
#include "TransformSystem.h"
#include "Component.h"

#include "Editor.h"

using namespace editor;

RotateTool::RotateTool(Editor* editor, mono::TransformSystem* transform_system)
    : m_editor(editor)
    , m_transform_system(transform_system)
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

    const math::Matrix& transform = m_transform_system->GetTransform(m_entity_id);
    const math::Vector position = math::GetPosition(transform);

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

    math::Matrix& transform = m_transform_system->GetTransform(m_entity_id);

    const math::Vector position = math::GetPosition(transform);
    const float angle = math::AngleBetweenPoints(position, world_pos);

    transform = math::CreateMatrixFromZRotation(angle + m_rotation_diff);
    math::Position(transform, position);

    m_editor->EntityComponentUpdated(m_entity_id, TRANSFORM_COMPONENT);
    m_editor->UpdateGrabbers();
    m_editor->UpdateSnappers();
}

void RotateTool::UpdateModifierState(bool ctrl, bool shift, bool alt)
{ }

