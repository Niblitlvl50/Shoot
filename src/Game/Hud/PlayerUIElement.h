
#pragma once

#include "MonoFwd.h"
#include "Rendering/IDrawable.h"
#include "IUpdatable.h"
#include "Math/Vector.h"

namespace game
{
    struct PlayerInfo;

    class PlayerUIElement : public mono::IUpdatable
    {
    public:

        PlayerUIElement(
            const PlayerInfo& player_info,
            const math::Vector& position,
            const math::Vector& offscreen_position,
            mono::SpriteSystem* sprite_system,
            mono::IEntityManager* entity_manager);
        
        void Update(const mono::UpdateContext& update_context) override;
        //void EntityDraw(mono::IRenderer& renderer) const override;

    private:

        const PlayerInfo& m_player_info;
        math::Vector m_position;
        const math::Vector m_screen_position;
        const math::Vector m_offscreen_position;
        float m_timer;

        int m_current_score;

        class UISquareElement* m_background;
        class UITextElement* m_ammo_text;
        class UISpriteElement* m_weapon_sprites;
        class UISpriteElement* m_mugshot_sprite;
        class UITextElement* m_weapon_state_text;

        class UITextElement* m_score_text;
    };
}
