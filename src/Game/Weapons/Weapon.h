
#pragma once

#include "IWeaponSystem.h"
#include "WeaponConfiguration.h"

#include "MonoPtrFwd.h"
#include "Math/MathFwd.h"

class IEntityManager;

namespace mono
{
    class PhysicsSystem;
    class ParticleSystem;
}

namespace game
{
    class EntityLogicSystem;

    class Weapon : public IWeaponSystem
    {
    public:

        Weapon(const WeaponConfiguration& config, IEntityManager* entity_manager, mono::SystemContext* system_context);
        ~Weapon();

        WeaponFireResult Fire(const math::Vector& position, float direction, uint32_t timestamp) override;
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

        mono::TransformSystem* m_transform_system;
        mono::PhysicsSystem* m_physics_system;
        mono::ParticleSystem* m_particle_system;
        EntityLogicSystem* m_logic_system;

        std::unique_ptr<class MuzzleFlash> m_muzzle_flash;
    };
}
