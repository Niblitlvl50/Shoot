
#include "RegionSystem.h"
#include "TriggerSystem/TriggerSystem.h"

#include "System/Hash.h"
#include "System/File.h"
#include "System/System.h"

#include "nlohmann/json.hpp"

using namespace game;

namespace
{
    std::vector<game::RegionDescription> ParseRegionConfig(const char* region_config)
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
            System::Log("[%u] %s|%s", region.trigger_hash, region.text.c_str(), region.sub_text.c_str());

        return regions;
    }
}

RegionSystem::RegionSystem(TriggerSystem* trigger_system)
    : m_trigger_system(trigger_system)
{
    m_region_descriptions = ParseRegionConfig("res/region_config.json");

    const game::TriggerCallback handle_region_trigger = [this](uint32_t trigger_hash) {
        HandleRegionTrigger(trigger_hash);
    };

    for(const game::RegionDescription& description : m_region_descriptions)
    {
        const uint32_t trigger_id = m_trigger_system->RegisterTriggerCallback(description.trigger_hash, handle_region_trigger, -1);
        m_registred_triggers.push_back(trigger_id);
    }

    m_activated_region = { 0, "", "" };
}

uint32_t RegionSystem::Id() const
{
    return hash::Hash(Name());
}

const char* RegionSystem::Name() const
{
    return "regionsystem";
}

void RegionSystem::Destroy()
{
    for(uint32_t index = 0; index < m_region_descriptions.size(); ++index)
    {
        const uint32_t trigger_hash = m_region_descriptions[index].trigger_hash;
        const uint32_t callback_id = m_registred_triggers[index];
        m_trigger_system->RemoveTriggerCallback(trigger_hash, callback_id, -1);
    }
}

void RegionSystem::Update(const mono::UpdateContext& update_context)
{

}

const RegionDescription& RegionSystem::GetActivatedRegion() const
{
    return m_activated_region;
}

void RegionSystem::HandleRegionTrigger(uint32_t trigger_hash)
{
    const auto find_by_hash = [](const RegionDescription& left, uint32_t trigger_hash) {
        return left.trigger_hash < trigger_hash;
    };
    const auto it = std::lower_bound(m_region_descriptions.begin(), m_region_descriptions.end(), trigger_hash, find_by_hash);

    if(it != m_region_descriptions.end())
        m_activated_region = *it;
}
