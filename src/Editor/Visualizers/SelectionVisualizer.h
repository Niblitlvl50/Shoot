
#pragma once

#include "Rendering/IDrawable.h"
#include <cstdint>

namespace mono
{
    class TransformSystem;
}

namespace editor
{
    class SelectionVisualizer : public mono::IDrawable
    {
    public:

        SelectionVisualizer(const uint32_t& selection_id, mono::TransformSystem* transform_system);
        void doDraw(mono::IRenderer& renderer) const override;
        math::Quad BoundingBox() const override;

    private:

        const uint32_t& m_selection_id;
        mono::TransformSystem* m_transform_system;
    };
}
