
#pragma once

#include "Math/Vector.h"
#include "System/File.h"
#include "ObjectAttribute.h"

#include <vector>

namespace world
{
    struct PolygonData
    {
        math::Vector position;
        math::Vector local_offset;
        float rotation = 0.0f;
        char texture[64] = { 0 };
        std::vector<math::Vector> vertices;
    };

    struct LevelFileHeader
    {
        int version = 0;
        std::vector<PolygonData> polygons;
    };

    struct WorldObject
    {
        char name[24] = { 0 };
        std::vector<ID_Attribute> attributes;
    };

    struct WorldObjectsHeader
    {
        int version = 0;
        std::vector<WorldObject> objects;
    };

    bool WriteWorld(File::FilePtr& file, const LevelFileHeader& level);
    bool ReadWorld(File::FilePtr& file, LevelFileHeader& level);

    bool WriteWorldObjects(File::FilePtr& file, const std::vector<WorldObject>& objects);
    bool ReadWorldObjects(File::FilePtr& file, std::vector<WorldObject>& objects);


    bool WriteWorldObjects2(File::FilePtr& file, const WorldObjectsHeader& objects);
    bool ReadWorldObjects2(File::FilePtr& file, WorldObjectsHeader& objects);
}
