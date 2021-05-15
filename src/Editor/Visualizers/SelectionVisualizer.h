
#pragma once

#include "MonoFwd.h"
#include "Rendering/RenderFwd.h"
#include "Rendering/IDrawable.h"

#include <cstdint>
#include <memory>
#include <vector>

namespace editor
{
    class SelectionVisualizer : public mono::IDrawable
    {
    public:

        SelectionVisualizer(
            const std::vector<uint32_t>& selected_ids, const uint32_t& preselection_id, mono::TransformSystem* transform_system);
        void Draw(mono::IRenderer& renderer) const override;
        math::Quad BoundingBox() const override;

    private:

        const std::vector<uint32_t>& m_selected_ids;
        const uint32_t& m_preselection_id;
        mono::TransformSystem* m_transform_system;

        std::unique_ptr<mono::IRenderBuffer> m_vertices;
        std::unique_ptr<mono::IRenderBuffer> m_colors;
        std::unique_ptr<mono::IElementBuffer> m_indices;
    };
}
