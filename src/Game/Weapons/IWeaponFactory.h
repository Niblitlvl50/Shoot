
#pragma once

#include "WeaponTypes.h"
#include "Particle/ParticleFwd.h"
#include <memory>

namespace mono
{
    struct ParticlePool;
}

namespace game
{
    class IWeaponSystem;

    class IWeaponFactory
    {
    public:
        virtual ~IWeaponFactory()
        { }

        virtual std::unique_ptr<IWeaponSystem> CreateWeapon(WeaponType weapon, WeaponFaction faction) = 0;
        virtual std::unique_ptr<IWeaponSystem> CreateWeapon(
            WeaponType weapon, WeaponFaction faction, mono::ParticlePool* pool) = 0;
    };
}
