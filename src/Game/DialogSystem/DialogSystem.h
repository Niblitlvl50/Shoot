
#pragma once

#include "MonoFwd.h"
#include "IGameSystem.h"
#include "Input/InputInterfaces.h"
#include "System/System.h"
#include "System/Hash.h"
#include "Util/ActiveVector.h"

#include <string>
#include <deque>
#include <vector>
#include <unordered_map>

namespace game
{
    constexpr uint32_t MAX_DIALOG_OPTIONS = 8;

    struct DialogOption
    {
        std::string text;
        uint32_t trigger_hash = hash::NO_HASH;
    };

    struct DialogData
    {
        std::string speaker;
        std::string message;
        std::string sprite_file;
        float duration = 0.0f;

        // A dialog with options waits for the player to pick one, duration is then ignored.
        std::vector<DialogOption> options;
    };

    struct DialogComponent
    {
        uint32_t trigger_hash;
        uint32_t trigger_callback_id;
        bool emit_once;
        bool shown;
        DialogData data;
    };

    struct DialogLine
    {
        uint32_t id;
        DialogData data;
        int selected_option;
        bool answered;
    };

    class DialogSystem : public mono::IGameSystem, public mono::IKeyboardInput, public mono::IControllerInput
    {
    public:

        DialogSystem(uint32_t n, mono::TriggerSystem* trigger_system, mono::InputSystem* input_system);

        DialogComponent* AllocateComponent(uint32_t entity_id);
        void ReleaseComponent(uint32_t entity_id);
        void SetComponentData(uint32_t entity_id, uint32_t trigger_hash, bool emit_once, const DialogData& data);

        // An entity can carry several dialog_options components, each Allocate call appends one
        // option and the following SetOptionData call fills in the one just added.
        void AllocateOption(uint32_t entity_id);
        void ReleaseOptions(uint32_t entity_id);
        void SetOptionData(uint32_t entity_id, const DialogOption& option);

        // Queues the component's line, with the entity's dialog options if it has any. Lines are shown one after another.
        void ShowDialog(uint32_t entity_id);
        void PushDialog(const DialogData& data);

        // Returns nullptr when no dialog is showing.
        const DialogLine* GetActiveDialog() const;
        float GetActiveDialogTime() const;

        const char* Name() const override;
        bool UpdateInPause() const override;
        void Update(const mono::UpdateContext& update_context) override;
        void Reset() override;
        void Destroy() override;

    private:

        mono::InputResult KeyDown(const event::KeyDownEvent& event) override;
        mono::InputResult KeyUp(const event::KeyUpEvent& event) override;
        mono::InputResult UpdatedControllerState(const System::ControllerState& updated_state) override;

        void UpdateChoice(DialogLine& line);
        void SetInputEnabled(bool enabled);

        mono::TriggerSystem* m_trigger_system;
        mono::InputSystem* m_input_system;
        mono::InputContext* m_input_context;
        mono::ActiveVector<DialogComponent> m_components;
        std::unordered_map<uint32_t, std::vector<DialogOption>> m_options;

        std::deque<DialogLine> m_dialog_queue;
        float m_active_dialog_time_s;
        uint32_t m_next_line_id;

        int m_move_selection;
        int m_pick_option;
        bool m_confirm;
        System::ControllerState m_last_controller_state;
    };
}
