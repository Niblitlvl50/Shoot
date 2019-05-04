
#pragma once

#include "Entity/EntityBase.h"
#include "Math/Vector.h"

#include <vector>

namespace game
{
    struct PlayerInfo;

    class WeaponStatusElement : public mono::EntityBase
    {
    public:

        WeaponStatusElement(
            const PlayerInfo& player_info, const math::Vector& position, const math::Vector& offscreen_position);
        void Draw(mono::IRenderer& renderer) const override;
        void Update(const mono::UpdateContext& update_context) override;

    private:

        const PlayerInfo& m_player_info;
        const math::Vector m_screen_position;
        const math::Vector m_offscreen_position;
        float m_timer;

        std::shared_ptr<class UITextElement> m_ammo_text;
        std::shared_ptr<class UISpriteElement> m_weapon_sprites;
        std::shared_ptr<class UISpriteElement> m_frame_sprite;
    };    
}
