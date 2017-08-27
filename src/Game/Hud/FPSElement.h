
#pragma once

#include "Overlay.h"
#include "Utils.h"

namespace game
{
    class FPSElement : public UIElement
    {
    public:

        void Draw(mono::IRenderer& renderer) const;
        mutable mono::FPSCounter m_counter;        
    };
}
