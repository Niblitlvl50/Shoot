
#pragma once

#include "Entity/PhysicsEntityBase.h"
#include "Rendering/RenderPtrFwd.h"

namespace game
{
    class Meteor : public mono::PhysicsEntityBase
    {
    public:
        Meteor(float x, float y);
    
    private:
        void Draw(mono::IRenderer& renderer) const override;
        void Update(unsigned int delta) override;
        
        mono::ISpritePtr mSprite;
    };
}

