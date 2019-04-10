
#pragma once

#include "IWeaponSystem.h"
#include "WeaponConfiguration.h"

#include "MonoPtrFwd.h"
#include "Math/MathFwd.h"

class IEntityManager;

namespace mono
{
    class PhysicsSystem;
}

namespace game
{
    class EntityLogicSystem;

    class Weapon : public IWeaponSystem
    {
    public:

        Weapon(const WeaponConfiguration& config, IEntityManager* entity_manager, mono::SystemContext* system_context);

        WeaponFireResult Fire(const math::Vector& position, float direction) override;
        int AmmunitionLeft() const override;
        int MagazineSize() const override;
        void Reload() override;

    private:

        const WeaponConfiguration m_weapon_config;
        IEntityManager* m_entity_manager;
        uint32_t m_last_fire_timestamp;
        float m_current_fire_rate;
        int m_ammunition;

        mono::ISoundPtr m_fire_sound;
        mono::ISoundPtr m_ooa_sound;
        mono::ISoundPtr m_reload_sound;

        mono::PhysicsSystem* m_physics_system;
        EntityLogicSystem* m_logic_system;
    };
}
