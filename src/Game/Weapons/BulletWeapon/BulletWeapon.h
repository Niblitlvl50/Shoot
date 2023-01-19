
#pragma once

#include "MonoFwd.h"
#include "Weapons/IWeapon.h"
#include "Weapons/WeaponConfiguration.h"

#include "System/Audio.h"
#include "Math/MathFwd.h"

#include <unordered_map>

namespace game
{
    class EntityLogicSystem;

    class Weapon : public IWeapon
    {
    public:

        Weapon(
            uint32_t owner_id,
            const WeaponSetup& weapon_setup,
            const WeaponConfiguration& weapon_config,
            const BulletConfiguration& bullet_config,
            const CollisionConfiguration& collision_config,
            mono::IEntityManager* entity_manager,
            mono::SystemContext* system_context);
        ~Weapon();

        WeaponState Fire(const math::Vector& position, float direction, uint32_t timestamp) override;
        WeaponState Fire(const math::Vector& position, const math::Vector& target, uint32_t timestamp) override;
        void StopFire(uint32_t timestamp) override;
        void Reload(uint32_t timestamp) override;
        WeaponState UpdateWeaponState(uint32_t timestamp) override;
        WeaponState GetWeaponState() const override;
        void AddAmmunition(int amount) override;
        int AmmunitionLeft() const override;
        int MagazineSize() const override;
        int ReloadPercentage() const override;
        WeaponSetup GetWeaponSetup() const override;

    private:

        const uint32_t m_owner_id;
        const WeaponSetup m_weapon_setup;
        const WeaponConfiguration m_weapon_config;
        const BulletConfiguration m_bullet_config;
        const CollisionConfiguration m_collision_config;
        mono::IEntityManager* m_entity_manager;
        uint32_t m_last_fire_timestamp;
        uint32_t m_last_reload_timestamp;
        float m_current_fire_rate;
        int m_ammunition;
        int m_reload_percentage;
        WeaponState m_state;

        audio::ISoundPtr m_fire_sound;
        audio::ISoundPtr m_ooa_sound;
        audio::ISoundPtr m_reload_sound;
        audio::ISoundPtr m_reload_finished_sound;

        mono::TransformSystem* m_transform_system;
        mono::PhysicsSystem* m_physics_system;
        mono::ParticleSystem* m_particle_system;
        EntityLogicSystem* m_logic_system;

        std::unique_ptr<class MuzzleFlash> m_muzzle_flash;
        std::unique_ptr<class BulletTrailEffect> m_bullet_trail;

        std::unordered_map<uint32_t, uint32_t> m_bullet_id_to_callback;
    };
}
