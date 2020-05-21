
#pragma once

#include "Zone/EntityBase.h"
#include "Math/Matrix.h"

namespace game
{
    class UIOverlayDrawer : public mono::EntityBase
    {
    public:

        UIOverlayDrawer();

    private:

        void Draw(mono::IRenderer& renderer) const override;
        void EntityDraw(mono::IRenderer& renderer) const override;
        void EntityUpdate(const mono::UpdateContext& update_context) override;
        math::Quad BoundingBox() const override;

        math::Matrix m_projection;
    };
}
