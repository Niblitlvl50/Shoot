
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
            float mass = 10.0f;
            int health = -1;
        };

        Barrel(const Configuration& config);
        ~Barrel();
        
        void Update(unsigned int delta) override;
        void Draw(mono::IRenderer& renderer) const override;
        math::Quad BoundingBox() const override;

    private:

        mono::ISpritePtr m_sprite;
    };
}
