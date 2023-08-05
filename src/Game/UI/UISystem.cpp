
#include "UISystem.h"
#include "Camera/ICamera.h"
#include "GameCamera/CameraSystem.h"
#include "TriggerSystem/TriggerSystem.h"

#include "Debug/GameDebug.h"
#include "Debug/IDebugDrawer.h"
#include "Rendering/Color.h"

#include "EntitySystem/Entity.h"
#include "Input/InputSystem.h"
#include "TransformSystem/TransformSystem.h"
#include "Util/Algorithm.h"

#include <algorithm>
#include <cmath>

namespace
{
    constexpr int INVALID_UI_INDEX = -1;
}

using namespace game;

UISystem::UISystem(
    mono::InputSystem* input_system,
    mono::TransformSystem* transform_system,
    CameraSystem* camera_system,
    TriggerSystem* trigger_system)
    : m_input_system(input_system)
    , m_transform_system(transform_system)
    , m_camera_system(camera_system)
    , m_trigger_system(trigger_system)
    , m_clicked_this_frame(false)
    , m_button_left(false)
    , m_button_right(false)
    , m_button_up(false)
    , m_button_down(false)
    , m_button_push_this_frame(false)
{
    m_input_context = m_input_system->CreateContext(0, mono::InputContextBehaviour::ConsumeAll, "UISystem");

    m_input_context->enabled = false;
    m_input_context->mouse_input = this;
    m_input_context->controller_input = this;
}

UISystem::~UISystem()
{
}

const char* UISystem::Name() const
{
    return "uisystem";
}

void UISystem::Begin()
{
    m_active_item_index = INVALID_UI_INDEX;
}

void UISystem::Destroy()
{
    m_input_system->ReleaseContext(m_input_context);
}

void UISystem::Update(const mono::UpdateContext& update_context)
{
    const int last_active_item_index = m_active_item_index;
    m_active_item_index = INVALID_UI_INDEX;

    if(!m_input_context->enabled)
        return;

    const mono::InputContextType active_input_type = m_input_context->most_recent_input;
    switch(active_input_type)
    {
    case mono::InputContextType::Mouse:
    {
        for(uint32_t index = 0; index < m_items.size(); ++index)
        {
            const UIItem& ui_item = m_items[index];
            if(ui_item.state != UIItemState::Enabled)
                continue;

            const math::Quad& world_bb = m_transform_system->GetWorldBoundingBox(ui_item.entity_id);
            const bool inside_quad = math::PointInsideQuad(m_mouse_world_position, world_bb);
            if(inside_quad)
            {
                m_active_item_index = index;
                break;
            }
        }

        break;
    }
    case mono::InputContextType::Controller:
    {
        if(m_button_left || m_button_up || m_button_right || m_button_down)
        {
            if(last_active_item_index == INVALID_UI_INDEX)
            {
                for(uint32_t index = 0; index < m_items.size(); ++index)
                {
                    const UIItem& item = m_items[index];
                    if(item.state == UIItemState::Enabled)
                    {
                        m_active_item_index = index;
                        break;
                    }
                }
            }
            else
            {
                const UIItem& last_active_item = m_items[last_active_item_index];
                const UINavigationSetup& navigation_setup = last_active_item.navigation_setup;

                uint32_t new_item_entity_id = mono::INVALID_ID;
                if(m_button_left)
                    new_item_entity_id = navigation_setup.left_entity_id;
                else if(m_button_right)
                    new_item_entity_id = navigation_setup.right_entity_id;
                else if(m_button_up)
                    new_item_entity_id = navigation_setup.above_entity_id;
                else if(m_button_down)
                    new_item_entity_id = navigation_setup.below_entity_id;

                if(new_item_entity_id != mono::INVALID_ID)
                {
                    const uint32_t entity_id = new_item_entity_id;

                    for(uint32_t index = 0; index < m_items.size(); ++index)
                    {
                        const UIItem& item = m_items[index];
                        if(item.state == UIItemState::Enabled && item.entity_id == entity_id)
                        {
                            m_active_item_index = index;
                            break;
                        }
                    }
                }
                else
                {
                    // Keep last active as active, only if controller is last used input.
                    m_active_item_index = last_active_item_index;
                }
            }
        }
        else
        {
            // Keep last active as active, only if controller is last used input.
            m_active_item_index = last_active_item_index;
        }

        break;
    }
    default:
        break;
    }

    if(game::g_draw_debug_uisystem)
        DrawDebug(update_context);

    if((m_clicked_this_frame || m_button_push_this_frame) && m_active_item_index != INVALID_UI_INDEX)
    {
        const UIItem& active_item = m_items[m_active_item_index];
        m_trigger_system->EmitTrigger(active_item.on_click_hash);

        const auto it = m_uiitem_callbacks.find(active_item.entity_id);
        if(it != m_uiitem_callbacks.end())
        {
            for(const UIItemCallback& callback : it->second)
            {
                if(callback)
                    callback(active_item.entity_id);
            }
        }
    }

    m_clicked_this_frame = false;

    m_button_left = false;
    m_button_right = false;
    m_button_up = false;
    m_button_down = false;
    m_button_push_this_frame = false;
}

