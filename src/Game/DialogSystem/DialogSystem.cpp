
#include "DialogSystem.h"
#include "TriggerSystem/TriggerSystem.h"
#include "Input/InputSystem.h"
#include "System/Hash.h"
#include "IUpdatable.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace
{
    constexpr uint32_t NO_CALLBACK_SET = std::numeric_limits<uint32_t>::max();
    constexpr int NO_OPTION = -1;

    // Matches the drawer's open/close animation time.
    constexpr float choice_close_time_s = 0.25f;
    constexpr float stick_threshold = 0.5f;
}

using namespace game;

DialogSystem::DialogSystem(uint32_t n, mono::TriggerSystem* trigger_system, mono::InputSystem* input_system)
    : m_trigger_system(trigger_system)
    , m_input_system(input_system)
    , m_components(n)
    , m_active_dialog_time_s(0.0f)
    , m_next_line_id(0)
    , m_move_selection(0)
    , m_pick_option(NO_OPTION)
    , m_confirm(false)
    , m_last_controller_state{}
{
    // Before the ui and player contexts, so a choice takes the input while it's waiting.
    m_input_context = m_input_system->CreateContext(-1, mono::InputContextBehaviour::ConsumeIfHandled, "DialogSystem");
    m_input_context->keyboard_input = this;
    m_input_context->controller_input = this;
    m_input_system->DisableContext(m_input_context);
}

DialogComponent* DialogSystem::AllocateComponent(uint32_t entity_id)
{
    DialogComponent component;
    component.trigger_hash = hash::NO_HASH;
    component.trigger_callback_id = NO_CALLBACK_SET;
    component.emit_once = false;
    component.shown = false;

    return m_components.Set(entity_id, std::move(component));
}

void DialogSystem::ReleaseComponent(uint32_t entity_id)
{
    DialogComponent* component = m_components.Get(entity_id);
    if(component->trigger_callback_id != NO_CALLBACK_SET)
        m_trigger_system->RemoveTriggerCallback(component->trigger_hash, component->trigger_callback_id, entity_id);

    m_components.Release(entity_id);
}

void DialogSystem::SetComponentData(uint32_t entity_id, uint32_t trigger_hash, bool emit_once, const DialogData& data)
{
    DialogComponent* component = m_components.Get(entity_id);

    if(component->trigger_callback_id != NO_CALLBACK_SET)
    {
        m_trigger_system->RemoveTriggerCallback(component->trigger_hash, component->trigger_callback_id, entity_id);
        component->trigger_callback_id = NO_CALLBACK_SET;
    }

    component->trigger_hash = trigger_hash;
    component->emit_once = emit_once;
    component->data = data;

    if(component->trigger_hash != hash::NO_HASH)
    {
        const mono::TriggerCallback show_callback = [this, entity_id](uint32_t trigger_id) {
            ShowDialog(entity_id);
        };
        component->trigger_callback_id = m_trigger_system->RegisterTriggerCallback(component->trigger_hash, show_callback, entity_id);
    }
}

void DialogSystem::AllocateOption(uint32_t entity_id)
{
    m_options[entity_id].emplace_back();
}

void DialogSystem::ReleaseOptions(uint32_t entity_id)
{
    m_options.erase(entity_id);
}

void DialogSystem::SetOptionData(uint32_t entity_id, const DialogOption& option)
{
    const auto it = m_options.find(entity_id);
    if(it == m_options.end() || it->second.empty())
        return;

    it->second.back() = option;
}

void DialogSystem::ShowDialog(uint32_t entity_id)
{
    if(!m_components.IsActive(entity_id))
        return;

    DialogComponent* component = m_components.Get(entity_id);
    if(component->emit_once && component->shown)
        return;

    component->shown = true;

    DialogData data = component->data;

    const auto options_it = m_options.find(entity_id);
    if(options_it != m_options.end())
        data.options = options_it->second;

    PushDialog(data);
}

void DialogSystem::PushDialog(const DialogData& data)
{
    DialogLine line;
    line.id = m_next_line_id++;
    line.data = data;
    line.selected_option = 0;
    line.answered = false;

    std::vector<DialogOption>& options = line.data.options;
    const auto is_empty = [](const DialogOption& option) { return option.text.empty(); };
    options.erase(std::remove_if(options.begin(), options.end(), is_empty), options.end());
    if(options.size() > MAX_DIALOG_OPTIONS)
        options.resize(MAX_DIALOG_OPTIONS);

    if(!options.empty())
        line.data.duration = std::numeric_limits<float>::max();

    m_dialog_queue.push_back(line);
}

const DialogLine* DialogSystem::GetActiveDialog() const
{
    if(m_dialog_queue.empty())
        return nullptr;

    return &m_dialog_queue.front();
}

float DialogSystem::GetActiveDialogTime() const
{
    return m_active_dialog_time_s;
}

const char* DialogSystem::Name() const
{
    return "dialogsystem";
}

