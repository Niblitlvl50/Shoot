
#include "TranslateTool.h"
#include "Entity/IEntity.h"
#include "Math/Vector.h"
#include "Math/Matrix.h"
#include "Math/MathFunctions.h"
#include "Editor.h"

using namespace editor;

TranslateTool::TranslateTool(Editor* editor)
    : m_editor(editor)
    , m_was_snapped(false)
{ }

void TranslateTool::Begin()
{ }

void TranslateTool::End()
{
    m_entity = nullptr;
}

bool TranslateTool::IsActive() const
{
    return m_entity != nullptr;
}

void TranslateTool::HandleContextMenu(int menu_index)
{ }

void TranslateTool::HandleMouseDown(const math::Vector& world_pos, mono::IEntityPtr entity)
{
    m_entity = entity;
    if(!m_entity)
        return;

    m_begin_translate = world_pos;
    m_position_diff = m_entity->Position() - world_pos;
}

void TranslateTool::HandleMouseUp(const math::Vector& world_pos)
{
    End();
}

void TranslateTool::HandleMousePosition(const math::Vector& world_pos)
{
    if(!m_entity)
        return;

    const math::Vector& delta = m_begin_translate - world_pos;
    const math::Vector& new_pos = m_begin_translate - delta + m_position_diff;
    m_entity->SetPosition(new_pos);

    const SnapPair& snap_pair = m_editor->FindSnapPosition(new_pos);
    if(snap_pair.found_snap && !m_was_snapped)
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

        End();
    }

    m_was_snapped = snap_pair.found_snap;

    m_editor->UpdateGrabbers();
    m_editor->UpdateSnappers();
}
