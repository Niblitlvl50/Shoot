
#pragma once

#include "MonoFwd.h"
#include "Rendering/IDrawable.h"
#include "Rendering/Texture/ITextureFactory.h"

#include "Effects/SmokeEffect.h"

#include "Paths/PathDrawBuffer.h"

#include <unordered_map>

namespace game
{
    class SpawnSystemDrawer : public mono::IDrawable
    {
    public:

        SpawnSystemDrawer(
            class SpawnSystem* spawn_system, mono::TransformSystem* transform_system, mono::ParticleSystem* particle_system, mono::IEntityManager* entity_manager);
        void Draw(mono::IRenderer& renderer) const override;
        math::Quad BoundingBox() const override;

        void DebugDrawSpawnPoints(mono::IRenderer& renderer) const;

        SpawnSystem* m_spawn_system;
        mono::TransformSystem* m_transform_system;
        mutable SmokeEffect m_smoke_effect;

        mono::ITexturePtr m_texture;

        struct SpawnEffect
        {
            mono::PathDrawBuffer buffers;
            bool emit_smoke;
        };

        mutable std::unordered_map<uint32_t, SpawnEffect> m_path_draw_buffers;
    };
}
