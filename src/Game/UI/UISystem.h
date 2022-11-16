
#pragma once

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
    };

    struct UILayer
    {
        std::string name;
        int width;
        int height;
        bool enabled;

        std::vector<uint32_t> text_items;
    };

    std::vector<UILayer> LoadUIConfig(const char* ui_config);

    class UISystem : public mono::IGameSystem
    {
    public:

        UISystem();

        uint32_t Id() const override;
        const char* Name() const override;
        void Update(const mono::UpdateContext& update_context) override;

        void EnableLayer(const std::string& name, bool enable);

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

        const std::vector<UILayer>& GetLayers() const;
        UILayer* FindLayer(const std::string& layer_name);

        const UITextItem* FindTextItem(uint32_t entity_id) const;

        std::vector<UILayer> m_layers;

        std::unordered_map<uint32_t, UITextItem> m_text_items;
        std::unordered_map<uint32_t, std::string> m_text_item_to_layer;
    };
}
