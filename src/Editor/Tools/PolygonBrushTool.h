
#pragma once

#include "ITool.h"
#include "Math/Vector.h"
#include <memory>
#include <vector>

namespace editor
{
    class Editor;
    class PolygonEntity;

    class PolygonBrushTool : public ITool
    {
    public:

        PolygonBrushTool(Editor* editor);
        ~PolygonBrushTool();

        virtual void Begin();
        virtual void End();
        virtual bool IsActive() const;
        virtual void HandleContextMenu(int menu_index);
        virtual void HandleMouseDown(const math::Vector& world_pos, uint32_t entity_id);
        virtual void HandleMouseUp(const math::Vector& world_pos);
        virtual void HandleMousePosition(const math::Vector& world_pos);
        virtual void UpdateModifierState(bool ctrl, bool shift, bool alt);

        Editor* m_editor;
        bool m_painting;

        editor::PolygonEntity* m_polygon_entity;
        math::Vector m_previously_added_point;
        math::Vector m_previous_point;
        float m_direction;

        class Visualizer;
        std::unique_ptr<Visualizer> m_visualizer;

        std::vector<math::Vector> m_drawn_points;
    };
}
