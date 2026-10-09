
#pragma once

#include "MonoFwd.h"
#include "Math/Vector.h"
#include <cstdint>

namespace game
{
    // A dust puff that rises, used when cargo is loaded into or dropped off from a train car.
    class CargoTransferEffect
    {
    public:

        CargoTransferEffect(mono::ParticleSystem* particle_system, mono::IEntityManager* entity_system);
        ~CargoTransferEffect();
        void EmitAt(const math::Vector& position);

    private:
        mono::ParticleSystem* m_particle_system;
        mono::IEntityManager* m_entity_system;
        uint32_t m_particle_entity;
    };
}
