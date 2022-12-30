
#pragma once

#include "MonoFwd.h"
#include "IGameSystem.h"

#include "WeaponTypes.h"
#include "WeaponConfiguration.h"

#include <unordered_map>
#include <memory>
#include <cstdint>
#include <string>

namespace game
{
    using IWeaponPtr = std::unique_ptr<class IWeapon>;
    
    class WeaponSystem : public mono::IGameSystem
    {
    public:

        WeaponSystem(
            mono::TransformSystem* transform_system,
            mono::SpriteSystem* sprite_system,
            mono::PhysicsSystem* physics_system,
            class DamageSystem* damage_system,
            class CameraSystem* camera_system,
            mono::IEntityManager* entity_manager,
            mono::SystemContext* system_context);

        const char* Name() const override;
        void Begin() override;
        void Reset() override;

        void Update(const mono::UpdateContext& update_context) override;

        void SetWeaponLoadout(
            uint32_t entity_id, const std::string& primary, const std::string& secondary, const std::string& tertiary);

        IWeaponPtr CreatePrimaryWeapon(uint32_t entity_id, WeaponFaction faction);
        IWeaponPtr CreateSecondaryWeapon(uint32_t entity_id, WeaponFaction faction);
        IWeaponPtr CreateTertiaryWeapon(uint32_t entity_id, WeaponFaction faction);

        IWeaponPtr CreateWeapon(WeaponSetup setup, WeaponFaction faction, uint32_t owner_id);
        IWeaponPtr CreateWeapon(const char* weapon_name, WeaponFaction faction, uint32_t owner_id);

        std::vector<WeaponBulletCombination> GetAllWeaponCombinations() const;

    private:

        IWeaponPtr CreateThrowableWeapon(WeaponSetup setup, WeaponFaction faction, uint32_t owner_id);

        mono::IEntityManager* m_entity_manager;
        mono::SystemContext* m_system_context;

        WeaponConfig m_weapon_configuration;
        BulletImpactCallback m_standard_collision;
        std::unordered_map<uint32_t, BulletImpactCallback> m_bullet_callbacks;

        struct WeaponLoadoutComponent
        {
            std::string primary_name;
            std::string secondary_name;
            std::string tertiary_name;
        };

        std::unordered_map<uint32_t, WeaponLoadoutComponent> m_weapon_loadout;

        class DamageEffect* m_damage_effect = nullptr;
        class ImpactEffect* m_impact_effect = nullptr;
    };
}
