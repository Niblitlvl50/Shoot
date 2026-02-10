
#include "WriteSpriteFile.h"
#include "System/File.h"

#include "Rendering/RenderSystem.h"
#include "Rendering/Sprite/Serialize.h"
#include "nlohmann/json.hpp"

namespace
{
    std::vector<std::string> g_all_sprite_files;
}

void animator::WriteSpriteFile(const char* sprite_file, const mono::SpriteData* sprite_data)
{
    mono::SpriteData local_sprite_data = *sprite_data;
    for(auto& frame : local_sprite_data.frames)
    {
        const float ppm = mono::RenderSystem::PixelsPerMeter();
        frame.size = (frame.size * ppm);
        frame.uv_upper_left = (frame.uv_upper_left * local_sprite_data.texture_size);
        frame.uv_lower_right = (frame.uv_lower_right * local_sprite_data.texture_size);
    }

    nlohmann::json sprite_data_json = local_sprite_data;

    const std::string& serialized_sprite = sprite_data_json.dump(4);
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
