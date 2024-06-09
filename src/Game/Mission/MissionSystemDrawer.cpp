
#include "Mission/MissionSystemDrawer.h"
#include "Mission/MissionSystem.h"
#include "FontIds.h"

#include "Math/EasingFunctions.h"
#include "Math/Quad.h"
#include "Rendering/RenderSystem.h"

using namespace game;

MissionSystemDrawer::MissionSystemDrawer(MissionSystem* mission_system)
    : UIOverlay(50.0f, 50.0f / mono::RenderSystem::GetWindowAspect())
    , m_mission_system(mission_system)
{
    m_mission_name_text = new UITextElement(FontId::RUSSOONE_MEGA, "", mono::FontCentering::DEFAULT_CENTER, mono::Color::GRAY);
    m_mission_name_text->SetPosition(1.0f, m_height - 3.0f);

    m_mission_description_text = new UITextElement(FontId::RUSSOONE_MEDIUM, "", mono::FontCentering::DEFAULT_CENTER, mono::Color::DIM_GRAY);
    m_mission_description_text->SetPosition(1.5f, m_height - 4.5f);

    AddChild(m_mission_name_text);
    AddChild(m_mission_description_text);
}

void MissionSystemDrawer::Update(const mono::UpdateContext& context)
{
    UIOverlay::Update(context);

    const std::vector<MissionTrackerComponent>& mission_status = m_mission_system->GetMissionStatus();

/*
    const RegionDescription& activated_region = m_region_system->GetActivatedRegion();
    if(activated_region.entity_id != m_current_region_entity_id)
    {
        m_region_text->SetText(activated_region.text);
        m_region_subtext->SetText(activated_region.sub_text);
        m_text_timer_s = 0.0f;

        m_current_region_entity_id = activated_region.entity_id;
    }
*/

    m_text_timer_s += context.delta_s;

    constexpr float ease_in_out_time_s = 1.0f;
    constexpr float display_time_s = 3.0f;
    constexpr float ease_out_time_s = ease_in_out_time_s + display_time_s;

    if(m_text_timer_s < ease_in_out_time_s)
    {
        const float alpha = math::EaseInOutCubic(m_text_timer_s, ease_in_out_time_s, 0.0f, 1.0f);
        m_mission_name_text->SetAlpha(alpha);
        m_mission_description_text->SetAlpha(alpha);
    }
    else if(m_text_timer_s > display_time_s && m_text_timer_s < ease_out_time_s)
    {
        const float time_s = m_text_timer_s - display_time_s;
        const float alpha = math::EaseInOutCubic(time_s, 1.0f, 1.0f, -1.0f);
        m_mission_name_text->SetAlpha(alpha);
        m_mission_description_text->SetAlpha(alpha);
    }
}
