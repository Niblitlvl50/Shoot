
#pragma once

#include "Overlay.h"
#include "Utils.h"
#include "Math/Vector.h"

namespace game
{
    class FPSElement : public UIElement
    {
    public:

        FPSElement(const math::Vector& position);
        void Draw(mono::IRenderer& renderer) const override;

        const math::Vector m_position;
        mutable mono::FPSCounter m_counter;        
    };
}
