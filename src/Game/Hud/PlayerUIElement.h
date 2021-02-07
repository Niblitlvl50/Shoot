
#pragma once

#include "MonoFwd.h"
#include "UIElements.h"
#include "Math/Vector.h"

namespace game
{
    struct PlayerInfo;

    class PlayerUIElement : public UIOverlay
    {
    public:

        PlayerUIElement(const PlayerInfo& player_info);
        void Update(const mono::UpdateContext& update_context) override;
        void Draw(mono::IRenderer& renderer) const override;

    private:

        const PlayerInfo& m_player_info;
        math::Vector m_screen_position;
        math::Vector m_offscreen_position;
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
