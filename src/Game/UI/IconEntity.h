
#pragma once

#include "Entity/EntityBase.h"
#include "Rendering/RenderPtrFwd.h"
#include "Rendering/Sprite/ISpriteFactory.h"

namespace game
{
    class IconEntity : public mono::EntityBase
    {
    public:

        IconEntity(const char* sprite_file);
        ~IconEntity();
        
        void Update(const mono::UpdateContext& update_context) override;
        void Draw(mono::IRenderer& renderer) const override;

        mono::ISpritePtr m_sprite;
    };
}
