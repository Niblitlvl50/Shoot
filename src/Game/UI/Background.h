
#pragma once

#include "Math/Quad.h"
#include "Rendering/Color.h"
#include "Zone/EntityBase.h"

namespace game
{
    class Background : public mono::EntityBase
    {
    public:
        Background(const math::Quad& viewport, const mono::Color::HSL& color);
        void Draw(mono::IRenderer& renderer) const override;
        void Update(const mono::UpdateContext& update_context) override;

    private:
        const math::Quad m_viewport;
        const mono::Color::HSL m_color;
    };    
}
