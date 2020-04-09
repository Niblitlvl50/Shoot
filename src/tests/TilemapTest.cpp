
#include "World/IsometricTileMap.h"
#include <gtest/gtest.h>

TEST(TilemapTest, LoadMap)
{
    const game::TileMap tilemap = game::LoadTilemap("res/tilemaps/prototype_map.json");

    ASSERT_EQ(tilemap.width, 16);
    ASSERT_EQ(tilemap.height, 16);
    ASSERT_EQ(tilemap.tile_width, 256);
    ASSERT_EQ(tilemap.tile_height, 149);

    const game::TileLayer& ground_layer = tilemap.tile_layers.front();
    ASSERT_EQ(ground_layer.name, "ground_layer");
    ASSERT_EQ(ground_layer.visible, true);
    ASSERT_EQ(ground_layer.opacity, 1.0f);
    ASSERT_EQ(ground_layer.tiles_indices.size(), size_t(tilemap.width * tilemap.height));

    const game::TileLayer& props_layer = tilemap.tile_layers.back();
    ASSERT_EQ(props_layer.name, "props_layer");
    ASSERT_EQ(props_layer.visible, true);
    ASSERT_FLOAT_EQ(props_layer.opacity, 0.32f);
    ASSERT_EQ(props_layer.tiles_indices.size(), size_t(tilemap.width * tilemap.height));

    ASSERT_EQ(tilemap.tileset_references.size(), size_t(2));

    const game::TileSetReference& first_tileset = tilemap.tileset_references.front();
    ASSERT_EQ(first_tileset.first_gid, 1);
    ASSERT_EQ(first_tileset.tileset_file, "prototype_tileset.json");

    const game::TileSetReference& last_tileset = tilemap.tileset_references.back();
    ASSERT_EQ(last_tileset.first_gid, 181);
    ASSERT_EQ(last_tileset.tileset_file, "another_tileset.json");
}

TEST(TilemapTest, LoadTileSet)
{
    const game::TileSet tileset = game::LoadTileset("res/tilemaps/prototype_tileset.json");

    ASSERT_EQ(tileset.name, "prototype_tileset");
    ASSERT_EQ(tileset.tile_height, 512);
    ASSERT_EQ(tileset.tile_width, 256);
    ASSERT_EQ(tileset.tile_files.size(), size_t(180));

    const game::Tile& first_tile = tileset.tile_files.front();
    ASSERT_EQ(first_tile.filename, "prototype_tiles/blockHuge_E.png");

    const game::Tile& fourtieth_tile = tileset.tile_files[40];
    ASSERT_EQ(fourtieth_tile.filename, "prototype_tiles/columnBlocks_E.png");
}
