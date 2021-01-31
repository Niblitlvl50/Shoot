
#pragma once

#include "Rendering/RenderFwd.h"
#include "Rendering/IDrawable.h"
#include <vector>
#include <memory>

namespace editor
{
    struct Grabber;

    class GrabberVisualizer : public mono::IDrawable
    {
    public:

        GrabberVisualizer(const std::vector<editor::Grabber>& grabbers);

        virtual void Draw(mono::IRenderer& renderer) const;
        virtual math::Quad BoundingBox() const;

        const std::vector<editor::Grabber>& m_grabbers;
        std::unique_ptr<mono::IRenderBuffer> m_vertices;
        std::unique_ptr<mono::IRenderBuffer> m_colors;
    };
}
