
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

    struct PerkSetup
    {
        float reroll_duration;
    };

    struct PerkDefinition
    {
        uint32_t id;
        std::string name;
        std::string description;
        std::string icon_sprite_file;
    };
}
