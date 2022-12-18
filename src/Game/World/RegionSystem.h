
#pragma once

#include "IGameSystem.h"

#include <vector>
#include <cstdint>
#include <string>

namespace game
{
    struct RegionDescription
    {
        uint32_t trigger_hash;
        std::string text;
        std::string sub_text;
    };

    class RegionSystem : public mono::IGameSystem
    {
    public:

        RegionSystem(class TriggerSystem* trigger_system);

        uint32_t Id() const override;
        const char* Name() const override;
        void Reset() override;
        void Destroy() override;
        void Update(const mono::UpdateContext& update_context) override;

        const RegionDescription& GetActivatedRegion() const;

    private:
        void HandleRegionTrigger(uint32_t trigger_hash);

        game::TriggerSystem* m_trigger_system;
        std::vector<game::RegionDescription> m_region_descriptions;
        std::vector<uint32_t> m_registred_triggers;
        game::RegionDescription m_activated_region;
    };
}
