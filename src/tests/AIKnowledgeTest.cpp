
#include "gtest/gtest.h"

#include "AIKnowledge.h"

static_assert(game::n_players == 4);

TEST(AIKnowledgeTest, AllocateAndRelease)
{
    game::InitializeAIKnowledge();

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
    player_info_4->player_state = game::PlayerState::ALIVE;
    ASSERT_TRUE(player_info_4 != nullptr);

    game::PlayerInfo* player_info_5 = game::AllocatePlayerInfo();
    ASSERT_FALSE(player_info_5 != nullptr);

    game::ReleasePlayerInfo(player_info_1);

    game::PlayerInfo* player_info_6 = game::AllocatePlayerInfo();
    ASSERT_TRUE(player_info_6 != nullptr);

    //bool game::IsPlayer(uint32_t entity_id);
}

TEST(AIKnowledgeTest, FindPlayerInfo)
{
    game::InitializeAIKnowledge();

    game::PlayerInfo* player_info_1 = game::AllocatePlayerInfo();
    player_info_1->player_state = game::PlayerState::ALIVE;
    player_info_1->entity_id = 666;
    player_info_1->score = 666;

    game::PlayerInfo* player_info_2 = game::AllocatePlayerInfo();
    player_info_2->player_state = game::PlayerState::ALIVE;
    player_info_2->entity_id = 777;
    player_info_2->score = 777;

    const game::PlayerInfo* found_player_info_1 = game::FindPlayerInfoFromEntityId(666);
    EXPECT_EQ(found_player_info_1->score, player_info_1->score);

    const game::PlayerInfo* found_player_info_2 = game::FindPlayerInfoFromEntityId(777);
    EXPECT_EQ(found_player_info_2->score, player_info_2->score);
}

TEST(AIKnowledgeTest, GetClosestActivePlayer)
{
    game::InitializeAIKnowledge();

    game::PlayerInfo* player_info_1 = game::AllocatePlayerInfo();
    player_info_1->player_state = game::PlayerState::ALIVE;
    player_info_1->position = math::Vector(0.0f, 0.0f);

    game::PlayerInfo* player_info_2 = game::AllocatePlayerInfo();
    player_info_2->player_state = game::PlayerState::ALIVE;
    player_info_2->position = math::Vector(1.0f, 0.0f);

    game::PlayerInfo* player_info_3 = game::AllocatePlayerInfo();
    player_info_3->player_state = game::PlayerState::NOT_SPAWNED;
    player_info_3->position = math::Vector(2.0f, 0.0f);

    game::PlayerInfo* player_info_4 = game::AllocatePlayerInfo();
    player_info_4->player_state = game::PlayerState::ALIVE;
    player_info_4->position = math::Vector(3.0f, 0.0f);

    const game::PlayerInfo* found_player_info_1 = game::GetClosestActivePlayer(math::Vector(0.4f, 0.0f));
    EXPECT_FLOAT_EQ(found_player_info_1->position.x, player_info_1->position.x);

    const game::PlayerInfo* found_player_info_2 = game::GetClosestActivePlayer(math::Vector(0.7f, 0.0f));
    EXPECT_FLOAT_EQ(found_player_info_2->position.x, player_info_2->position.x);

    const game::PlayerInfo* found_player_info_3 = game::GetClosestActivePlayer(math::Vector(2.1f, 0.0f));
    EXPECT_FLOAT_EQ(found_player_info_3->position.x, player_info_4->position.x);
}
