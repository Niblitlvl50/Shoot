
#pragma once

namespace game
{
    enum CollisionCategory
    {
        NONE = 0,
        PLAYER = 1,
        PLAYER_BULLET = 2,
        ENEMY = 4,
        ENEMY_BULLET = 8,
        PROPS = 16,
        STATIC = 32,
    };

    constexpr unsigned int STATIC_MASK =
        CollisionCategory::PLAYER |
        CollisionCategory::PLAYER_BULLET |
        CollisionCategory::ENEMY |
        CollisionCategory::ENEMY_BULLET |
        CollisionCategory::PROPS;

    constexpr unsigned int PLAYER_MASK =
        CollisionCategory::PLAYER |
        CollisionCategory::ENEMY |
        CollisionCategory::ENEMY_BULLET |
        CollisionCategory::PROPS |
        CollisionCategory::STATIC;

    constexpr unsigned int PLAYER_BULLET_MASK =
        CollisionCategory::ENEMY |
        CollisionCategory::PROPS |
        CollisionCategory::STATIC;

    constexpr unsigned int ENEMY_MASK =
        CollisionCategory::PLAYER |
        CollisionCategory::PLAYER_BULLET |
        CollisionCategory::ENEMY |
        CollisionCategory::PROPS |
        CollisionCategory::STATIC;

    constexpr unsigned int ENEMY_BULLET_MASK =
        CollisionCategory::PLAYER |
        CollisionCategory::PROPS |
        CollisionCategory::STATIC;

    constexpr unsigned int PROPS_MASK =
        CollisionCategory::PLAYER |
        CollisionCategory::PLAYER_BULLET |
        CollisionCategory::ENEMY |
        CollisionCategory::ENEMY_BULLET |
        CollisionCategory::PROPS |
        CollisionCategory::STATIC;


    struct FactionPair
    {
        CollisionCategory category;
        uint32_t mask;
    };

    constexpr FactionPair faction_lookup_table[] = {
        { CollisionCategory::STATIC,        STATIC_MASK },
        { CollisionCategory::PLAYER,        PLAYER_MASK },
        { CollisionCategory::PLAYER_BULLET, PLAYER_BULLET_MASK },
        { CollisionCategory::ENEMY,         ENEMY_MASK },
        { CollisionCategory::ENEMY_BULLET,  ENEMY_BULLET_MASK },
        { CollisionCategory::PROPS,         PROPS_MASK },
    };
}
