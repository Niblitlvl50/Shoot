
#pragma once

#include "ITool.h"
#include "Rendering/IDrawable.h"
#include "Rendering/Color.h"
#include <vector>

namespace editor
{
    class PaletteTool : public ITool, public mono::IDrawable
    {
    public:

        PaletteTool(class Editor* editor);

        void Begin() override;
        void End() override;
        bool IsActive() const override;
        void HandleContextMenu(int menu_index) override;
        void HandleMouseDown(const math::Vector& world_pos, uint32_t entity_id) override;
        void HandleMouseUp(const math::Vector& world_pos) override;
        void HandleMousePosition(const math::Vector& world_pos) override;
        void UpdateModifierState(bool ctrl, bool shift, bool alt) override;

        void Draw(mono::IRenderer& renderer) const override;
        math::Quad BoundingBox() const override;

        Editor* m_editor;
        mutable std::vector<mono::Color::RGBA> m_colors;
        mutable int m_active_index;
    };
}
