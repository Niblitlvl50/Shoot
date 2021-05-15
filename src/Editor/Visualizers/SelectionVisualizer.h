
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

        void SetClickPoint(const math::Vector& world_point);
        void SetSelectionBox(const math::Quad& world_bb);

        void Draw(mono::IRenderer& renderer) const override;
        math::Quad BoundingBox() const override;

    private:

        const std::vector<uint32_t>& m_selected_ids;
        const uint32_t& m_preselection_id;
        mono::TransformSystem* m_transform_system;

        math::Vector m_selection_point;
        mutable uint32_t m_selection_point_timer;

        math::Quad m_selection_box;
        mutable uint32_t m_selection_box_timer;
    };
}
