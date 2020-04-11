
#pragma once

#include "ITool.h"
#include "Math/Vector.h"
#include <vector>
#include <memory>

namespace editor
{
    class Editor;

    class PathTool : public ITool
    {
    public:

        PathTool(Editor* editor);
        ~PathTool();

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
        math::Vector m_mouse_position;
        std::vector<math::Vector> m_points;

        class Visualizer;
        std::unique_ptr<Visualizer> m_visualizer;
    };
}
