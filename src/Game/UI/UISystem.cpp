
#include "UISystem.h"
#include "System/Hash.h"
#include "System/File.h"
#include "Util/Algorithm.h"

#include "nlohmann/json.hpp"

#include <algorithm>

using namespace game;

UISystem::UISystem()
{
    const std::vector<byte> file_data = file::FileReadAll("res/ui_config.json");
    const nlohmann::json& json = nlohmann::json::parse(file_data);

    for(const auto& layer_json : json["layers"])
    {
        UILayer layer;
        layer.name = layer_json["name"];
        layer.width = layer_json["width"];
        layer.height = layer_json["height"];
        layer.enabled = layer_json["enabled"];

        m_layers.push_back(layer);
    }
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

void UISystem::EnableLayer(const std::string& name, bool enable)
{
    UILayer* layer = FindLayer(name);
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
    uint32_t entity_id, const std::string& layer_name, int font_id, const std::string& text, const math::Vector& offset, const mono::Color::RGBA& color)
{
    m_text_item_to_layer[entity_id] = layer_name;

    UITextItem& text_item = m_text_items[entity_id];
    text_item.font_id = font_id;
    text_item.text = text;
    text_item.position = offset;
    text_item.color = color;

    UILayer* layer = FindLayer(layer_name);
    if(layer)
    {
        mono::remove(layer->text_items, entity_id);
        layer->text_items.push_back(entity_id);
    }
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
