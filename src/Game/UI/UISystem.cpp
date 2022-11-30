
#include "UISystem.h"
#include "TriggerSystem/TriggerSystem.h"

#include "Input/InputSystem.h"
#include "Rendering/Text/TextFunctions.h"
#include "System/Hash.h"
#include "System/File.h"
#include "TransformSystem/TransformSystem.h"
#include "Util/Algorithm.h"

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

UISystem::UISystem(mono::InputSystem* input_system, mono::TransformSystem* transform_system, TriggerSystem* trigger_system)
    : m_input_system(input_system)
    , m_transform_system(transform_system)
    , m_trigger_system(trigger_system)
    , m_clicked_this_frame(false)
{
    m_layers = LoadUIConfig("res/ui_config.json");
    m_input_context = m_input_system->CreateContext(0, mono::InputContextBehaviour::ConsumeAll);

    m_input_context->enabled = false;
    m_input_context->mouse_input = this;
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

void UISystem::Destroy()
{
    m_input_system->ReleaseContext(m_input_context);
}

void UISystem::Update(const mono::UpdateContext& update_context)
{
    m_active_items.clear();

    if(m_input_context->enabled)
    {
        for(auto item_pair : m_items)
        {
            const math::Quad& world_bb = m_transform_system->GetWorldBoundingBox(item_pair.first);
            const bool inside_quad = math::PointInsideQuad(m_mouse_world_position, world_bb);
            if(inside_quad)
            {
                m_active_items.push_back(item_pair.first);
                if(m_clicked_this_frame)
                    m_trigger_system->EmitTrigger(item_pair.second.on_click_hash);
            }
        }
    }

    m_clicked_this_frame = false;
}

void UISystem::LayerEnable(const std::string& layer_name, bool enable)
{
    UILayer* layer = FindLayer(layer_name);
    if(layer)
        layer->enabled = enable;
}

void UISystem::AllocateUIItem(uint32_t entity_id)
{
    m_items[entity_id] = UIItem();
}

void UISystem::ReleaseUIItem(uint32_t entity_id)
{
    m_items.erase(entity_id);
}

void UISystem::UpdateUIItem(uint32_t entity_id, uint32_t on_click_hash)
{
    auto it = m_items.find(entity_id);
    if(it != m_items.end())
    {
        it->second.on_click_hash = on_click_hash;
    }
}

const std::vector<UILayer>& UISystem::GetLayers() const
{
    return m_layers;
}

const std::vector<uint32_t>& UISystem::GetActiveItems() const
{
    return m_active_items;
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

mono::InputResult UISystem::Move(const event::MouseMotionEvent& event) 
{
    m_mouse_world_position = math::Vector(event.world_x, event.world_y);
    return mono::InputResult::Handled;
}

mono::InputResult UISystem::ButtonDown(const event::MouseDownEvent& event)
{
    m_mouse_click_position = math::Vector(event.world_x, event.world_y);
    m_clicked_this_frame = true;

    return mono::InputResult::Handled;
}
