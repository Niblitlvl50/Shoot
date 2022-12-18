
#include "RegionDrawer.h"
#include "RegionSystem.h"

#include "FontIds.h"

#include "Math/EasingFunctions.h"
#include "Rendering/RenderSystem.h"
#include "System/Hash.h"

using namespace game;

RegionDrawer::RegionDrawer(const RegionSystem* region_system)
    : UIOverlay(50.0f, 50.0f / mono::GetWindowAspect())
    , m_region_system(region_system)
    , m_current_region_hash(0)
{
    m_region_text = new UITextElement(FontId::RUSSOONE_MEGA, "", mono::FontCentering::DEFAULT_CENTER, mono::Color::GRAY);
    m_region_text->SetPosition(1.0f, m_height - 3.0f);

    m_region_subtext = new UITextElement(FontId::RUSSOONE_MEDIUM, "", mono::FontCentering::DEFAULT_CENTER, mono::Color::DARK_GRAY);
    m_region_subtext->SetPosition(1.5f, m_height - 4.5f);

    AddChild(m_region_text);
    AddChild(m_region_subtext);
}

void RegionDrawer::Update(const mono::UpdateContext& context)
{
    UIOverlay::Update(context);

    const RegionDescription& activated_region = m_region_system->GetActivatedRegion();
    if(activated_region.trigger_hash != m_current_region_hash)
    {
        m_region_text->SetText(activated_region.text);
        m_region_subtext->SetText(activated_region.sub_text);
        m_text_timer = 0.0f;

        m_current_region_hash = activated_region.trigger_hash;
    }

    m_text_timer += context.delta_s;

    constexpr float ease_in_out_time = 1.0f;
    constexpr float display_time = 3.0f;
    constexpr float ease_out_time = ease_in_out_time + display_time;

    if(m_text_timer < ease_in_out_time)
    {
        const float alpha = math::EaseInOutCubic(m_text_timer, ease_in_out_time, 0.0f, 1.0f);
        m_region_text->SetAlpha(alpha);
        m_region_subtext->SetAlpha(alpha);
    }
    else if(m_text_timer > display_time && m_text_timer < ease_out_time)
    {
        const float time = m_text_timer - display_time;
        const float alpha = math::EaseInOutCubic(time, 1.0f, 1.0f, -1.0f);
        m_region_text->SetAlpha(alpha);
        m_region_subtext->SetAlpha(alpha);
    }
}

