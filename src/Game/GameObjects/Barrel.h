
#pragma once

#include <Rendering/RenderPtrFwd.h>
#include <Entity/PhysicsEntityBase.h>

#include <string>

namespace game
{
    class Barrel : public mono::PhysicsEntityBase
    {
    public:

        struct Configuration
        {
            std::string sprite_file;
            math::Vector scale;
            int health = -1;
        };

        Barrel(const Configuration& config);
        ~Barrel();
        
        void Update(unsigned int delta) override;
        void Draw(mono::IRenderer& renderer) const override;

    private:

        mono::ISpritePtr m_sprite;
    };
}