bool UISystem::UpdateInPause() const
{
    return true;
}

void UISystem::Enable()
{
    m_input_system->ResetAllInput();
    m_input_system->EnableContext(m_input_context);
}

void UISystem::Disable()
{
    m_input_system->DisableContext(m_input_context);
}

bool UISystem::IsEnabled() const
{
    return m_input_context->enabled;
}

void UISystem::SetItemGroupState(int group_id, UIItemState new_state, bool exclusive_state)
{
    for(int index = 0; index < (int)m_items.size(); ++index)
    {
        UIItem& item = m_items[index];

        if(item.group_id == group_id)
            item.state = new_state;
        else if(exclusive_state)
        {
            item.state = UIItemState::Disabled;
            if(index == m_active_item_index)
                m_active_item_index = INVALID_UI_INDEX;
        }
    }
}

void UISystem::AllocateUIItem(uint32_t entity_id)
{
    UIItem item;
    item.on_click_hash = mono::INVALID_ID;
    item.group_id = 0;
    item.entity_id = entity_id;
    item.state = UIItemState::Enabled;

    m_items.push_back(std::move(item));
}

void UISystem::ReleaseUIItem(uint32_t entity_id)
{
    const auto remove_by_entity_id = [entity_id](const UIItem& item) {
        return item.entity_id == entity_id;
    };
    mono::remove_if(m_items, remove_by_entity_id);

    m_uiitem_callbacks.erase(entity_id);
}

void UISystem::UpdateUIItem(
    uint32_t entity_id, uint32_t on_click_hash, int group_id, UIItemState state, const UINavigationSetup& navigation_setup)
{
    const auto find_by_entity_id = [entity_id](const UIItem& item) {
        return item.entity_id == entity_id;
    };

    auto it = std::find_if(m_items.begin(), m_items.end(), find_by_entity_id);
    if(it != m_items.end())
    {
        it->on_click_hash = on_click_hash;
        it->group_id = group_id;
        it->state = state;
        it->navigation_setup = navigation_setup;
    }
}

void UISystem::AllocateUISetGroupState(uint32_t entity_id)
{
    UISetGroupState group_state;
    group_state.entity_id = entity_id;
    group_state.trigger_callback_id = mono::INVALID_ID;

    m_set_group_states.push_back(group_state);
}

void UISystem::ReleaseUISetGroupState(uint32_t entity_id)
{
    const auto remove_by_entity_id = [this, entity_id](UISetGroupState& item) {
        const bool found_item = item.entity_id == entity_id;
        if(found_item)
        {
            m_trigger_system->RemoveTriggerCallback(item.trigger_hash, item.trigger_callback_id, entity_id);
            item.trigger_callback_id = mono::INVALID_ID;
        }

        return found_item;
    };
    mono::remove_if(m_set_group_states, remove_by_entity_id);
}

