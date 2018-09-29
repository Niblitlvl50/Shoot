
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

    struct RotateActionContext
    {
        EaseFunction ease_function = nullptr;

        bool ping_pong = false;
        int counter = 0;
        int duration = 0;

        mono::IEntityPtr entity;
        float start_rotation;
        float min_rotation;
        float max_rotation;
    };

    void UpdateMoveContexts(unsigned int delta, std::vector<MoveActionContext>& move_contexts);
    void UpdateRotateContexts(unsigned int delta, std::vector<RotateActionContext>& rotate_contexts);
}
