
#pragma once

#include "MonoFwd.h"
#include "IGameSystem.h"
#include "Math/Vector.h"
#include "Rendering/Color.h"
#include "Rendering/Text/TextFlags.h"
#include "Input/InputSystem.h"

#include <string>
#include <vector>
#include <unordered_map>

namespace game
{
    struct UIItem
    {
        uint32_t on_click_hash;
    };

    struct UILayer
    {
        std::string name;
        int width;
        int height;
        bool enabled;
        bool consume_input;

        std::vector<uint32_t> items;
    };

    std::vector<UILayer> LoadUIConfig(const char* ui_config);

    class UISystem : public mono::IGameSystem, public mono::IMouseInput
    {
    public:

        UISystem(mono::InputSystem* input_system, mono::TransformSystem* transform_system, class TriggerSystem* trigger_system);
        ~UISystem();

        uint32_t Id() const override;
        const char* Name() const override;
        void Destroy() override;
        void Update(const mono::UpdateContext& update_context) override;

        void LayerEnable(const std::string& layer_name, bool enable);

        void AllocateUIItem(uint32_t entity_id);
        void ReleaseUIItem(uint32_t entity_id);
        void UpdateUIItem(uint32_t entity_id, uint32_t on_click_hash);

        const std::vector<UILayer>& GetLayers() const;
        const std::vector<uint32_t>& GetActiveItems() const;
        UILayer* FindLayer(const std::string& layer_name);

    private:

        mono::InputResult Move(const event::MouseMotionEvent& event) override;
        mono::InputResult ButtonDown(const event::MouseDownEvent& event) override;

        mono::InputSystem* m_input_system;
        mono::TransformSystem* m_transform_system;
        TriggerSystem* m_trigger_system;

        mono::InputContext* m_input_context;

        math::Vector m_mouse_world_position;
        math::Vector m_mouse_click_position;
        bool m_clicked_this_frame;

        std::vector<UILayer> m_layers;

        std::unordered_map<uint32_t, UIItem> m_items;
        std::vector<uint32_t> m_active_items;
    };
}
