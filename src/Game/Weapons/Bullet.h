
#pragma once

#include "MonoPtrFwd.h"
#include "Entity/PhysicsEntityBase.h"
#include "Physics/IBody.h"
#include "Rendering/RenderPtrFwd.h"
#include "Particle/ParticleFwd.h"

#include "WeaponConfiguration.h"

namespace game
{
    class Bullet : public mono::PhysicsEntityBase, public mono::ICollisionHandler
    {
    public:

        Bullet(const BulletConfiguration& config);
        ~Bullet();

        void Draw(mono::IRenderer& renderer) const override;
        void Update(unsigned int delta) override;
        math::Quad BoundingBox() const override;
        void OnCollideWith(mono::IBody* body, unsigned int category) override;

    private:
        mono::ISpritePtr m_sprite;
        mono::ISoundPtr m_sound;

        std::unique_ptr<mono::ParticleEmitter> m_emitter;

        BulletImpactCallback m_collision_callback;
        int m_lifeSpan;
    };
}
