
#pragma once

#include <cstdint>

namespace game
{
    struct ScoreEvent
    {
        ScoreEvent(uint32_t entity_id, int score)
            : entity_id(entity_id)
            , score(score)  
        { }

        const uint32_t entity_id;
        const int score;
    };
}
