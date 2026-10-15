
#include "RegionDrawer.h"
#include "RegionSystem.h"
#include "FontIds.h"

#include "EntitySystem/Entity.h"
#include "Math/EasingFunctions.h"
#include "Rendering/RenderSystem.h"

#include <algorithm>

using namespace game;

namespace tweak_values
{
    constexpr float ease_in_out_time_s = 0.5f;
    //constexpr float display_time_s = 3.0f;
}

RegionDrawer::RegionDrawer(const RegionSystem* region_system)
    : UIOverlay(50.0f, 50.0f / mono::RenderSystem::GetWindowAspect())
    , m_region_system(region_system)
    , m_text_timer_s(0.0f)
    , m_fade_out_start_s(0.0f)
    , m_fade_out_start_alpha(0.0f)
    , m_alpha(0.0f)
    , m_shown_activation_count(0)
{
    m_region_text = new UITextElement(FontId::RUSSOONE_MEGA, "", mono::Color::GRAY);
    m_region_text->SetAchorPoint(mono::AnchorPoint::BOTTOM_LEFT);
    m_region_text->SetPosition(1.0f, m_height - 3.0f);

    m_region_subtext = new UITextElement(FontId::RUSSOONE_MEDIUM, "", mono::Color::DIM_GRAY);
    m_region_subtext->SetAchorPoint(mono::AnchorPoint::BOTTOM_LEFT);
    m_region_subtext->SetPosition(1.5f, m_height - 4.5f);

    AddChild(m_region_text);
    AddChild(m_region_subtext);
}

void RegionDrawer::Update(const mono::UpdateContext& context)
{
    UIOverlay::Update(context);

    const RegionDescription& activated_region = m_region_system->GetActivatedRegion();
    if(activated_region.activation_count != m_shown_activation_count)
    {
        m_region_text->SetText(activated_region.text);
        m_region_subtext->SetText(activated_region.sub_text);

        m_text_timer_s = 0.0f;
        m_fade_out_start_s = tweak_values::ease_in_out_time_s + activated_region.duration;
        m_fade_out_start_alpha = 1.0f;

        m_shown_activation_count = activated_region.activation_count;
    }

    // Leaving the region fades it out from wherever it is, also while it's still fading in.
    if(activated_region.exited && m_text_timer_s < m_fade_out_start_s)
    {
        m_fade_out_start_s = m_text_timer_s;
        m_fade_out_start_alpha = m_alpha;
    }

    m_text_timer_s += context.delta_s;

    if(m_text_timer_s < m_fade_out_start_s)
    {
        m_alpha = (m_text_timer_s < tweak_values::ease_in_out_time_s) ?
            math::EaseInOutCubic(m_text_timer_s, tweak_values::ease_in_out_time_s, 0.0f, 1.0f) : 1.0f;
    }
    else
    {
        const float fade_time_s = std::min(m_text_timer_s - m_fade_out_start_s, tweak_values::ease_in_out_time_s);
        m_alpha = math::EaseInOutCubic(fade_time_s, tweak_values::ease_in_out_time_s, m_fade_out_start_alpha, -m_fade_out_start_alpha);
    }

    m_region_text->SetAlpha(m_alpha);
    m_region_subtext->SetAlpha(m_alpha);
}
