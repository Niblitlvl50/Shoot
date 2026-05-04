
#include "PerkSystem.h"
#include "Serialize.h"
#include "System/File.h"

#include "nlohmann/json.hpp"


using namespace game;

PerkSystem::PerkSystem()
{
    const std::vector<byte> file_data = file::FileReadAll("res/configs/perks_config.json");
    const nlohmann::json& json = nlohmann::json::parse(file_data);
    m_perk_definitions = json["perks"];
}

const char* PerkSystem::Name() const
{
    return "PerkSystem";
}

void PerkSystem::Update(const mono::UpdateContext& update_context)
{

}
