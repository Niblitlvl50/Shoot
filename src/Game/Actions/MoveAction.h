
#pragma once

#include "Math/Vector.h"
#include "MonoPtrFwd.h"
#include "EasingFunctions.h"

#include <vector>
#include <functional>

namespace game
{
    struct MoveActionContext
    {
        EaseFunction ease_func = nullptr;
        std::function<void ()> callback = nullptr;

        bool ping_pong = false;
        int counter = 0;
        int duration = 0;

        mono::IEntityPtr entity;
        math::Vector start_position;
        math::Vector end_position;
    };

    void UpdateMoveContexts(unsigned int delta, std::vector<MoveActionContext>& move_contexts);
}