void UISystem::UpdateUISetGroupState(uint32_t entity_id, int group_id, UIItemState state, uint32_t trigger_hash)
{
    const auto find_by_entity_id = [entity_id](const UISetGroupState& item) {
        return item.entity_id == entity_id;
    };
    auto it = std::find_if(m_set_group_states.begin(), m_set_group_states.end(), find_by_entity_id);
    if(it != m_set_group_states.end())
    {
        if(it->trigger_callback_id != mono::INVALID_ID)
            m_trigger_system->RemoveTriggerCallback(it->trigger_hash, it->trigger_callback_id, entity_id);

        it->group_id = group_id;
        it->state = state;
        it->state_is_exclusive = false;
        it->trigger_hash = trigger_hash;

        const TriggerCallback on_trigger = [this, group_id, state](uint32_t trigger_id) {
            SetItemGroupState(group_id, state, true);
        };
        it->trigger_callback_id = m_trigger_system->RegisterTriggerCallback(it->trigger_hash, on_trigger, entity_id);
    }
}

uint32_t UISystem::SetUIItemCallback(uint32_t entity_id, const UIItemCallback& item_callback)
{
    UIItemCallbacks& callbacks = m_uiitem_callbacks[entity_id];

    uint32_t free_index = mono::INVALID_ID;

    for(uint32_t index = 0; index < callbacks.size(); ++index)
    {
        if(callbacks[index] == nullptr)
        {
            free_index = index;
            callbacks[index] = item_callback;
            break;
        }
    }

    return free_index;
}

void UISystem::ReleaseUIItemCallback(uint32_t entity_id, uint32_t callback_handle)
{
    auto it = m_uiitem_callbacks.find(entity_id);
    if(it != m_uiitem_callbacks.end())
        it->second[callback_handle] = nullptr;
}

void UISystem::SetUIItemState(uint32_t entity_id, UIItemState new_state)
{
    const auto find_by_entity_id = [entity_id](const UIItem& item) {
        return item.entity_id == entity_id;
    };

    auto it = std::find_if(m_items.begin(), m_items.end(), find_by_entity_id);
    if(it != m_items.end())
        it->state = new_state;
}

uint32_t UISystem::GetActiveEntityItem() const
{
    if(m_active_item_index == INVALID_UI_INDEX)
        return mono::INVALID_ID;

    return m_items[m_active_item_index].entity_id;
}

bool UISystem::DrawCursor() const
{
    return (m_input_context->most_recent_input == mono::InputContextType::Mouse);
}

math::Vector UISystem::GetCursorTargetPosition() const
{
    return m_camera_system->GetActiveCamera()->ScreenToWorld(m_mouse_screen_position);
}

mono::InputResult UISystem::Move(const event::MouseMotionEvent& event) 
{
    m_mouse_world_position = math::Vector(event.world_x, event.world_y);
    m_mouse_screen_position = math::Vector(event.screen_x, event.screen_y);
    return mono::InputResult::Handled;
}

mono::InputResult UISystem::ButtonDown(const event::MouseDownEvent& event)
{
    m_mouse_click_position = math::Vector(event.world_x, event.world_y);
    m_clicked_this_frame = true;

    return mono::InputResult::Handled;
}

mono::InputResult UISystem::ButtonDown(const event::ControllerButtonDownEvent& event)
{
    m_button_left = (event.button == System::ControllerButton::LEFT);
    m_button_right = (event.button == System::ControllerButton::RIGHT);
    m_button_up = (event.button == System::ControllerButton::UP);
    m_button_down = (event.button == System::ControllerButton::DOWN);
    m_button_push_this_frame = event.button == System::ControllerButton::FACE_BOTTOM;

    const bool event_handled =
        (m_button_left || m_button_right || m_button_up || m_button_down || m_button_push_this_frame);
    return event_handled ? mono::InputResult::Handled : mono::InputResult::Pass;
};

void UISystem::DrawDebug(const mono::UpdateContext& update_context)
{
    const std::string active_index = "Active Index: " + std::to_string(m_active_item_index);
    g_debug_drawer->DrawScreenText(active_index.c_str(), math::Vector(1.0f, 1.0f), mono::Color::BLACK);

    for(const UIItem& item : m_items)
    {
        const math::Quad world_bb = m_transform_system->GetWorldBoundingBox(item.entity_id);
        g_debug_drawer->DrawPoint(math::Center(world_bb), 10.0f, mono::Color::CYAN);
    }
}
