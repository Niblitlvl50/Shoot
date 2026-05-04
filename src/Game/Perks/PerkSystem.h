
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

        std::vector<PerkDefinition> m_perk_definitions;
    };
}
