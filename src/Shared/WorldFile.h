
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
        int version = 1;
        std::vector<PolygonData> polygons;
        std::vector<PrefabData> prefabs;
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

    bool WriteWorld(File::FilePtr& file, const LevelFileHeader& level);
    bool ReadWorld(const File::FilePtr& file, LevelFileHeader& level);

    bool WriteWorldObjects(File::FilePtr& file, const std::vector<WorldObject>& objects);
    bool ReadWorldObjects(const File::FilePtr& file, std::vector<WorldObject>& objects);

    bool WriteWorldObjectsBinary(File::FilePtr& file, const WorldObjectsHeader& objects);
    bool ReadWorldObjectsBinary(const File::FilePtr& file, WorldObjectsHeader& objects);
}
