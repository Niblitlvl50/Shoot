
#pragma once

#include "MonoFwd.h"
#include "IGameSystem.h"
#include "Math/Vector.h"
#include "Rendering/Color.h"
#include "Rendering/Text/TextFlags.h"

#include <string>
#include <vector>
#include <unordered_map>

namespace game
{
    struct UITextItem
    {
        int font_id;
        std::string text;
        math::Vector position;
        mono::Color::RGBA color;
        mono::FontCentering centering;

        // Internal data
        math::Vector text_size;
        math::Vector text_offset;
    };

    struct UIRectangle
    {
        math::Vector position;
        math::Vector size;
        mono::Color::RGBA color;
        mono::Color::RGBA border_color;
        float border_width;
    };

    struct UILayer
    {
        std::string name;
        int width;
        int height;
        bool enabled;
        bool consume_input;

        std::vector<uint32_t> text_items;
        std::vector<uint32_t> rect_items;
    };

    std::vector<UILayer> LoadUIConfig(const char* ui_config);

    class UISystem : public mono::IGameSystem
    {
    public:

        UISystem(mono::InputSystem* input_system);
        ~UISystem();

        uint32_t Id() const override;
        const char* Name() const override;
        void Update(const mono::UpdateContext& update_context) override;

        void LayerEnable(const std::string& layer_name, bool enable);
        void LayerConsumeInput(const std::string& layer_name, bool consume);

        void AllocateUIText(uint32_t entity_id);
        void ReleaseUIText(uint32_t entity_id);
        void UpdateUIText(
            uint32_t entity_id,
            const std::string& layer_name,
            int font_id,
            const std::string& text,
            const math::Vector& offset,
            const mono::Color::RGBA& color,
            mono::FontCentering centering);

        void AllocateUIRect(uint32_t entity_id);
        void ReleaseUIRect(uint32_t entity_id);
        void UpdateUIRect(
            uint32_t entity_id,
            const std::string& layer_name,
            const math::Vector& position,
            const math::Vector& size,
            const mono::Color::RGBA& color,
            const mono::Color::RGBA& border_color,
            float border_width);

        const std::vector<UILayer>& GetLayers() const;
        UILayer* FindLayer(const std::string& layer_name);

        const UITextItem* FindTextItem(uint32_t entity_id) const;
        const UIRectangle* FindRectItem(uint32_t entity_id) const;

        mono::InputSystem* m_input_system;

        std::vector<UILayer> m_layers;

        std::unordered_map<uint32_t, UITextItem> m_text_items;
        std::unordered_map<uint32_t, std::string> m_text_item_to_layer;

        std::unordered_map<uint32_t, UIRectangle> m_rect_items;
        std::unordered_map<uint32_t, std::string> m_rect_item_to_layer;
    };
}
