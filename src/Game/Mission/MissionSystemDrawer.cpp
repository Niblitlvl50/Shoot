
#include "Mission/MissionSystemDrawer.h"
#include "Mission/MissionSystem.h"
#include "FontIds.h"
#include "Hud/MissionStatusUIElement.h"

#include "Math/EasingFunctions.h"
#include "Math/Quad.h"
#include "Rendering/RenderSystem.h"
#include "Util/Algorithm.h"

using namespace game;

namespace
{
    constexpr float UI_ELEMENT_WIDTH = 10.0f;
    constexpr float UI_ELEMENT_HEIGHT = 2.5f;
    constexpr float UI_ELEMENT_SPACING = 1.0f;

    constexpr float transition_duration_s = 0.8f;
    constexpr float ease_in_out_time_s = 1.0f;
    constexpr float display_time_s = 3.0f;
    constexpr float ease_out_time_s = ease_in_out_time_s + display_time_s;
}

MissionSystemDrawer::MissionSystemDrawer(MissionSystem* mission_system)
    : UIOverlay(50.0f, 50.0f / mono::RenderSystem::GetWindowAspect())
    , m_mission_system(mission_system)
{ }

void MissionSystemDrawer::Update(const mono::UpdateContext& update_context)
{
    UIOverlay::Update(update_context);

    const std::vector<MissionStatusEvent>& mission_status = m_mission_system->GetMissionStatusEvents();
    for(const MissionStatusEvent& mission_status_event : mission_status)
    {
        switch(mission_status_event.status)
        {
        case MissionStatus::Inactive:
            break;
        case MissionStatus::Active:
        {
            MissionTrackerComponent* mission_tracker = m_mission_system->GetComponentById(mission_status_event.mission_id);

            MissionStatusUIElement* ui_element = AddMissionUIElement(mission_status_event.mission_id);
            ui_element->SetText(mission_tracker->name);
            ui_element->SetDescription(mission_tracker->description);

            break;
        }
        case MissionStatus::Completed:
        case MissionStatus::Failed:
            RemoveMissionUIElement(mission_status_event.mission_id);
            break;
        }
    }

    if(!mission_status.empty())
        ReCalculateLayout();
}

MissionStatusUIElement* MissionSystemDrawer::AddMissionUIElement(uint32_t entity_id)
{
    MissionStatusData mission_status_data;
    mission_status_data.entity_id = entity_id;
    mission_status_data.timer_s = 0.0f;
    mission_status_data.onscreen_position = math::ZeroVec;
    mission_status_data.offscreen_position = math::ZeroVec;
    mission_status_data.ui_element = new MissionStatusUIElement(UI_ELEMENT_WIDTH, UI_ELEMENT_HEIGHT, mono::Color::MakeWithAlpha(mono::Color::DARK_GRAY, 0.25f));

    m_mission_ui_collection.push_back(mission_status_data);

    AddChild(mission_status_data.ui_element);

    return mission_status_data.ui_element;
}

void MissionSystemDrawer::RemoveMissionUIElement(uint32_t entity_id)
{
    const auto find_by_id = [this, entity_id](const MissionStatusData& mission_status_data) {
        const bool this_is_the_one = (mission_status_data.entity_id == entity_id);
        if(this_is_the_one)
        {
            RemoveChild(mission_status_data.ui_element);
            delete mission_status_data.ui_element;
        }

        return this_is_the_one;
    };
    mono::remove_if(m_mission_ui_collection, find_by_id);
}

void MissionSystemDrawer::ReCalculateLayout()
{
    constexpr float base_offset_x = 0.5f;
    constexpr float base_offset_y = 0.5f;

    math::Vector current_position = math::Vector(base_offset_x, m_height - base_offset_y);

    for(MissionStatusData& mission_status_data : m_mission_ui_collection)
    {
        current_position.y -= (UI_ELEMENT_HEIGHT + UI_ELEMENT_SPACING);

        mission_status_data.onscreen_position = current_position;
        mission_status_data.offscreen_position = current_position - math::Vector(0.0f, UI_ELEMENT_WIDTH + 1.0f);

        mission_status_data.ui_element->SetPosition(mission_status_data.onscreen_position);
    }
}

/*
    m_text_timer_s += update_context.delta_s;

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
*/