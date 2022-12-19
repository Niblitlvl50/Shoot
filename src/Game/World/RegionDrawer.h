
#pragma once

#include "Hud/UIElements.h"
#include <cstdint>

namespace game
{
    class RegionDrawer : public game::UIOverlay
    {
    public:

        RegionDrawer(const class RegionSystem* region_system);
        void Update(const mono::UpdateContext& context) override;

    private:

        const RegionSystem* m_region_system;

        UITextElement* m_region_text;
        UITextElement* m_region_subtext;
        float m_text_timer_s;

        uint32_t m_current_region_entity_id;
    };
}
