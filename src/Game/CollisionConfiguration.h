
#pragma once

#include <cstdint>
#include <vector>

namespace game
{
    enum CollisionCategory : uint32_t
    {
        NONE = 0,
        PLAYER = 1,
        PLAYER_BULLET = 2,
        ENEMY = 4,
        ENEMY_BULLET = 8,
        PROPS = 16,
        PICKUPS = 32,
        STATIC = 64,
        CC_ALL = (~(uint32_t)0)
    };

    constexpr uint32_t STATIC_MASK =
        CollisionCategory::PLAYER |
        CollisionCategory::PLAYER_BULLET |
        CollisionCategory::ENEMY |
        CollisionCategory::ENEMY_BULLET |
        CollisionCategory::PROPS;

    constexpr uint32_t PLAYER_MASK =
        CollisionCategory::PLAYER |
        CollisionCategory::ENEMY |
        CollisionCategory::ENEMY_BULLET |
        CollisionCategory::PROPS |
        CollisionCategory::PICKUPS |
        CollisionCategory::STATIC;

    constexpr uint32_t PLAYER_BULLET_MASK =
        CollisionCategory::ENEMY |
        CollisionCategory::PROPS |
        CollisionCategory::STATIC;

    constexpr uint32_t ENEMY_MASK =
        CollisionCategory::PLAYER |
        CollisionCategory::PLAYER_BULLET |
        CollisionCategory::ENEMY |
        CollisionCategory::PROPS |
        CollisionCategory::STATIC;

    constexpr uint32_t ENEMY_BULLET_MASK =
        CollisionCategory::PLAYER |
        CollisionCategory::PROPS |
        CollisionCategory::STATIC;

    constexpr uint32_t PROPS_MASK =
        CollisionCategory::PLAYER |
        CollisionCategory::PLAYER_BULLET |
        CollisionCategory::ENEMY |
        CollisionCategory::ENEMY_BULLET |
        CollisionCategory::PROPS |
        CollisionCategory::STATIC;

    constexpr uint32_t PICKUPS_MASK =
        CollisionCategory::PLAYER;


    struct FactionPair
    {
        CollisionCategory category;
        uint32_t mask;
    };

    constexpr FactionPair g_faction_lookup_table[] = {
        //{ CollisionCategory::NONE,          0 },
        { CollisionCategory::PLAYER,        PLAYER_MASK },
        { CollisionCategory::PLAYER_BULLET, PLAYER_BULLET_MASK },
        { CollisionCategory::ENEMY,         ENEMY_MASK },
        { CollisionCategory::ENEMY_BULLET,  ENEMY_BULLET_MASK },
        { CollisionCategory::PROPS,         PROPS_MASK },
        { CollisionCategory::PICKUPS,       PICKUPS_MASK },
        { CollisionCategory::STATIC,        STATIC_MASK },
    };

    static const std::vector<uint32_t> all_collision_categories = {
        CollisionCategory::PLAYER,
        CollisionCategory::PLAYER_BULLET,
        CollisionCategory::ENEMY,
        CollisionCategory::ENEMY_BULLET,
        CollisionCategory::PROPS,
        CollisionCategory::PICKUPS,
        CollisionCategory::STATIC,
        //CollisionCategory::ALL,
    };

    inline const char* CollisionCategoryToString(uint32_t category)
    {
        switch(category)
        {
        case CollisionCategory::NONE:
            return "None";
        case CollisionCategory::PLAYER:
            return "Player";
        case CollisionCategory::PLAYER_BULLET:
            return "Player Bullet";
        case CollisionCategory::ENEMY:
            return "Enemy";
        case CollisionCategory::ENEMY_BULLET:
            return "Enemy Bullet";
        case CollisionCategory::PROPS:
            return "Props";
        case CollisionCategory::PICKUPS:
            return "Pickups";
        case CollisionCategory::STATIC:
            return "Static";
        case CollisionCategory::CC_ALL:
            return "All";
        };

        return "Unknown";
    };
}
