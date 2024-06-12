
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
    , m_text_timer_s(1000.0f)
{
    m_onscreen_position = math::Vector(0.5f, m_height - 4.0f);
    m_offscreen_position = m_onscreen_position; // - math::Vector(10.0f, 0.0f);

    m_mission_name_text = new UITextElement(FontId::MITR_SMALL, "", mono::FontCentering::DEFAULT_CENTER, mono::Color::GOLDEN_YELLOW);
    m_mission_name_text->SetPosition(0.5f, 1.5f);

    m_mission_description_text = new UITextElement(FontId::MITR_TINY, "", mono::FontCentering::DEFAULT_CENTER, mono::Color::LIGHT_GRAY);
    m_mission_description_text->SetPosition(0.75f, 0.5f);

    m_background = new UISquareElement(10.0f, 2.5f, mono::Color::MakeWithAlpha(mono::Color::DARK_GRAY, 0.25f));
    m_background->SetPosition(m_offscreen_position);

    m_background->AddChild(m_mission_name_text);
    m_background->AddChild(m_mission_description_text);

    AddChild(m_background);
}

void MissionSystemDrawer::Update(const mono::UpdateContext& update_context)
{
    UIOverlay::Update(update_context);

    const std::vector<MissionTrackerComponent>& mission_status = m_mission_system->GetMissionStatus();
    for(const MissionTrackerComponent& mission_tracker : mission_status)
    {
        if(mission_tracker.status == MissionStatus::Active)
        {
            m_mission_name_text->SetText(mission_tracker.name);
            m_mission_description_text->SetText(mission_tracker.description);
            m_text_timer_s = 0.0f;
        }
    }

    m_text_timer_s += update_context.delta_s;

    constexpr float transition_duration_s = 0.8f;
    constexpr float ease_in_out_time_s = 1.0f;
    constexpr float display_time_s = 3.0f;
    constexpr float ease_out_time_s = ease_in_out_time_s + display_time_s;

    math::Vector background_position = m_background->GetPosition();

    if(m_text_timer_s < ease_in_out_time_s)
    {
        background_position.x = math::EaseInOutCubic(m_text_timer_s, transition_duration_s, m_offscreen_position.x, m_onscreen_position.x - m_offscreen_position.x);
    }
    else if(m_text_timer_s > display_time_s && m_text_timer_s < ease_out_time_s)
    {
        const float time_s = m_text_timer_s - display_time_s;
        background_position.x = math::EaseInOutCubic(time_s, transition_duration_s, m_onscreen_position.x, m_offscreen_position.x - m_onscreen_position.x);
    }

    m_background->SetPosition(background_position);
}
