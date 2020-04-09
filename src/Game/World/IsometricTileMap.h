
#pragma once

#include <vector>
#include <string>

#include "Rendering/RenderFwd.h"
#include "Rendering/BufferFactory.h"

namespace game
{
    struct Tile
    {
        int sort_index;
        std::string filename;
    };

    struct TileSet
    {
        std::string name;
        int tile_height;
        int tile_width;
        std::vector<Tile> tile_files;
    };

    struct TileLayer
    {
        std::string name;
        bool visible;
        float opacity;
        std::vector<int> tiles_indices;
    };

    struct TileSetReference
    {
        int first_gid;
        std::string tileset_file;
    };

    struct TileMap
    {
        int width;
        int height;
        int tile_height;
        int tile_width;
        std::vector<TileLayer> tile_layers;
        std::vector<TileSetReference> tileset_references;
    };

    TileMap LoadTilemap(const char* tilemap_file);
    TileSet LoadTileset(const char* tileset_file);

    struct IsometricDrawContext
    {
        std::unique_ptr<mono::IRenderBuffer> vertices;
        std::unique_ptr<mono::IRenderBuffer> texture_coordinates;
    };

    IsometricDrawContext CreateIsometricDrawContext(const TileMap& tilemap, const std::vector<TileSet>& tilesets);
    void DrawTilemap(mono::IRenderer& renderer, const TileMap& tilemap, const std::vector<TileSet>& tilesets);
}
