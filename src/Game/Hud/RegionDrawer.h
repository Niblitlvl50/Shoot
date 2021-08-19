
#pragma once

#include "UIElements.h"

namespace game
{
    class RegionDrawer : public game::UIOverlay
    {
    public:

        RegionDrawer(class TriggerSystem* trigger_system);
        ~RegionDrawer();
        void Update(const mono::UpdateContext& context) override;

    private:

        void HandleRegionTrigger(uint32_t trigger_hash);

        game::TriggerSystem* m_trigger_system;
        UITextElement* m_region_text;
        mono::Color::RGBA m_color;

        uint32_t m_enter_death_valley_id;
        uint32_t m_enter_oak_forest_id;

        float m_text_timer;
    };
}
