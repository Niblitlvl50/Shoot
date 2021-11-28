
#include "TranslateTool.h"
#include "Editor.h"
#include "Math/MathFunctions.h"
#include <cassert>

using namespace editor;

TranslateTool::TranslateTool(Editor* editor)
    : m_editor(editor)
{ }

void TranslateTool::Begin()
{ }

void TranslateTool::End()
{
    m_proxy_start_positions.clear();
}

bool TranslateTool::IsActive() const
{
    return !m_proxy_start_positions.empty();
}

void TranslateTool::HandleContextMenu(int menu_index)
{ }

void TranslateTool::HandleMouseDown(const math::Vector& world_pos, uint32_t entity_id)
{
    assert(m_proxy_start_positions.empty());
    m_proxy_start_positions.clear();

    m_begin_translate = world_pos;

    for(uint32_t id : m_editor->GetSelection())
    {
        IObjectProxy* proxy = m_editor->FindProxyObject(id);
        m_proxy_start_positions.push_back(proxy->GetPosition());
    }
}

void TranslateTool::HandleMouseUp(const math::Vector& world_pos)
{
    End();
}

void TranslateTool::HandleMousePosition(const math::Vector& world_pos)
{
    if(IsActive())
        MoveObjects(world_pos - m_begin_translate);
}

void TranslateTool::UpdateModifierState(bool ctrl, bool shift, bool alt)
{ }

void TranslateTool::MoveObjects(const math::Vector& delta)
{
    const std::vector<uint32_t> selected_proxies = m_editor->GetSelection();

    for(size_t index = 0; index < selected_proxies.size(); ++index)
    {
        const uint32_t id = selected_proxies[index];
        const math::Vector& start_position = m_proxy_start_positions[index];

        IObjectProxy* proxy = m_editor->FindProxyObject(id);
        if(proxy)
            proxy->SetPosition(start_position + delta);
    }

    m_editor->UpdateGrabbers();
    m_editor->UpdateSnappers();
}
