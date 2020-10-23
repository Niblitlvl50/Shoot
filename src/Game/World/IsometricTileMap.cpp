
#include "IsometricTileMap.h"
#include "System/File.h"
#include "nlohmann/json.hpp"

game::TileMap game::LoadTilemap(const char* tilemap_file)
{
    file::FilePtr file = file::OpenAsciiFile(tilemap_file);
    if(!file)
        return {};

    const std::vector<byte> file_data = file::FileRead(file);
    const nlohmann::json& json = nlohmann::json::parse(file_data);

    game::TileMap tilemap;
    tilemap.width = json.value("width", 0);
    tilemap.height = json.value("height", 0);
    tilemap.tile_width = json.value("tilewidth", 0);
    tilemap.tile_height = json.value("tileheight", 0);

    const nlohmann::json& layers = json["layers"];
    tilemap.tile_layers.reserve(layers.size());

    for(const auto& layer_json : layers)
    {
        game::TileLayer layer;
        layer.name = layer_json.value("name", "");
        layer.visible = layer_json.value("visible", true);
        layer.opacity = layer_json.value("opacity", 1.0f);
        layer.tiles_indices = layer_json.value("data", std::vector<int>());

        tilemap.tile_layers.push_back(std::move(layer));
    }

    const nlohmann::json& tilesets = json["tilesets"];
    tilemap.tileset_references.reserve(tilesets.size());

    for(const auto& tileset : tilesets)
    {
        game::TileSetReference tileset_reference;
        tileset_reference.first_gid = tileset.value("firstgid", 0);
        tileset_reference.tileset_file = tileset.value("source", "");

        tilemap.tileset_references.push_back(std::move(tileset_reference));
    }

    return tilemap;
}

game::TileSet game::LoadTileset(const char* tileset_file)
{
    file::FilePtr file = file::OpenAsciiFile(tileset_file);
    if(!file)
        return {};

    const std::vector<byte> file_data = file::FileRead(file);
    const nlohmann::json& json = nlohmann::json::parse(file_data);

    game::TileSet tileset;
    tileset.name = json.value("name", "");
    tileset.tile_width = json.value("tilewidth", 0);
    tileset.tile_height = json.value("tileheight", 0);
    
    const nlohmann::json& tiles_json = json["tiles"];
    tileset.tile_files.reserve(tiles_json.size());

    for(const auto& tile_json : tiles_json.items())
    {
        game::Tile tile;
        tile.sort_index = std::stoi(tile_json.key());
        tile.filename = tile_json.value().value("image", "");
        tileset.tile_files.push_back(std::move(tile));
    }

    const auto sort_by_index = [](const game::Tile& left, const game::Tile& right) {
        return left.sort_index < right.sort_index;
    };
    std::sort(tileset.tile_files.begin(), tileset.tile_files.end(), sort_by_index);

    return tileset;
}

game::IsometricDrawContext game::CreateIsometricDrawContext(const TileMap& tilemap, const std::vector<TileSet>& tilesets)
{
    game::IsometricDrawContext context;

    // for(const TileLayer& layer : tilemap.tile_layers)
    // {

    // }

    return context;
}

void game::DrawTilemap(mono::IRenderer& renderer, const TileMap& tilemap, const std::vector<TileSet>& tilesets)
{
    // for(const TileLayer& layer : tilemap.tile_layers)
    // {

    // }
}
