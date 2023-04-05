
#pragma once

#include "MonoFwd.h"
#include "Math/MathFwd.h"
#include "Math/Vector.h"

#include <cstdint>

namespace game
{
    enum class PrimaryAIBehaviour : uint32_t
    {
        TargetPlayer,
        TargetPackage
    };

    struct AIInfo
    {
        PrimaryAIBehaviour behaviour;
    };

    extern AIInfo g_ai_info;

    void InitializeAIInfo();

    struct FindTargetResult
    {
        uint32_t entity_id;
        math::Vector world_position;
    };
    FindTargetResult FindAITargetFromPosition(const math::Vector& world_position);

    bool SeesPlayer(
        const mono::PhysicsSystem* physics_system, const math::Vector& position, const struct PlayerInfo* player_info);
}
