
#pragma once

#include <cstdint>
#include <string>

namespace game
{
    enum class PerkType : uint32_t
    {
        None = 0,
        IncreasedDamage,
        IncreasedFireRate,
        IncreasedBulletVelocity,
        IncreasedBulletSpread,
        IncreasedBulletRange,
        IncreasedMagazineSize,
        InfiniteAmmo,
        PiercingBullets,
        ExplosiveBullets,
        HomingBullets,
        RicochetBullets,
        FireBullets,
        IceBullets,
        ElectricBullets,
        VampiricBullets,
        TimeSlowBullets,

        Count
    };

    struct PerkDefinition
    {
        uint32_t id;
        std::string name;
        std::string description;
        uint32_t icon_sprite_id;
    };
}
