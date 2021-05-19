
#include "TranslateTool.h"
#include "Math/Vector.h"
#include "Math/Matrix.h"
#include "Math/MathFunctions.h"
#include "TransformSystem/TransformSystem.h"
#include "Component.h"

#include "Editor.h"

#include <limits>

using namespace editor;

constexpr uint32_t NO_ENTITY = std::numeric_limits<uint32_t>::max();

TranslateTool::TranslateTool(Editor* editor)
    : m_editor(editor)
    , m_entity_id(NO_ENTITY)
{ }

void TranslateTool::Begin()
{ }

void TranslateTool::End()
{
    m_entity_id = NO_ENTITY;
}

bool TranslateTool::IsActive() const
{
    return m_entity_id != NO_ENTITY;
}

void TranslateTool::HandleContextMenu(int menu_index)
{ }

void TranslateTool::HandleMouseDown(const math::Vector& world_pos, uint32_t entity_id)
{
    m_entity_id = entity_id;
    if(m_entity_id == NO_ENTITY)
        return;

    m_begin_translate = world_pos;
}

void TranslateTool::HandleMouseUp(const math::Vector& world_pos)
{
    End();
}

void TranslateTool::HandleMousePosition(const math::Vector& world_pos)
{
    if(m_entity_id == NO_ENTITY)
        return;

    const math::Vector& delta = world_pos - m_begin_translate;
    MoveObjects(m_entity_id, delta);

    m_begin_translate = world_pos;
}

void TranslateTool::UpdateModifierState(bool ctrl, bool shift, bool alt)
{ }

void TranslateTool::MoveObjects(uint32_t entity_id, const math::Vector& delta)
{
    std::vector<uint32_t> selected_proxies = m_editor->GetSelection();

    for(uint32_t id : selected_proxies)
    {
        IObjectProxy* proxy = m_editor->FindProxyObject(id);
        if(proxy)
        {
            const math::Vector new_pos = proxy->GetPosition() + delta;
            proxy->SetPosition(new_pos);
        }
    }

    m_editor->UpdateGrabbers();
    m_editor->UpdateSnappers();
}
