
#pragma once

#include "MonoFwd.h"
#include "Rendering/IDrawable.h"
#include "Math/Vector.h"
#include "Math/Quad.h"

#include <cstdint>
#include <vector>

namespace editor
{
    class SelectionVisualizer : public mono::IDrawable
    {
    public:

        SelectionVisualizer(
            const std::vector<uint32_t>& selected_ids, const uint32_t& preselection_id, mono::TransformSystem* transform_system);
        void PushSelectionQuad(const math::Quad& world_bb);

        void Draw(mono::IRenderer& renderer) const override;
        math::Quad BoundingBox() const override;

    private:

        const std::vector<uint32_t>& m_selected_ids;
        const uint32_t& m_preselection_id;
        mono::TransformSystem* m_transform_system;

        struct SelectionBox
        {
            math::Quad bb;
            uint32_t timer;
        };
        mutable std::vector<SelectionBox> m_selection_boxes;
    };
}
