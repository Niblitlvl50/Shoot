
#pragma once

#include "MonoFwd.h"
#include "UIElements.h"

namespace game
{
    struct PlayerInfo;

    class PlayerUIElement : public UIOverlay
    {
    public:

        PlayerUIElement(
            const PlayerInfo* player_infos, int num_players, mono::SpriteSystem* sprite_system, mono::EventHandler* event_handler);
    };
}
