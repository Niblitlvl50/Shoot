
#pragma once

#include "ITool.h"
#include "Math/Vector.h"
#include <vector>

namespace editor
{
    class Editor;

    class PolygonBoxTool : public editor::ITool
    {
    public:

        PolygonBoxTool(editor::Editor* editor);

        virtual void Begin();
        virtual void End();
        virtual bool IsActive() const;
        virtual void HandleContextMenu(int menu_index);
        virtual void HandleMouseDown(const math::Vector& world_pos, mono::IEntityPtr entity);
        virtual void HandleMouseUp(const math::Vector& world_pos);
        virtual void HandleMousePosition(const math::Vector& world_pos);
        virtual void UpdateModifierState(bool ctrl, bool shift, bool alt);

        editor::Editor* m_editor = nullptr;
        bool m_active = false;

        std::shared_ptr<class PolygonVisualizer2> m_visualizer;
        
        math::Vector m_mouse_down;
        math::Vector m_mouse_position;
        std::vector<math::Vector> m_points;
    };
}
