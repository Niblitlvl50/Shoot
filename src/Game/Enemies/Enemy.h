
#pragma once

#include "Rendering/RenderPtrFwd.h"
#include "Entity/PhysicsEntityBase.h"
#include "Math/Vector.h"

namespace game
{
    class IEnemyController : public mono::IUpdatable
    {
    public:

        virtual void Initialize(class Enemy* enemy) = 0;
    };

    struct EnemySetup
    {
        const char* sprite_file = nullptr;
        float size = 1.0f;
        float mass = 1.0f;
        math::Vector position;
        std::unique_ptr<IEnemyController> controller;
    };

    class Enemy : public mono::PhysicsEntityBase
    {
    public:

        Enemy(EnemySetup& setup);
        ~Enemy();

        void Draw(mono::IRenderer& renderer) const override;
        void Update(unsigned int delta) override;
        math::Quad BoundingBox() const override;

        std::unique_ptr<IEnemyController> m_controller;
        mono::ISpritePtr m_sprite;
    };
}
