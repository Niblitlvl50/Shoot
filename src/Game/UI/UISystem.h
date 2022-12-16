
#pragma once

#include "MonoFwd.h"
#include "IGameSystem.h"
#include "Math/Vector.h"
#include "Input/InputSystem.h"

#include <string>
#include <vector>
#include <unordered_map>

namespace game
{
    enum class UIItemState : uint8_t
    {
        Enabled,
        Disabled,
        Hidden
    };

    constexpr const char* g_ui_item_state_strings[] = {
        "Enabled",
        "Disabled",
        "Hidden",
    };

    inline const char* UIItemStateToString(UIItemState state)
    {
        return g_ui_item_state_strings[uint8_t(state)];
    }

    struct UINavigationSetup
    {
        uint32_t left_entity_uuid;
        uint32_t right_entity_uuid;
        uint32_t above_entity_uuid;
        uint32_t below_entity_uuid;
    };

    struct UIItem
    {
        uint32_t entity_id;
        int group_id;
        UIItemState state;
        uint32_t on_click_hash;
        UINavigationSetup navigation_setup;
    };

    struct UISetGroupState
    {
        uint32_t entity_id;
        int group_id;
        UIItemState state;
        bool state_is_exclusive;
        uint32_t trigger_hash;

        uint32_t trigger_callback_id;
    };

    class UISystem : public mono::IGameSystem, public mono::IMouseInput, public mono::IControllerInput
    {
    public:

        UISystem(
            mono::InputSystem* input_system,
            mono::TransformSystem* transform_system,
            mono::IEntityManager* entity_system,
            class TriggerSystem* trigger_system);
        ~UISystem();

        uint32_t Id() const override;
        const char* Name() const override;
        
        void Begin() override;
        void Destroy() override;
        void Update(const mono::UpdateContext& update_context) override;

        void Enable();
        void Disable();
        bool IsEnabled() const;
        void SetItemGroupState(int group_id, UIItemState new_state, bool exclusive_state);

        void AllocateUIItem(uint32_t entity_id);
        void ReleaseUIItem(uint32_t entity_id);
        void UpdateUIItem(
            uint32_t entity_id,
            uint32_t on_click_hash,
            int group_id,
            UIItemState state,
            const UINavigationSetup& navigation_setup);

        void AllocateUISetGroupState(uint32_t entity_id);
        void ReleaseUISetGroupState(uint32_t entity_id);
        void UpdateUISetGroupState(uint32_t entity_id, int group_id, UIItemState state, uint32_t trigger_hash);

        uint32_t GetActiveEntityItem() const;

        bool DrawCursor() const;
        const math::Vector& GetCursorTargetPosition() const;

    private:

        mono::InputResult Move(const event::MouseMotionEvent& event) override;
        mono::InputResult ButtonDown(const event::MouseDownEvent& event) override;
        mono::InputResult ButtonDown(const event::ControllerButtonDownEvent& event) override;

        void DrawDebug(const mono::UpdateContext& update_context);

        mono::InputSystem* m_input_system;
        mono::TransformSystem* m_transform_system;
        mono::IEntityManager* m_entity_system;
        TriggerSystem* m_trigger_system;

        mono::InputContext* m_input_context;

        // Mouse Input Data
        math::Vector m_mouse_world_position;
        math::Vector m_mouse_click_position;
        bool m_clicked_this_frame;

        // Controller Input Data
        bool m_button_left;
        bool m_button_right;
        bool m_button_up;
        bool m_button_down;
        bool m_button_push_this_frame;

        std::vector<UIItem> m_items;
        std::vector<UISetGroupState> m_set_group_states;

        int m_active_item_index;
    };
}
