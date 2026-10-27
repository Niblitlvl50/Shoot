
#include "gtest/gtest.h"

#include "World/WorldStateSystem.h"
#include "Player/SaveSystem.h"

#include <filesystem>
#include <string>

TEST(WorldStateTest, MissingKeysReadAsDefaults)
{
    game::WorldStateSystem world_state(nullptr);

    EXPECT_FALSE(world_state.Has("bridge_repaired"));
    EXPECT_FALSE(world_state.GetBool("bridge_repaired"));
    EXPECT_EQ(world_state.GetInt("villagers_saved"), 0);
    EXPECT_EQ(world_state.GetInt("villagers_saved", 7), 7);
}

TEST(WorldStateTest, SetAddAndRemove)
{
    game::WorldStateSystem world_state(nullptr);

    world_state.SetBool("bridge_repaired", true);
    world_state.SetInt("villagers_saved", 2);
    world_state.AddInt("villagers_saved", 3);
    world_state.AddInt("coins_found", 1);

    EXPECT_TRUE(world_state.GetBool("bridge_repaired"));
    EXPECT_EQ(world_state.GetInt("villagers_saved"), 5);
    EXPECT_EQ(world_state.GetInt("coins_found"), 1);

    world_state.Remove("bridge_repaired");
    EXPECT_FALSE(world_state.Has("bridge_repaired"));

    world_state.Clear();
    EXPECT_TRUE(world_state.GetValues().empty());
}

TEST(WorldStateTest, OnlyBoolAndIntAreLoaded)
{
    game::WorldStateSystem world_state(nullptr);

    game::WorldStateSystem::WorldStateValues values;
    values["flag"] = true;
    values["count"] = 4;
    values["name"] = std::string("not allowed");
    values["speed"] = 1.5f;
    world_state.SetValues(values);

    EXPECT_EQ(world_state.GetValues().size(), 2u);
    EXPECT_TRUE(world_state.GetBool("flag"));
    EXPECT_EQ(world_state.GetInt("count"), 4);
    EXPECT_FALSE(world_state.Has("name"));
}

TEST(WorldStateTest, SavesAndLoadsWithTheSlot)
{
    const std::string user_path = "world_state_test_";
    constexpr int slot_index = 99;

    game::SaveSlotData saved_data = {};
    saved_data.world_state["bridge_repaired"] = true;
    saved_data.world_state["villagers_saved"] = 12;
    saved_data.player_data[0].chips = 42;
    game::SavePlayerData(user_path.c_str(), slot_index, saved_data);

    game::SaveSlotData loaded_data = {};
    game::LoadPlayerData(user_path.c_str(), slot_index, loaded_data);

    std::filesystem::remove_all(user_path + "saves");

    ASSERT_EQ(loaded_data.world_state.size(), 2u);
    EXPECT_TRUE(std::get<bool>(loaded_data.world_state["bridge_repaired"]));
    EXPECT_EQ(std::get<int>(loaded_data.world_state["villagers_saved"]), 12);
    EXPECT_EQ(loaded_data.player_data[0].chips, 42);
}
