
#pragma once

#include "Math/Vector.h"
#include "MonoFwd.h"
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

        mono::IEntity* entity;
        math::Vector start_position;
        math::Vector end_position;
    };

    struct RotateActionContext
    {
        EaseFunction ease_function = nullptr;

        bool ping_pong = false;
        int counter = 0;
        int duration = 0;

        mono::IEntity* entity;
        float start_rotation;
        float min_rotation;
        float max_rotation;
    };

    void UpdateMoveContexts(uint32_t delta_ms, std::vector<MoveActionContext>& move_contexts);
    void UpdateRotateContexts(uint32_t delta_ms, std::vector<RotateActionContext>& rotate_contexts);
}
