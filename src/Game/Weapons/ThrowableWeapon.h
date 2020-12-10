
#pragma once

#include "IWeapon.h"
#include "WeaponConfiguration.h"

#include "MonoPtrFwd.h"

namespace game
{
    class ThrowableWeapon : public IWeapon
    {
    public:

        ThrowableWeapon(const ThrowableWeaponConfig& config, mono::IEntityManager* entity_manager, mono::SystemContext* system_context);

        WeaponState Fire(const math::Vector& position, float direction, uint32_t timestamp) override;
        WeaponState Fire(const math::Vector& position, const math::Vector& target, uint32_t timestamp) override;
        void Reload(uint32_t timestamp) override;
        int AmmunitionLeft() const override;
        int MagazineSize() const override;
        WeaponState GetState() const override;

    private:

        const ThrowableWeaponConfig m_config;
        mono::IEntityManager* m_entity_manager;
        mono::TransformSystem* m_transform_system;
        mono::SpriteSystem* m_sprite_system;
        mono::ParticleSystem* m_particle_system;
        class EntityLogicSystem* m_logic_system;

        uint32_t m_last_fire_timestamp;
        uint32_t m_last_reload_timestamp;
        int m_ammunition;
        WeaponState m_state;
    };
}
