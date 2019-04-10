
#include "TranslateTool.h"
#include "Entity/IEntity.h"
#include "Math/Vector.h"
#include "Math/Matrix.h"
#include "Math/MathFunctions.h"
#include "TransformSystem.h"
#include "Component.h"

#include "Editor.h"

using namespace editor;

TranslateTool::TranslateTool(Editor* editor, mono::TransformSystem* transform_system)
    : m_editor(editor)
    , m_transform_system(transform_system)
    , m_entity_id(std::numeric_limits<uint32_t>::max())
    , m_was_snapped(false)
    , m_snap_rotate(false)
{ }

void TranslateTool::Begin()
{ }

void TranslateTool::End()
{
    m_entity_id = std::numeric_limits<uint32_t>::max();
}

bool TranslateTool::IsActive() const
{
    return m_entity_id != std::numeric_limits<uint32_t>::max();
}

void TranslateTool::HandleContextMenu(int menu_index)
{ }

void TranslateTool::HandleMouseDown(const math::Vector& world_pos, uint32_t entity_id)
{
    m_entity_id = entity_id;
    if(m_entity_id == std::numeric_limits<uint32_t>::max())
        return;

    const math::Matrix& transform = m_transform_system->GetTransform(m_entity_id);

    m_begin_translate = world_pos;
    m_position_diff = math::GetPosition(transform) - world_pos;
}

void TranslateTool::HandleMouseUp(const math::Vector& world_pos)
{
    End();
}

void TranslateTool::HandleMousePosition(const math::Vector& world_pos)
{
    if(m_entity_id == std::numeric_limits<uint32_t>::max())
        return;

    const math::Vector& delta = m_begin_translate - world_pos;
    const math::Vector& new_pos = m_begin_translate - delta + m_position_diff;

    math::Matrix& transform = m_transform_system->GetTransform(m_entity_id);
    math::Position(transform, new_pos);

    /*
    const SnapPair& snap_pair = m_editor->FindSnapPosition(new_pos);
    if(snap_pair.found_snap && !m_was_snapped)
    {
        if(m_snap_rotate)
        {
            const math::Vector& snap_offset = m_entity->Position() - snap_pair.snap_from.position;
            const float new_angle = math::NormalizeAngle(snap_pair.snap_from.normal - snap_pair.snap_to.normal - math::PI());

            math::Matrix translation;
            math::Translate(translation, snap_pair.snap_to.position);

            math::Matrix rotation;
            math::RotateZ(rotation, new_angle, snap_offset);

            const math::Matrix& transform = translation * rotation;
            const math::Vector& position = math::Transform(transform, snap_offset);
            m_entity->SetPosition(position);
            m_entity->SetRotation(math::NormalizeAngle(new_angle + m_entity->Rotation()));
        }
        else
        {
            const math::Vector& snap_offset = m_entity->Position() - snap_pair.snap_from.position;

            math::Matrix translation;
            math::Translate(translation, snap_pair.snap_to.position);

            const math::Vector& position = math::Transform(translation, snap_offset);
            m_entity->SetPosition(position);
        }

        End();
    }

    m_was_snapped = snap_pair.found_snap;
    */

    m_editor->EntityComponentUpdated(m_entity_id, TRANSFORM_COMPONENT);
    m_editor->UpdateGrabbers();
    m_editor->UpdateSnappers();
}

void TranslateTool::UpdateModifierState(bool ctrl, bool shift, bool alt)
{
    m_snap_rotate = shift;
}
