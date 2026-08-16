
#pragma once

#include "IGameSystem.h"
#include "PerkTypes.h"

#include <array>
#include <memory>
#include <unordered_map>
#include <vector>

namespace game
{
    class WeaponSystem;
    class DamageSystem;
    class IWeaponModifier;
    class IDamageModifier;

    struct EnemyPerkModifiers
    {
        std::unique_ptr<IWeaponModifier> weapon_modifier;
        std::unique_ptr<IDamageModifier> damage_modifier;
    };

    class PerkSystem : public mono::IGameSystem
    {
    public:

        PerkSystem(WeaponSystem* weapon_system, DamageSystem* damage_system);
        const char* Name() const override;
        void Update(const mono::UpdateContext& update_context) override;

        void RollForNewPlayerPerk();
        void RollForNewEnemyPerk();
        float GetCurrentRerollFraction() const;

        const PerkDefinition& GetCurrentPlayerPerk() const;
        const PerkDefinition& GetCurrentEnemyPerk() const;

        EnemyPerkModifiers GetCurrentEnemyPerkModifiers() const;

    private:

        uint32_t RerollPerkAvoidId(uint32_t current_id) const;
        std::array<uint32_t, 2> RerollPerkCombo(const std::array<uint32_t, 2>& current_ids) const;
        PerkDefinition MakeComboDisplay(uint32_t id_a, uint32_t id_b) const;

        void ApplyPerkToPlayers(uint32_t perk_id);
        void RemoveCurrentPlayerPerk();

        static IWeaponModifier* CreateModifierForPerk(PerkType type);
        static IDamageModifier* CreateDamageModifierForPerk(PerkType type);

        WeaponSystem* m_weapon_system;
        DamageSystem* m_damage_system;

        PerkSetup m_perk_setup;
        std::vector<PerkDefinition> m_perk_definitions;
        std::vector<uint32_t> m_weapon_perk_indices;
        std::vector<uint32_t> m_damage_perk_indices;

        float m_current_reroll_time = 0.0f;

        uint32_t m_current_player_perk_id = 0;
        std::array<uint32_t, 2> m_current_enemy_perk_ids = { 0, 1 };

        PerkDefinition m_enemy_combo_display;


        std::unordered_map<uint32_t, int> m_player_modifier_slots;
        std::unordered_map<uint32_t, int> m_player_damage_modifier_slots;
    };
}
