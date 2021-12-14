
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

    private:

        const PlayerInfo& m_player_info;
        math::Vector m_screen_position;
        math::Vector m_offscreen_position;
        float m_timer;

        class UITextElement* m_ammo_text;
        class UISpriteElement* m_weapon_sprites;
        class UITextElement* m_weapon_state_text;
    };
}
