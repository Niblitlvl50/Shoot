
#pragma once

#include "MonoFwd.h"
#include "UIElements.h"

namespace game
{
    struct PlayerInfo;
    class WeaponSystem;
    class PickupSystem;

    class PlayerUIElement : public UIOverlay
    {
    public:

        PlayerUIElement(
            const PlayerInfo* player_infos,
            int num_players,
            game::WeaponSystem* weapon_system,
            game::PickupSystem* pickup_system,
            mono::SpriteSystem* sprite_system);

        ~PlayerUIElement();

    private:

        game::PickupSystem* m_pickup_system;
        uint32_t m_pickup_callback_id;
    };
}
