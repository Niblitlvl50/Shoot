
#pragma once

#include "Zone/EntityBase.h"
#include "Math/Vector.h"

#include <memory>

namespace game
{
    struct PlayerInfo;

    class PlayerUIElement : public mono::EntityBase
    {
    public:

        PlayerUIElement(
            const PlayerInfo& player_info, const math::Vector& position, const math::Vector& offscreen_position);
        ~PlayerUIElement();
        void Draw(mono::IRenderer& renderer) const override;
        void Update(const mono::UpdateContext& update_context) override;

    private:

        const PlayerInfo& m_player_info;
        const math::Vector m_screen_position;
        const math::Vector m_offscreen_position;
        float m_timer;

        int m_current_score;

        std::unique_ptr<class UITextElement> m_ammo_text;
        std::unique_ptr<class UISpriteElement> m_weapon_sprites;
        std::unique_ptr<class UISpriteElement> m_frame_sprite;
        std::unique_ptr<class UISpriteElement> m_mugshot_sprite;

        std::unique_ptr<class UITextElement> m_score_text;
    };
}
