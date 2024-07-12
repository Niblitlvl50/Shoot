
#include "Mission/MissionSystemDrawer.h"
#include "Mission/MissionSystem.h"
#include "FontIds.h"
#include "Hud/MissionStatusUIElement.h"

#include "Math/CriticalDampedSpring.h"
#include "Math/EasingFunctions.h"
#include "Math/Quad.h"
#include "Rendering/RenderSystem.h"
#include "Util/Algorithm.h"

using namespace game;

namespace
{
    constexpr float UI_ELEMENT_WIDTH = 9.0f;
    constexpr float UI_ELEMENT_HEIGHT = 2.0f;
    constexpr float UI_ELEMENT_SPACING = 0.5f;

    constexpr float transition_duration_s = 0.8f;
    constexpr float ease_in_out_time_s = 1.0f;
    constexpr float display_time_s = 3.0f;
    constexpr float ease_out_time_s = ease_in_out_time_s + display_time_s;
}

MissionSystemDrawer::MissionSystemDrawer(const MissionSystem* mission_system)
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
            const MissionTrackerComponent* mission_tracker = m_mission_system->GetComponentById(mission_status_event.mission_id);
            const bool time_based_mission = m_mission_system->IsTimeBasedMission(mission_status_event.mission_id);

            MissionStatusUIElement* ui_element = AddMissionUIElement(mission_status_event.mission_id, time_based_mission);
            ui_element->SetText(mission_tracker->name);
            ui_element->SetDescription(mission_tracker->description);

            break;
        }
        case MissionStatus::Completed:
            CompleteAndRemoveMissionUIElement(mission_status_event.mission_id);
            break;
        case MissionStatus::Failed:
            FailAndRemoveMissionUIElement(mission_status_event.mission_id);
            break;
        }
    }

    UpdateAnimations(update_context);
    CheckForFinishedAnimations(update_context);
}

void MissionSystemDrawer::UpdateAnimations(const mono::UpdateContext& context)
{
    for(MissionStatusData& mission_status_data : m_mission_ui_collection)
    {
        math::Vector current_position = mission_status_data.ui_element->GetPosition();
        math::critical_spring_damper(
            current_position, mission_status_data.current_velocity, mission_status_data.desired_position, math::ZeroVec, 0.15f, context.delta_s);
        mission_status_data.ui_element->SetPosition(current_position);

        if(mission_status_data.time_based_mission)
        {
            const float time_left_s = m_mission_system->GetTimeLeftForMission(mission_status_data.entity_id);
            mission_status_data.ui_element->SetTime(time_left_s);
        }
    }
}

void MissionSystemDrawer::CheckForFinishedAnimations(const mono::UpdateContext& context)
{
    const auto remove_if_timed_out = [&](MissionStatusData& mission_status_data) {
        if(!mission_status_data.delayed_remove)
            return false;

        mission_status_data.time_s -= context.delta_s;

        if(mission_status_data.time_s <= 1.0f && !mission_status_data.initiated_transition_out)
        {
            mission_status_data.desired_position -= math::Vector(10.0f, 0.0f);
            mission_status_data.initiated_transition_out = true;
        }

        const bool time_to_remove = (mission_status_data.time_s <= 0.0f);
        if(time_to_remove)
        {
            RemoveChild(mission_status_data.ui_element);
            delete mission_status_data.ui_element;
        }

        return time_to_remove;
    };

    const bool removed_item = mono::remove_if(m_mission_ui_collection, remove_if_timed_out);
    if(removed_item)
        ReCalculateLayout();
}

MissionStatusUIElement* MissionSystemDrawer::AddMissionUIElement(uint32_t entity_id, bool time_based_mission)
{
    const math::Vector onscreen_position = CalculatePositionForItem(m_mission_ui_collection.size());

    MissionStatusData mission_status_data;
    mission_status_data.entity_id = entity_id;
    mission_status_data.time_based_mission = time_based_mission;

    mission_status_data.desired_position = onscreen_position;
    mission_status_data.current_velocity = math::ZeroVec;

    mission_status_data.ui_element = new MissionStatusUIElement(UI_ELEMENT_WIDTH, UI_ELEMENT_HEIGHT, mono::Color::MakeWithAlpha(mono::Color::DARK_GRAY, 0.25f));
    mission_status_data.ui_element->ShowIcon(false, 0);
    mission_status_data.ui_element->SetPosition(onscreen_position - math::Vector(UI_ELEMENT_WIDTH + 1.0f, 0.0f));

    mission_status_data.delayed_remove = false;
    mission_status_data.initiated_transition_out = false;
    mission_status_data.time_s = 0.0f;

    m_mission_ui_collection.push_back(mission_status_data);
    AddChild(mission_status_data.ui_element);

    return mission_status_data.ui_element;
}

void MissionSystemDrawer::CompleteAndRemoveMissionUIElement(uint32_t entity_id)
{
    const auto find_by_id = [entity_id](const MissionStatusData& mission_status_data) {
        return mission_status_data.entity_id == entity_id;
    };

    MissionStatusData* mission_status_data = mono::find_if(m_mission_ui_collection, find_by_id);
    if(mission_status_data)
    {
        mission_status_data->ui_element->ShowIcon(true, 0);
        mission_status_data->delayed_remove = true;
        mission_status_data->time_s = 3.0f;
    }
}

void MissionSystemDrawer::FailAndRemoveMissionUIElement(uint32_t entity_id)
{
    const auto find_by_id = [entity_id](const MissionStatusData& mission_status_data) {
        return mission_status_data.entity_id == entity_id;
    };

    MissionStatusData* mission_status_data = mono::find_if(m_mission_ui_collection, find_by_id);
    if(mission_status_data)
    {
        mission_status_data->ui_element->ShowIcon(true, 1);
        mission_status_data->delayed_remove = true;
        mission_status_data->time_s = 3.0f;
    }
}

void MissionSystemDrawer::ReCalculateLayout()
{
    for(int index = 0; index < (int)m_mission_ui_collection.size(); ++index)
    {
        MissionStatusData& mission_status_data = m_mission_ui_collection[index];
        mission_status_data.desired_position = CalculatePositionForItem(index);
    }
}

math::Vector MissionSystemDrawer::CalculatePositionForItem(int item_index) const
{
    constexpr float base_offset_x = 0.5f;
    constexpr float base_offset_y = 5.5f;

    return math::Vector(base_offset_x, m_height - base_offset_y) -
        (math::Vector(0.0f, UI_ELEMENT_HEIGHT + UI_ELEMENT_SPACING) * (item_index + 1));
}
