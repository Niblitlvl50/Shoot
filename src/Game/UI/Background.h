
#pragma once

#include "Math/Quad.h"
#include "Rendering/Color.h"
#include "Entity/EntityBase.h"

namespace game
{
    class Background : public mono::EntityBase
    {
    public:
        Background(const math::Quad& viewport, const mono::Color::HSL& color);
        void Draw(mono::IRenderer& renderer) const override;
        void Update(unsigned int delta) override;

    private:
        const math::Quad m_viewport;
        const mono::Color::HSL m_color;
    };    
}
