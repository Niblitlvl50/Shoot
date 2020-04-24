
#pragma once

#include "Zone/EntityBase.h"

namespace game
{
    class SmokeEffect : public mono::EntityBase
    {
    public:

        SmokeEffect(const math::Vector& position);
        virtual ~SmokeEffect();

        void Draw(mono::IRenderer& renderer) const override;
        void Update(const mono::UpdateContext& update_context) override;
    };
}
