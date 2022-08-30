
#pragma once

#include "MonoFwd.h"
#include "UIElements.h"

namespace game
{
    struct PlayerInfo;
    class WeaponSystem;

    class PlayerUIElement : public UIOverlay
    {
    public:

        PlayerUIElement(
            const PlayerInfo* player_infos,
            int num_players,
            game::WeaponSystem* weapon_system,
            mono::SpriteSystem* sprite_system,
            mono::EventHandler* event_handler);
    };
}
