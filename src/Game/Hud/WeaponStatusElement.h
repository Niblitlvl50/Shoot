
#pragma once

#include "Overlay.h"
#include "Math/Vector.h"
#include "Rendering/RenderPtrFwd.h"

#include <vector>

namespace game
{
    struct PlayerInfo;

    class WeaponStatusElement : public mono::EntityBase
    {
    public:

        WeaponStatusElement(const PlayerInfo& player_info, const math::Vector& position);
        void Draw(mono::IRenderer& renderer) const override;
        void Update(unsigned int delta) override;

    private:

        const PlayerInfo& m_player_info;

        std::shared_ptr<class UITextElement> m_ammo_text;
        std::shared_ptr<class UISpriteElement> m_weapon_sprites;

        mono::ISpritePtr m_frame_sprite;
        mono::ISpritePtr m_bullet_sprite;
    };    
}