bool DialogSystem::UpdateInPause() const
{
    // Runs in pause so it can hand the input over to the pause menu.
    return true;
}

void DialogSystem::Update(const mono::UpdateContext& update_context)
{
    if(update_context.paused)
    {
        // Leave the input to the pause menu, the dialog picks up again on unpause.
        if(m_input_context->enabled)
            SetInputEnabled(false);
        return;
    }

    if(m_dialog_queue.empty())
    {
        if(m_input_context->enabled)
            SetInputEnabled(false);
    }
    else
    {
        DialogLine& active_line = m_dialog_queue.front();

        const bool wants_input = !active_line.data.options.empty() && !active_line.answered;
        if(wants_input != m_input_context->enabled)
        {
            // Drop what the player is holding, the dialog keeps the input until an option is picked.
            if(wants_input)
                m_input_system->ResetAllInput();
            SetInputEnabled(wants_input);
        }
        else if(wants_input)
        {
            UpdateChoice(active_line);
        }

        m_active_dialog_time_s += update_context.delta_s;
        if(m_active_dialog_time_s >= active_line.data.duration)
        {
            m_dialog_queue.pop_front();
            m_active_dialog_time_s = 0.0f;
        }
    }
}

void DialogSystem::UpdateChoice(DialogLine& line)
{
    const int n_options = static_cast<int>(line.data.options.size());
    line.selected_option = std::clamp(line.selected_option + m_move_selection, 0, n_options - 1);

    int picked_option = NO_OPTION;
    if(m_pick_option != NO_OPTION && m_pick_option < n_options)
        picked_option = m_pick_option;
    else if(m_confirm)
        picked_option = line.selected_option;

    m_move_selection = 0;
    m_pick_option = NO_OPTION;
    m_confirm = false;

    if(picked_option == NO_OPTION)
        return;

    line.selected_option = picked_option;
    line.answered = true;
    line.data.duration = m_active_dialog_time_s + choice_close_time_s;
    SetInputEnabled(false);

    const uint32_t trigger_hash = line.data.options[picked_option].trigger_hash;
    if(trigger_hash != hash::NO_HASH)
        m_trigger_system->EmitTrigger(trigger_hash);
}

void DialogSystem::SetInputEnabled(bool enabled)
{
    m_move_selection = 0;
    m_pick_option = NO_OPTION;
    m_confirm = false;

    if(enabled)
    {
        m_last_controller_state = System::GetController(m_input_context->controller_id);
        m_input_system->EnableContext(m_input_context);
    }
    else
    {
        m_input_system->DisableContext(m_input_context);
    }
}

void DialogSystem::Reset()
{
    m_dialog_queue.clear();
    m_active_dialog_time_s = 0.0f;
    SetInputEnabled(false);
}

void DialogSystem::Destroy()
{
    m_input_system->ReleaseContext(m_input_context);
    m_input_context = nullptr;
}

mono::InputResult DialogSystem::KeyDown(const event::KeyDownEvent& event)
{
    switch(event.key)
    {
    case Keycode::W:
    case Keycode::UP:
        m_move_selection = -1;
        break;
    case Keycode::S:
    case Keycode::DOWN:
        m_move_selection = 1;
        break;
    case Keycode::ONE:
    case Keycode::TWO:
    case Keycode::THREE:
    case Keycode::FOUR:
    case Keycode::FIVE:
    case Keycode::SIX:
    case Keycode::SEVEN:
    case Keycode::EIGHT:
        m_pick_option = static_cast<int>(event.key) - static_cast<int>(Keycode::ONE);
        break;
    case Keycode::E:
    case Keycode::ENTER:
        m_confirm = true;
        break;
    default:
        break;
    }

    // Swallow all presses so the player doesn't act while choosing.
    return mono::InputResult::Handled;
}

mono::InputResult DialogSystem::KeyUp(const event::KeyUpEvent& event)
{
    // Releases pass on, otherwise keys held when the dialog opened would get stuck in the player controllers.
    return mono::InputResult::Pass;
}

mono::InputResult DialogSystem::UpdatedControllerState(const System::ControllerState& updated_state)
{
    const System::ControllerState& last = m_last_controller_state;

    const bool up_pressed =
        System::IsButtonTriggered(last.button_state, updated_state.button_state, System::ControllerButton::UP) ||
        (last.left_y < stick_threshold && updated_state.left_y >= stick_threshold);
    const bool down_pressed =
        System::IsButtonTriggered(last.button_state, updated_state.button_state, System::ControllerButton::DOWN) ||
        (last.left_y > -stick_threshold && updated_state.left_y <= -stick_threshold);
    const bool confirm_pressed =
        System::IsButtonTriggered(last.button_state, updated_state.button_state, System::ControllerButton::FACE_BOTTOM);

    if(up_pressed)
        m_move_selection = -1;
    else if(down_pressed)
        m_move_selection = 1;

    if(confirm_pressed)
        m_confirm = true;

    m_last_controller_state = updated_state;
    return mono::InputResult::Handled;
}
