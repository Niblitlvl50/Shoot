
#pragma once

#include "MonoFwd.h"
#include "UIElements.h"

namespace game
{
    struct PlayerInfo;

    class PlayerUIElement : public UIOverlay
    {
    public:

        PlayerUIElement(const PlayerInfo* player_infos, int n_players, mono::EventHandler* event_handler);
    };
}
