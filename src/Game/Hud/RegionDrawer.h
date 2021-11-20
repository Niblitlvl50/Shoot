
#pragma once

#include "UIElements.h"

#include <cstdint>
#include <string>
#include <vector>

namespace game
{
    struct RegionDescription
    {
        uint32_t trigger_hash;
        std::string text;
        std::string sub_text;
    };

    std::vector<RegionDescription> ParseRegionConfig(const char* region_config);

    class RegionDrawer : public game::UIOverlay
    {
    public:

        RegionDrawer(const std::vector<game::RegionDescription>& region_descriptions, class TriggerSystem* trigger_system);
        ~RegionDrawer();
        void Update(const mono::UpdateContext& context) override;

    private:

        void HandleRegionTrigger(uint32_t trigger_hash);

        std::vector<game::RegionDescription> m_region_descriptions;
        game::TriggerSystem* m_trigger_system;

        std::vector<uint32_t> m_registred_triggers;
        
        UITextElement* m_region_text;
        UITextElement* m_region_subtext;
        float m_text_timer;
    };
}
