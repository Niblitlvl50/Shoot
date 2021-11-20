
#include "RegionDrawer.h"

#include "FontIds.h"
#include "TriggerSystem/TriggerSystem.h"

#include "Math/EasingFunctions.h"
#include "Rendering/RenderSystem.h"
#include "System/Hash.h"
#include "System/File.h"

#include "nlohmann/json.hpp"


std::vector<game::RegionDescription> game::ParseRegionConfig(const char* region_config)
{
    std::vector<RegionDescription> regions;

    file::FilePtr config_file = file::OpenAsciiFile(region_config);
    if(config_file)
    {
        const std::vector<byte> file_data = file::FileRead(config_file);
        const nlohmann::json& json = nlohmann::json::parse(file_data);

        for(const auto& regions_json : json["regions"])
        {
            const std::string trigger_name = regions_json["trigger_name"];

            RegionDescription region_description;
            region_description.trigger_hash = hash::Hash(trigger_name.c_str());
            region_description.text = regions_json["text"];
            region_description.sub_text = regions_json["sub_text"];
            regions.push_back(region_description);
        }
    }

    const auto sort_by_hash = [](const RegionDescription& left, const RegionDescription& right) {
        return left.trigger_hash < right.trigger_hash;
    };

    std::sort(regions.begin(), regions.end(), sort_by_hash);

    for(const auto& region : regions)
    {
        printf("[%u] %s|%s\n", region.trigger_hash, region.text.c_str(), region.sub_text.c_str());
    }

    return regions;
}

using namespace game;

RegionDrawer::RegionDrawer(const std::vector<game::RegionDescription>& region_descriptions, TriggerSystem* trigger_system)
    : UIOverlay(1200.0f, 1200.0f / mono::GetWindowAspect())
    , m_region_descriptions(region_descriptions)
    , m_trigger_system(trigger_system)
{
    const game::TriggerCallback handle_region_trigger = [this](uint32_t trigger_hash) {
        HandleRegionTrigger(trigger_hash);
    };

    for(const game::RegionDescription& description : m_region_descriptions)
    {
        const uint32_t trigger_id = m_trigger_system->RegisterTriggerCallback(description.trigger_hash, handle_region_trigger, -1);
        m_registred_triggers.push_back(trigger_id);
    }

    m_region_text = new UITextElement(shared::FontId::RUSSOONE_MEGA, "", mono::FontCentering::DEFAULT_CENTER, mono::Color::GRAY);
    m_region_text->SetPosition(100, 700);

    m_region_subtext = new UITextElement(shared::FontId::RUSSOONE_MEDIUM, "", mono::FontCentering::DEFAULT_CENTER, mono::Color::DARK_GRAY);
    m_region_subtext->SetPosition(110, 670);

    AddChild(m_region_text);
    AddChild(m_region_subtext);
}

RegionDrawer::~RegionDrawer()
{
    for(uint32_t index = 0; index < m_region_descriptions.size(); ++index)
    {
        const uint32_t trigger_hash = m_region_descriptions[index].trigger_hash;
        const uint32_t callback_id = m_registred_triggers[index];
        m_trigger_system->RemoveTriggerCallback(trigger_hash, callback_id, -1);
    }
}

void RegionDrawer::Update(const mono::UpdateContext& context)
{
    UIOverlay::Update(context);

    m_text_timer += context.delta_s;

    constexpr float ease_in_out_time = 1.0f;
    constexpr float display_time = 3.0f;
    constexpr float ease_out_time = ease_in_out_time + display_time;

    if(m_text_timer < ease_in_out_time)
    {
        const float alpha = math::EaseInOutCubic(m_text_timer, ease_in_out_time, 0.0f, 1.0f);
        m_region_text->SetAlpha(alpha);
        m_region_subtext->SetAlpha(alpha);
    }
    else if(m_text_timer > display_time && m_text_timer < ease_out_time)
    {
        const float time = m_text_timer - display_time;
        const float alpha = math::EaseInOutCubic(time, 1.0f, 1.0f, -1.0f);
        m_region_text->SetAlpha(alpha);
        m_region_subtext->SetAlpha(alpha);
    }
}

void RegionDrawer::HandleRegionTrigger(uint32_t trigger_hash)
{
    const auto find_by_hash = [](const RegionDescription& left, uint32_t trigger_hash) {
        return left.trigger_hash == trigger_hash;
    };
    const auto it = std::lower_bound(m_region_descriptions.begin(), m_region_descriptions.end(), trigger_hash, find_by_hash);

    if(it != m_region_descriptions.end())
    {
        m_region_text->SetText(it->text);
        m_region_subtext->SetText(it->sub_text);
        m_text_timer = 0.0f;
    }
}
