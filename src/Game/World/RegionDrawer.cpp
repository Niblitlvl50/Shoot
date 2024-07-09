
#include "RegionDrawer.h"
#include "RegionSystem.h"
#include "FontIds.h"

#include "EntitySystem/Entity.h"
#include "Math/EasingFunctions.h"
#include "Rendering/RenderSystem.h"

using namespace game;

RegionDrawer::RegionDrawer(const RegionSystem* region_system)
    : UIOverlay(50.0f, 50.0f / mono::RenderSystem::GetWindowAspect())
    , m_region_system(region_system)
    , m_current_region_entity_id(mono::INVALID_ID)
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
    if(activated_region.entity_id != m_current_region_entity_id)
    {
        m_region_text->SetText(activated_region.text);
        m_region_subtext->SetText(activated_region.sub_text);
        m_text_timer_s = 0.0f;

        m_current_region_entity_id = activated_region.entity_id;
    }

    m_text_timer_s += context.delta_s;

    constexpr float ease_in_out_time_s = 1.0f;
    constexpr float display_time_s = 3.0f;
    constexpr float ease_out_time_s = ease_in_out_time_s + display_time_s;

    if(m_text_timer_s < ease_in_out_time_s)
    {
        const float alpha = math::EaseInOutCubic(m_text_timer_s, ease_in_out_time_s, 0.0f, 1.0f);
        m_region_text->SetAlpha(alpha);
        m_region_subtext->SetAlpha(alpha);
    }
    else if(m_text_timer_s > display_time_s && m_text_timer_s < ease_out_time_s)
    {
        const float time_s = m_text_timer_s - display_time_s;
        const float alpha = math::EaseInOutCubic(time_s, 1.0f, 1.0f, -1.0f);
        m_region_text->SetAlpha(alpha);
        m_region_subtext->SetAlpha(alpha);
    }
}

