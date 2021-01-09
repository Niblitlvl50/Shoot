
#include "gtest/gtest.h"

#include "PredictionSystem/PositionPredictionSystem.h"

TEST(PredictionSystemTest, FindBestPredictionIndex)
{
    const game::PositionPredictionSystem::RemoteTransformBuffer prediction_buffer = {{
        { 100, math::Vector(), 0.0f, 0 },
        { 200, math::Vector(), 0.0f, 0 },
        { 300, math::Vector(), 0.0f, 0 },
        { 400, math::Vector(), 0.0f, 0 },
        { 500, math::Vector(), 0.0f, 0 },
        { 600, math::Vector(), 0.0f, 0 },
        { 700, math::Vector(), 0.0f, 0 },
        { 800, math::Vector(), 0.0f, 0 },
    }};

    const uint32_t best_index_1 = game::PositionPredictionSystem::FindBestPredictionIndex(0, prediction_buffer);
    const uint32_t best_index_2 = game::PositionPredictionSystem::FindBestPredictionIndex(123, prediction_buffer);
    const uint32_t best_index_3 = game::PositionPredictionSystem::FindBestPredictionIndex(666, prediction_buffer);
    const uint32_t best_index_4 = game::PositionPredictionSystem::FindBestPredictionIndex(900, prediction_buffer);

    EXPECT_EQ(0u, best_index_1);
    EXPECT_EQ(1u, best_index_2);
    EXPECT_EQ(6u, best_index_3);
    EXPECT_EQ(7u, best_index_4);
}
