
#pragma once

#include "Entity/EntityBase.h"
#include "Rendering/RenderPtrFwd.h"

namespace game
{
    class IconEntity : public mono::EntityBase
    {
    public:

        IconEntity(const char* sprite_file);
        ~IconEntity();
        
        void Update(unsigned int delta) override;
        void Draw(mono::IRenderer& renderer) const override;

        mono::ISpritePtr m_sprite;
    };
}
