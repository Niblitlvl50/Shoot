
#include "WriteSpriteFile.h"
#include "System/File.h"

#include "nlohmann/json.hpp"

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