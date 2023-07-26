
#include "ShopSystem.h"
#include "System/File.h"

#include "nlohmann/json.hpp"

using namespace game;

ShopSystem::ShopSystem()
{
    file::FilePtr config_file = file::OpenAsciiFile("res/configs/shop_config.json");
    if(config_file)
    {
        const std::vector<byte>& file_data = file::FileRead(config_file);
        const nlohmann::json& json = nlohmann::json::parse(file_data);

        for(const auto& json_item : json["items"])
        {
            const std::string item_name = json_item["name"];
            const int item_cost = json_item["cost"];

            ItemComponent item;
            item.name = item_name;
            item.cost = item_cost;
            m_items.push_back(item);
        }
    }
}

const char* ShopSystem::Name() const
{
    return "ShopSystem";
}

void ShopSystem::Update(const mono::UpdateContext& update_context)
{

}

const std::vector<ItemComponent>& ShopSystem::GetItems() const
{
    return m_items;
}
