
#pragma once

#include "ITool.h"
#include <memory>

namespace mono
{
    class TransformSystem;
}

namespace editor
{
    class Editor;
    
    class RotateTool : public ITool
    {
    public:

        RotateTool(Editor* editor, mono::TransformSystem* transform_system);

        virtual void Begin();
        virtual void End();
        virtual bool IsActive() const;
        virtual void HandleContextMenu(int menu_index);
        virtual void HandleMouseDown(const math::Vector& world_pos, uint32_t entity_id);
        virtual void HandleMouseUp(const math::Vector& world_pos);
        virtual void HandleMousePosition(const math::Vector& world_pos);
        virtual void UpdateModifierState(bool ctrl, bool shift, bool alt);

    private:
        Editor* m_editor;
        mono::TransformSystem* m_transform_system;
        uint32_t m_entity_id;
        float m_rotation_diff;
    };
}
