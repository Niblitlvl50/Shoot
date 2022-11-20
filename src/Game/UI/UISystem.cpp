
#include "UISystem.h"
#include "System/Hash.h"
#include "System/File.h"
#include "Util/Algorithm.h"
#include "Input/InputSystem.h"

#include "Rendering/Text/TextFunctions.h"

#include "nlohmann/json.hpp"

#include <algorithm>

using namespace game;

std::vector<game::UILayer> game::LoadUIConfig(const char* ui_config)
{
    std::vector<game::UILayer> layers;

    const std::vector<byte> file_data = file::FileReadAll(ui_config);
    const nlohmann::json& json = nlohmann::json::parse(file_data);

    for(const auto& layer_json : json["layers"])
    {
        UILayer layer;
        layer.name = layer_json["name"];
        layer.width = layer_json["width"];
        layer.height = layer_json["height"];
        layer.enabled = layer_json["enabled"];
        layer.consume_input = false;

        layers.push_back(layer);
    }

    return layers;
}

UISystem::UISystem(mono::InputSystem* input_system)
    : m_input_system(input_system)
{
    m_layers = LoadUIConfig("res/ui_config.json");
}

UISystem::~UISystem()
{
}

uint32_t UISystem::Id() const
{
    return hash::Hash(Name());
}

const char* UISystem::Name() const
{
    return "uisystem";
}

void UISystem::Update(const mono::UpdateContext& update_context)
{

}

void UISystem::LayerEnable(const std::string& layer_name, bool enable)
{
    UILayer* layer = FindLayer(layer_name);
    if(layer)
        layer->enabled = enable;
}

void UISystem::AllocateUIText(uint32_t entity_id)
{
    m_text_items[entity_id] = UITextItem();
}

void UISystem::ReleaseUIText(uint32_t entity_id)
{
    {
        const auto it = m_text_item_to_layer.find(entity_id);
        if(it != m_text_item_to_layer.end())
        {
            UILayer* layer = FindLayer(it->second);
            if(layer)
                mono::remove(layer->text_items, entity_id);
        
            m_text_item_to_layer.erase(entity_id);
        }
    }
    
    m_text_items.erase(entity_id);
}

void UISystem::UpdateUIText(
    uint32_t entity_id,
    const std::string& layer_name,
    int font_id,
    const std::string& text,
    const math::Vector& offset,
    const mono::Color::RGBA& color,
    mono::FontCentering centering)
{
    const auto it = m_text_item_to_layer.find(entity_id);
    if(it != m_text_item_to_layer.end())
    {
        UILayer* layer = FindLayer(it->second);
        if(layer)
            mono::remove(layer->text_items, entity_id);
    }

    m_text_item_to_layer[entity_id] = layer_name;

    UITextItem& text_item = m_text_items[entity_id];
    text_item.font_id = font_id;
    text_item.text = text;
    text_item.position = offset;
    text_item.color = color;
    text_item.centering = centering;

    const math::Vector& text_size = mono::MeasureString(text_item.font_id, text_item.text.c_str());
    const math::Vector& text_size_offset = mono::TextOffsetFromFontCentering(text_item.text_size, centering);

    text_item.text_size = text_size;
    text_item.text_offset = text_size_offset;

    UILayer* layer = FindLayer(layer_name);
    if(layer)
        layer->text_items.push_back(entity_id);
}

void UISystem::AllocateUIRect(uint32_t entity_id)
{
    m_rect_items[entity_id] = UIRectangle();
}

void UISystem::ReleaseUIRect(uint32_t entity_id)
{
    {
        const auto it = m_rect_item_to_layer.find(entity_id);
        if(it != m_rect_item_to_layer.end())
        {
            UILayer* layer = FindLayer(it->second);
            if(layer)
                mono::remove(layer->text_items, entity_id);
        
            m_rect_item_to_layer.erase(entity_id);
        }
    }
    
    m_rect_items.erase(entity_id);
}

void UISystem::UpdateUIRect(
    uint32_t entity_id,
    const std::string& layer_name,
    const math::Vector& position,
    const math::Vector& size,
    const mono::Color::RGBA& color,
    const mono::Color::RGBA& border_color,
    float border_width)
{
    const auto it = m_rect_item_to_layer.find(entity_id);
    if(it != m_rect_item_to_layer.end())
    {
        UILayer* layer = FindLayer(it->second);
        if(layer)
            mono::remove(layer->rect_items, entity_id);
    }

    m_rect_item_to_layer[entity_id] = layer_name;

    UIRectangle& rect_item = m_rect_items[entity_id];
    rect_item.position = position;
    rect_item.size = size;
    rect_item.color = color;
    rect_item.border_color = border_color;
    rect_item.border_width = border_width;

    UILayer* layer = FindLayer(layer_name);
    if(layer)
        layer->rect_items.push_back(entity_id);
}

const std::vector<UILayer>& UISystem::GetLayers() const
{
    return m_layers;
}

game::UILayer* UISystem::FindLayer(const std::string& layer_name)
{
    const auto find_by_name = [&layer_name](const UILayer& layer) {
        return layer.name == layer_name;
    };

    const auto it = std::find_if(m_layers.begin(), m_layers.end(), find_by_name);
    if(it != m_layers.end())
        return &(*it);

    return nullptr;
}

const UITextItem* UISystem::FindTextItem(uint32_t entity_id) const
{
    const auto it = m_text_items.find(entity_id);
    return (it != m_text_items.end()) ? &it->second : nullptr;
}

const UIRectangle* UISystem::FindRectItem(uint32_t entity_id) const
{
    const auto it = m_rect_items.find(entity_id);
    return (it != m_rect_items.end()) ? &it->second : nullptr;
}
