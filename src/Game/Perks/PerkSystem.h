
#pragma once

#include "IGameSystem.h"
#include "PerkTypes.h"

#include <memory>
#include <unordered_map>
#include <vector>

namespace game
{
    class WeaponSystem;
    class IWeaponModifier;

    class PerkSystem : public mono::IGameSystem
    {
    public:

        PerkSystem(WeaponSystem* weapon_system);
        const char* Name() const override;
        void Update(const mono::UpdateContext& update_context) override;

        void RollForNewPlayerPerk();
        void RollForNewEnemyPerk();
        float GetCurrentRerollFraction() const;

        const PerkDefinition& GetCurrentPlayerPerk() const;
        const PerkDefinition& GetCurrentEnemyPerk() const;

        IWeaponModifier* GetCurrentEnemyModifier() const;

    private:

        uint32_t RerollPerkAvoidId(uint32_t perk_id_to_avoid);

        void ApplyPerkToPlayers(const PerkDefinition& perk);
        void RemoveCurrentPlayerPerk();

        static IWeaponModifier* CreateModifierForPerk(PerkType type);

        WeaponSystem* m_weapon_system;

        PerkSetup m_perk_setup;
        std::vector<PerkDefinition> m_perk_definitions;

        float m_current_reroll_time = 0.0f;

        uint32_t m_current_player_perk_id = 0;
        uint32_t m_current_enemy_perk_id = 0;

        std::unique_ptr<IWeaponModifier> m_player_modifier;
        std::unique_ptr<IWeaponModifier> m_enemy_modifier;

        std::unordered_map<uint32_t, int> m_player_modifier_slots;
    };
}
