
#pragma once

#include "Hud/UIElements.h"

#include <cstdint>
#include <string>

namespace game
{
    class RegionSystem;

    class RegionDrawer : public game::UIOverlay
    {
    public:

        RegionDrawer(const RegionSystem* region_system);
        void Update(const mono::UpdateContext& context) override;

    private:

        const RegionSystem* m_region_system;

        UITextElement* m_region_text;
        UITextElement* m_region_subtext;
        float m_text_timer;

        uint32_t m_current_region_hash;
    };
}
