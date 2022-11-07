
#pragma once

#include "IGameMode.h"
#include <memory>
#include <vector>
#include <string>

namespace game
{
    using IGameModePtr = std::unique_ptr<IGameMode>;

    class GameModeFactory
    {
    public:
        static IGameModePtr CreateGameMode(uint32_t game_mode);
        static std::vector<std::string> GetAllGameModes();
    };
}
