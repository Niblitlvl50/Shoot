
#pragma once

#include "PlayerInfo.h"

namespace game
{
    struct SaveSlotData
    {
        PersistentPlayerData player_data[n_players];
    };

    void SavePlayerData(const char* user_path, int slot_index, const SaveSlotData& data);
    void LoadPlayerData(const char* user_path, int slot_index, SaveSlotData& data);
}
