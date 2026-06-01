
#pragma once

#include "IGameSystem.h"
#include "PerkTypes.h"

#include <vector>

namespace game
{
    class PerkSystem : public mono::IGameSystem
    {
    public:

        PerkSystem();
        const char* Name() const;
        void Update(const mono::UpdateContext& update_context) override;

        void RollForNewPlayerPerk();
        void RollForNewEnemyPerk();
        float GetCurrentRerollFraction() const; 

        const PerkDefinition& GetCurrentPlayerPerk() const;
        const PerkDefinition& GetCurrentEnemyPerk() const;

    private:

        uint32_t RerollPerkAvoidId(uint32_t perk_id_to_avoid);

        PerkSetup m_perk_setup;
        std::vector<PerkDefinition> m_perk_definitions;

        float m_current_reroll_time = 0.0f;

        uint32_t m_current_player_perk_id = 0;
        uint32_t m_current_enemy_perk_id = 0;
    };
}
