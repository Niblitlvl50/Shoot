
#include "WriteSpriteFile.h"
#include "System/File.h"

#include "nlohmann/json.hpp"

namespace
{
    std::vector<std::string> g_all_sprite_files;
}

void animator::WriteSpriteFile(const char* sprite_file, const mono::SpriteData* sprite_data)
{
    file::FilePtr input_file = file::OpenAsciiFile(sprite_file);
    if(!input_file)
        return;

    const std::vector<byte>& file_data = file::FileRead(input_file);

    input_file = nullptr;

    nlohmann::json json = nlohmann::json::parse(file_data);
    nlohmann::json& json_animations = json["animations"];
    json_animations.clear();

    for(const mono::SpriteAnimation& animation : sprite_data->animations)
    {
        nlohmann::json object;
        object["name"] = animation.name;
        object["loop"] = animation.looping;
        object["frame_duration"] = animation.frame_duration;
        object["frames"] = animation.frames;

        json_animations.push_back(object);
    }

    nlohmann::json& json_frame_offsets = json["frames_offsets"];
    for(size_t index = 0; index < sprite_data->frames.size(); ++index)
    {
        const mono::SpriteFrame& frame = sprite_data->frames[index];
        json_frame_offsets[index]["x"] = frame.center_offset.x;
        json_frame_offsets[index]["y"] = frame.center_offset.y;
    }

    const std::string& serialized_sprite = json.dump(4);
    file::FilePtr output_file = file::CreateAsciiFile(sprite_file);
    std::fwrite(serialized_sprite.data(), serialized_sprite.length(), sizeof(char), output_file.get());
}

bool animator::LoadAllSprites(const char* all_sprites_file)
{
    file::FilePtr file = file::OpenAsciiFile(all_sprites_file);
    if(!file)
        return false;

    const std::vector<byte> file_data = file::FileRead(file);
    const nlohmann::json& json = nlohmann::json::parse(file_data);
    for(const auto& list_entry : json["all_sprites"])
    {
        const std::string sprite_string = list_entry;
        const size_t slash_pos = sprite_string.find_last_of('/') + 1;
        const std::string sprite_name = sprite_string.substr(slash_pos);

        g_all_sprite_files.push_back(sprite_name);
    }

    return true;
}

const std::vector<std::string>& animator::GetAllSprites()
{
    return g_all_sprite_files;
}
