
#pragma once

#include "Rendering/IDrawable.h"
#include "Rendering/RenderPtrFwd.h"

namespace editor
{
    class ScaleVisualizer : public mono::IDrawable
    {
    public:

        ScaleVisualizer(const mono::ICameraPtr& camera);
        virtual void doDraw(mono::IRenderer& renderer) const;
        virtual math::Quad BoundingBox() const;
        
        const mono::ICameraPtr& m_camera;
    };
}
