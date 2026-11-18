
#pragma once

#include "ITool.h"
#include "Math/Vector.h"
#include <memory>

namespace editor
{
    class Editor;
    struct UIContext;

    // Paints the ground layers into the splat map. Left mouse paints the selected layer, holding alt
    // erases back to the base layer.
    class GroundPaintTool : public ITool
    {
    public:

        GroundPaintTool(Editor* editor, UIContext* context);
        ~GroundPaintTool();

        void Begin() override;
        void End() override;
        bool IsActive() const override;
        void HandleContextMenu(int menu_index) override;
        void HandleMouseDown(const math::Vector& world_pos, uint32_t entity_id) override;
        void HandleMouseUp(const math::Vector& world_pos) override;
        void HandleMousePosition(const math::Vector& world_pos) override;
        void UpdateModifierState(bool ctrl, bool shift, bool alt) override;

    private:

        void PaintAt(const math::Vector& world_pos);

        Editor* m_editor;
        UIContext* m_context;
        bool m_painting;
        bool m_erase;
        math::Vector m_brush_position;

        class Visualizer;
        std::unique_ptr<Visualizer> m_visualizer;
    };
}
