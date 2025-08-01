
#pragma once

#include "MonoFwd.h"
#include "IGameSystem.h"

#include "IWeaponModifier.h"
#include "WeaponTypes.h"
#include "WeaponConfiguration.h"
#include "WeaponEntityFactory.h"

#include <unordered_map>
#include <memory>
#include <cstdint>
#include <string>

namespace game
{
    using IWeaponPtr = std::unique_ptr<class IWeapon>;
    using WeaponModifierList = std::vector<IWeaponModifier*>;

    struct WeaponLevelExperience
    {
        int level;
        int current_level_experience;
        int next_level_experience;
    };
    
    class WeaponSystem : public mono::IGameSystem
    {
    public:

        WeaponSystem(
            mono::TransformSystem* transform_system,
            mono::SpriteSystem* sprite_system,
            mono::PhysicsSystem* physics_system,
            mono::IEntityManager* entity_manager,
            class DamageSystem* damage_system,
            class CameraSystem* camera_system,
            class EntityLogicSystem* logic_system,
            class TargetSystem* target_system,
            mono::SystemContext* system_context);

        const char* Name() const override;
        void Begin() override;
        void Reset() override;

        void Update(const mono::UpdateContext& update_context) override;

        void SetWeaponLoadout(
            uint32_t entity_id, const std::string& primary, const std::string& secondary, const std::string& tertiary);

        void SetWeaponLoadout(
            uint32_t entity_id, const WeaponSetup& primary, const WeaponSetup& secondary, const WeaponSetup& tertiary);

        IWeaponPtr CreatePrimaryWeapon(uint32_t entity_id, WeaponFaction faction);
        IWeaponPtr CreateSecondaryWeapon(uint32_t entity_id, WeaponFaction faction);
        IWeaponPtr CreateTertiaryWeapon(uint32_t entity_id, WeaponFaction faction);

        IWeaponPtr CreateWeapon(WeaponSetup setup, WeaponFaction faction, uint32_t owner_id);

        std::vector<WeaponBulletCombination> GetAllWeaponCombinations() const;
        const WeaponBulletCombination& GetWeaponBulletConfigForHash(uint32_t weapon_hash) const;

        uint32_t SpawnWeaponPickupAt(const WeaponSetup& setup, const math::Vector& world_position);

        int AddModifierForId(uint32_t id, IWeaponModifier* weapon_modifier);
        int AddModifierForIdWithDuration(uint32_t id, float duration_s, IWeaponModifier* weapon_modifier);
        int AddModifierForIdAndWeapon(uint32_t id, uint32_t weapon_identifier_hash, IWeaponModifier* weapon_modifier);

        WeaponLevelExperience GetWeaponLevelForExperience(uint32_t weapon_identifier_hash, int weapon_experience);
        void ApplyModifiersForWeaponLevel(uint32_t entity_id, uint32_t weapon_identifier_hash, int weapon_experience);
        float GetDurationFractionForModifierOnEntity(uint32_t entity_id, uint32_t modifier_id) const;
        WeaponModifierList GetWeaponModifiersForEntity(uint32_t entity_id) const;
        WeaponModifierList GetWeaponModifiersForIdAndWeapon(uint32_t id, uint32_t weapon_identifier_hash) const;

    private:

        mono::TransformSystem* m_transform_system;
        mono::IEntityManager* m_entity_manager;
        mono::SystemContext* m_system_context;
        game::WeaponEntityFactory m_weapon_entity_factory;

        WeaponConfig m_weapon_configuration;
        ModifiersConfig m_modifier_configuration;
        BulletImpactCallback m_standard_collision;
        std::unordered_map<uint32_t, BulletImpactCallback> m_bullet_callbacks;

        struct WeaponLoadoutComponent
        {
            WeaponSetup primary;
            WeaponSetup secondary;
            WeaponSetup tertiary;
        };
        std::unordered_map<uint32_t, WeaponLoadoutComponent> m_weapon_loadout;

        struct WeaponModifierDuration
        {
            float duration;
            float duration_counter;
        };

        struct WeaponModifierContext
        {
            WeaponModifierList modifiers;
            std::vector<WeaponModifierDuration> durations;
            std::vector<uint32_t> ids;
        };

        std::unordered_map<uint32_t, WeaponModifierContext> m_weapon_modifiers;
        std::unordered_map<uint32_t, WeaponModifierContext> m_weapon_level_modifiers;

        uint32_t m_modifier_id;

        class DamageEffect* m_damage_effect = nullptr;
        class ImpactEffect* m_impact_effect = nullptr;
    };
}
