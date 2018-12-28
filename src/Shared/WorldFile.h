
#pragma once

#include "Math/Vector.h"
#include "System/File.h"
#include "ObjectAttribute.h"

#include <vector>

namespace world
{
    constexpr size_t PolygonTextureNameMaxLength = 64;
    constexpr size_t WorldObjectNameMaxLength = 24;

    struct PolygonData
    {
        math::Vector position;
        math::Vector local_offset;
        float rotation = 0.0f;
        char texture[PolygonTextureNameMaxLength] = { 0 };
        std::vector<math::Vector> vertices;
    };

    struct PrefabData
    {
        char name[WorldObjectNameMaxLength] = { 0 };
        math::Vector position;
        math::Vector scale;
        float rotation = 0.0f;
    };

    struct LevelFileHeader
    {
        int version = 2;
        std::vector<PolygonData> polygons;
        std::vector<PrefabData> prefabs;
        std::vector<math::Vector> bounds;
    };

    struct WorldObject
    {
        char name[WorldObjectNameMaxLength] = { 0 };
        std::vector<Attribute> attributes;
    };

    struct WorldObjectsHeader
    {
        int version = 0;
        std::vector<WorldObject> objects;
    };

    bool WriteWorld(file::FilePtr& file, const LevelFileHeader& level);
    bool ReadWorld(const file::FilePtr& file, LevelFileHeader& level);

    bool WriteWorldObjects(file::FilePtr& file, const std::vector<WorldObject>& objects);
    bool ReadWorldObjects(const file::FilePtr& file, std::vector<WorldObject>& objects);

    bool WriteWorldObjectsBinary(file::FilePtr& file, const WorldObjectsHeader& objects);
    bool ReadWorldObjectsBinary(const file::FilePtr& file, WorldObjectsHeader& objects);
}
