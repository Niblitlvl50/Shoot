
#pragma once

#include "MonoFwd.h"
#include "IUpdatable.h"
#include "Math/Matrix.h"
#include "Rendering/IDrawable.h"
#include "Rendering/RenderBuffer/IRenderBuffer.h"
#include "Rendering/Sprite/ISpriteFactory.h"
#include "Rendering/Sprite/SpriteBufferFactory.h"
#include "Rendering/Text/TextBufferFactory.h"

#include <vector>

namespace game
{
    class DamageSystem;

    class HealthbarDrawer : public mono::IUpdatable, public mono::IDrawable
    {
    public:
        HealthbarDrawer(
            game::DamageSystem* damage_system, mono::TransformSystem* transform_system, mono::IEntityManager* entity_system);

        void Update(const mono::UpdateContext& update_context) override;
        void Draw(mono::IRenderer& renderer) const override;
        math::Quad BoundingBox() const override;

        game::DamageSystem* m_damage_system;
        mono::TransformSystem* m_transform_system;
        mono::IEntityManager* m_entity_system;

        mono::ISpritePtr m_boss_icon_sprite;
        mono::SpriteDrawBuffers m_sprite_buffers;
        std::unique_ptr<mono::IElementBuffer> m_indices;

        struct DamageNumber
        {
            float time_to_live_s;
            mono::TextDrawBuffers buffers;
            math::Matrix transform;
        };

        std::vector<DamageNumber> m_damage_numbers;
    };
}
