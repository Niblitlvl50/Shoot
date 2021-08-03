
#include "ConditionFile.h"
#include "ConditionSystem.h"

#include "System/System.h"
#include "System/File.h"
#include "System/Hash.h"
#include "nlohmann/json.hpp"

std::vector<game::ConditionData> game::LoadConditionsFromFile(const char* condition_file)
{
    std::vector<ConditionData> loaded_data;

    file::FilePtr file = file::OpenAsciiFile(condition_file);
    if(file)
    {
        const std::vector<byte> file_data = file::FileRead(file);
        const nlohmann::json& json = nlohmann::json::parse(file_data);

        for(const auto& condition_value : json["conditions"])
        {
            const std::string name = condition_value["name"];
            const bool initial_value = condition_value["initial_value"];

            loaded_data.push_back({ name, initial_value });
        }
    }
    else
    {
        System::Log("Unable to load condition file [%s]", condition_file);
    }

    return loaded_data;
}

void game::WriteConditionsToFile(const char* condition_file, const std::vector<ConditionData>& conditions)
{
    nlohmann::json condition_array;
    
    for(const ConditionData& condition : conditions)
    {
        nlohmann::json entry;
        entry["name"] = condition.name;
        entry["initial_value"] = condition.value;

        condition_array.push_back(entry);
    }

    nlohmann::json condition_container;
    condition_container["conditions"] = condition_array;

    const std::string& serialized_json = condition_container.dump(4);

    file::FilePtr file = file::CreateAsciiFile(condition_file);
    std::fwrite(serialized_json.data(), serialized_json.length(), sizeof(char), file.get());
}

void game::LoadConditionsFromFile(const char* condition_file, ConditionSystem* condition_system)
{
    const std::vector<ConditionData> loaded_data = LoadConditionsFromFile(condition_file);
    for(const ConditionData& condition : loaded_data)
        condition_system->RegisterCondition(hash::Hash(condition.name.c_str()), condition.value);
}
