
#pragma once

#include "MonoFwd.h"
#include "IGameSystem.h"
#include "Math/Vector.h"
#include "Input/InputSystem.h"

#include <string>
#include <vector>
#include <unordered_map>
#include <array>
#include <functional>

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
        uint32_t left_entity_id;
        uint32_t right_entity_id;
        uint32_t above_entity_id;
        uint32_t below_entity_id;
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

    using UIItemCallback = std::function<void (uint32_t entity_id)>;

    class UISystem : public mono::IGameSystem, public mono::IMouseInput, public mono::IControllerInput
    {
    public:

        UISystem(
            mono::InputSystem* input_system,
            mono::TransformSystem* transform_system,
            class CameraSystem* camera_system,
            class TriggerSystem* trigger_system);
        ~UISystem();

        const char* Name() const override;
        
        void Begin() override;
        void Destroy() override;
        void Update(const mono::UpdateContext& update_context) override;
        bool UpdateInPause() const override;

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

        uint32_t SetUIItemCallback(uint32_t entity_id, const UIItemCallback& item_callback);
        void ReleaseUIItemCallback(uint32_t entity_id, uint32_t callback_handle);

        void SetUIItemState(uint32_t entity_id, UIItemState new_state);

        uint32_t GetActiveEntityItem() const;

        bool DrawCursor() const;
        math::Vector GetCursorTargetPosition() const;

    private:

        mono::InputResult Move(const event::MouseMotionEvent& event) override;
        mono::InputResult ButtonDown(const event::MouseDownEvent& event) override;
        mono::InputResult ButtonDown(const event::ControllerButtonDownEvent& event) override;

        void DrawDebug(const mono::UpdateContext& update_context);

        mono::InputSystem* m_input_system;
        mono::TransformSystem* m_transform_system;
        CameraSystem* m_camera_system;
        TriggerSystem* m_trigger_system;

        mono::InputContext* m_input_context;

        // Mouse Input Data
        math::Vector m_mouse_world_position;
        math::Vector m_mouse_screen_position;
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

        using UIItemCallbacks = std::array<UIItemCallback, 8>;
        std::unordered_map<uint32_t, UIItemCallbacks> m_uiitem_callbacks;

        int m_active_item_index;
    };
}
