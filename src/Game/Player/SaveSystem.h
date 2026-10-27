
#pragma once

#include "PlayerInfo.h"
#include "EntitySystem/ObjectAttribute.h"

#include <string>
#include <unordered_map>

namespace game
{
    struct SaveSlotData
    {
        PersistentPlayerData player_data[n_players];

        // The world state, persists between levels. Only bool and int values are saved.
        std::unordered_map<std::string, mono::Variant> world_state;
    };

    void SavePlayerData(const char* user_path, int slot_index, const SaveSlotData& data);
    void LoadPlayerData(const char* user_path, int slot_index, SaveSlotData& data);
}
