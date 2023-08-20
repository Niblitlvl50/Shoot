
#include "gtest/gtest.h"

#include "Player/PlayerInfo.h"

static_assert(game::n_players == 3);

TEST(PlayerInfoTest, AllocateAndRelease)
{
    game::InitializePlayerInfo();

    game::PlayerInfo* player_info_1 = game::AllocatePlayerInfo();
    player_info_1->player_state = game::PlayerState::ALIVE;
    ASSERT_TRUE(player_info_1 != nullptr);

    game::PlayerInfo* player_info_2 = game::AllocatePlayerInfo();
    player_info_2->player_state = game::PlayerState::ALIVE;
    ASSERT_TRUE(player_info_2 != nullptr);

    game::PlayerInfo* player_info_3 = game::AllocatePlayerInfo();
    player_info_3->player_state = game::PlayerState::ALIVE;
    ASSERT_TRUE(player_info_3 != nullptr);

    game::PlayerInfo* player_info_4 = game::AllocatePlayerInfo();
    ASSERT_FALSE(player_info_4 != nullptr);

    game::ReleasePlayerInfo(player_info_1);

    game::PlayerInfo* player_info_5 = game::AllocatePlayerInfo();
    ASSERT_TRUE(player_info_5 != nullptr);

    //bool game::IsPlayer(uint32_t entity_id);
}

TEST(PlayerInfoTest, FindPlayerInfo)
{
    game::InitializePlayerInfo();

    game::PlayerInfo* player_info_1 = game::AllocatePlayerInfo();
    player_info_1->player_state = game::PlayerState::ALIVE;
    player_info_1->entity_id = 666;
    player_info_1->killer_entity_id = 1;

    game::PlayerInfo* player_info_2 = game::AllocatePlayerInfo();
    player_info_2->player_state = game::PlayerState::ALIVE;
    player_info_2->entity_id = 777;
    player_info_2->killer_entity_id = 11;

    const game::PlayerInfo* found_player_info_1 = game::FindPlayerInfoFromEntityId(666);
    EXPECT_EQ(found_player_info_1->killer_entity_id, player_info_1->killer_entity_id);

    const game::PlayerInfo* found_player_info_2 = game::FindPlayerInfoFromEntityId(777);
    EXPECT_EQ(found_player_info_2->killer_entity_id, player_info_2->killer_entity_id);
}
