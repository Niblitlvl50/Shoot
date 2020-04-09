
#include "SpriteAtlasReader.h"
#include "System/File.h"
#include "Util/Hash.h"
#include "nlohmann/json.hpp"


game::SpriteAtlasData game::ReadSpriteAtlas(const char* filename)
{
    SpriteAtlasData data;

    file::FilePtr file = file::OpenAsciiFile(filename);
    if(!file)
        return data;

    std::vector<byte> file_data;
    file::FileRead(file, file_data);

    const nlohmann::json& json = nlohmann::json::parse(file_data);
    const auto meta_json = json["meta"];
    const auto frames_json = json["frames"];
    const auto size_json = meta_json["size"];

    data.meta.app = meta_json["app"];
    data.meta.version = meta_json["version"];
    data.meta.image_filename = meta_json["image"];
    data.meta.image_width = size_json["w"];
    data.meta.image_height = size_json["h"];

    data.frames.reserve(frames_json.size());

    for(const auto& frame_json : frames_json)
    {
        const auto& frame_frame_json = frame_json["frame"];

        SpriteAtlasFrame frame;
        frame.filename = frame_json["filename"];
        frame.filename_hash = mono::Hash(frame.filename.c_str());
        frame.xy = math::Vector(frame_frame_json["x"], frame_frame_json["y"]);
        frame.wh = math::Vector(frame_frame_json["w"], frame_frame_json["h"]);
        data.frames.push_back(frame);
    }

    return data;
}
