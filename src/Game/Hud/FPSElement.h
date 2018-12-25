
#pragma once

#include "Entity/EntityBase.h"
#include "Util/FpsCounter.h"

namespace game
{
    class FPSElement : public mono::EntityBase
    {
    public:

        FPSElement(const math::Vector& position);
        void Draw(mono::IRenderer& renderer) const override;
        void Update(unsigned int delta) override;

    private:
        mono::FpsCounter m_counter;        
    };
}
