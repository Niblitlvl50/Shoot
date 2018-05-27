
#pragma once

#include "Entity/EntityBase.h"
#include "Math/Matrix.h"

namespace game
{
    class UIOverlayDrawer : public mono::EntityBase
    {
    public:

        UIOverlayDrawer();
        ~UIOverlayDrawer();

    private:

        void doDraw(mono::IRenderer& renderer) const override;
        void Draw(mono::IRenderer& renderer) const override;
        void Update(unsigned int delta) override;
        math::Quad BoundingBox() const override;

        math::Matrix m_projection;
    };
}
