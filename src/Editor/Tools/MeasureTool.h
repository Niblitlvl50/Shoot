
#pragma once

#include "ITool.h"
#include "Math/Vector.h"
#include <memory>

namespace editor
{
    class Editor;

    class MeasureTool : public ITool
    {
    public:
        
        MeasureTool(Editor* editor);
        ~MeasureTool();

        void Begin() override;
        void End() override;
        bool IsActive() const override;
        void HandleContextMenu(int menu_index) override;
        void HandleMouseDown(const math::Vector& world_pos, uint32_t entity_id) override;
        void HandleMouseUp(const math::Vector& world_pos) override;
        void HandleMousePosition(const math::Vector& world_pos) override;
        void UpdateModifierState(bool ctrl, bool shift, bool alt) override;

        Editor* m_editor;
        bool m_active;
        math::Vector m_start_position;
        math::Vector m_end_position;

        class Visualizer;
        std::unique_ptr<Visualizer> m_visualizer;
    };
}
