
#include "SaveSystem.h"
#include "PlayerInfo.h"
#include "System/File.h"

#include "nlohmann/json.hpp"
#include <string>

void game::SavePlayerData(const char* user_path, int slot_index, const SaveSlotData& data)
{
    nlohmann::json save_data_array;

    for(uint32_t index = 0; index < std::size(data.player_data); ++index)
    {
        const PersistentPlayerData& player_data = data.player_data[index];
        nlohmann::json player_data_json;
        player_data_json["chips"] = player_data.chips;
        player_data_json["rubble"] = player_data.rubble;
        player_data_json["experience"] = player_data.experience;
        player_data_json["god_mode"] = player_data.god_mode;
        player_data_json["auto_aim"] = player_data.auto_aim;
        player_data_json["auto_reload"] = player_data.auto_reload;
        save_data_array.push_back(player_data_json);
    }

    nlohmann::json json;
    json["slots"] = save_data_array;
    const std::string& serialized_config = json.dump(4);

    const std::string save_folder = user_path + std::string("saves/");
    file::EnsurePath(save_folder.c_str());

    const std::string save_file = save_folder + std::to_string(slot_index) + ".json";
    file::FilePtr file = file::CreateAsciiFile(save_file.c_str());
    std::fwrite(serialized_config.data(), serialized_config.length(), sizeof(char), file.get());
}

void game::LoadPlayerData(const char* user_path, int slot_index, SaveSlotData& data)
{
    const std::string save_file = user_path + std::string("saves/") + std::to_string(slot_index) + ".json";
    const bool file_exists = file::Exists(save_file.c_str());
    if(!file_exists)
        return;

    file::FilePtr file = file::OpenAsciiFile(save_file.c_str());
    if(!file)
        return;

    const std::vector<byte> file_data = file::FileRead(file);
    const nlohmann::json& json = nlohmann::json::parse(file_data);

    const nlohmann::json& save_data_array = json["slots"];

    for(uint32_t index = 0; index < save_data_array.size(); ++index)
    {
        data.player_data[index].chips = save_data_array[index].value("chips", 0);
        data.player_data[index].rubble = save_data_array[index].value("rubble", 0);
        data.player_data[index].experience = save_data_array[index].value("experience", 0);
        data.player_data[index].god_mode = save_data_array[index].value("god_mode", false);
        data.player_data[index].auto_aim = save_data_array[index].value("auto_aim", false);
        data.player_data[index].auto_reload = save_data_array[index].value("auto_reload", false);
    }
}
