
#pragma once

#include "MonoFwd.h"
#include "Rendering/IDrawable.h"
#include "Rendering/Sprite/ISpriteFactory.h"
#include "Rendering/Sprite/SpriteBufferFactory.h"
#include "Rendering/Text/TextBufferFactory.h"
#include "Rendering/Color.h"

#include <unordered_map>
#include <vector>

namespace game
{
    class InteractionSystemDrawer : public mono::IDrawable
    {
    public:

        InteractionSystemDrawer(
            class InteractionSystem* interaction_system,
            mono::SpriteSystem* sprite_system,
            const mono::TransformSystem* transform_system,
            const mono::EntitySystem* entity_system);

        void Draw(mono::IRenderer& renderer) const override;
        math::Quad BoundingBox() const override;

        InteractionSystem* m_interaction_system;
        mono::SpriteSystem* m_sprite_system;
        const mono::TransformSystem* m_transform_system;
        const mono::EntitySystem* m_entity_system;

        struct VerbSpriteBuffer
        {
            mono::ISpritePtr sprite;
            mono::SpriteDrawBuffers sprite_buffer;
        };
        std::vector<VerbSpriteBuffer> m_verb_sprites_buffers;
        std::vector<mono::TextDrawBuffers> m_verb_text_buffers;
        std::vector<float> m_verb_text_widths;

        std::unique_ptr<mono::IElementBuffer> m_indices;

        struct BackupData
        {
            mono::Color::RGBA shade;
        };
        mutable std::unordered_map<uint32_t, BackupData> m_backup_data;
    };
}
