
#include "World.h"
#include "CollisionConfiguration.h"
#include "Pickups/Ammo.h"

#include "RenderLayers.h"
#include "ObjectAttribute.h"
// #include "DefinedAttributes.h"
#include "Component.h"

#include "StaticTerrainBlock.h"
#include "StaticBackground.h"

#include "Navigation/NavmeshFactory.h"

//#include "Zone/IPhysicsZone.h"
#include "Math/Matrix.h"
#include "Util/StringFunctions.h"

#include <algorithm>
#include <string>


void game::LoadWorld(
    //mono::IPhysicsZone* zone,
    const std::vector<world::PolygonData>& polygons,
    std::vector<ExcludeZone>& exclude_zones)
{
    size_t count = 0;

    for(const world::PolygonData& polygon : polygons)
        count += polygon.vertices.size();

//    auto static_terrain = std::make_shared<StaticTerrainBlock>(count, polygons.size());
//    for(const world::PolygonData& polygon : polygons)
//        static_terrain->AddPolygon(polygon);
//    zone->AddDrawable(static_terrain, BACKGROUND);

    //zone->AddDrawable(std::make_unique<StaticBackground>(), BACKGROUND);
}

namespace
{
    void LoadAttributes(game::SpawnPoint& spawn_point, const std::vector<Attribute>& attributes)
    {
        FindAttribute(POSITION_ATTRIBUTE, attributes, spawn_point.position, FallbackMode::SET_DEFAULT);
        FindAttribute(RADIUS_ATTRIBUTE, attributes, spawn_point.radius, FallbackMode::SET_DEFAULT);
    }

    void LoadAttributes(game::Pickup& ammo_pickup, const std::vector<Attribute>& attributes)
    {
        FindAttribute(POSITION_ATTRIBUTE, attributes, ammo_pickup.position, FallbackMode::SET_DEFAULT);
        FindAttribute(PICKUP_TYPE_ATTRIBUTE, attributes, ammo_pickup.type, FallbackMode::SET_DEFAULT);
        FindAttribute(AMOUNT_ATTRIBUTE, attributes, ammo_pickup.value, FallbackMode::SET_DEFAULT);
    }
}

void game::LoadWorldObjects(
    const std::vector<world::WorldObject>& objects,
    std::vector<SpawnPoint>& spawn_points,
    std::vector<math::Vector>& player_points,
    std::vector<game::Pickup>& ammo_pickups)
{
    for(const world::WorldObject& object : objects)
    {
        const std::string name = object.name;
        if(name == "spawnpoint")
        {
            SpawnPoint spawn_point;
            LoadAttributes(spawn_point, object.attributes);
            spawn_points.push_back(spawn_point);
        }
        else if(name == "playerpoint")
        {
            math::Vector position;
            FindAttribute(POSITION_ATTRIBUTE, object.attributes, position, FallbackMode::SET_DEFAULT);
            player_points.push_back(position);
        }
        else if(name == "ammo_pickup")
        {
            game::Pickup ammo_pickup;
            LoadAttributes(ammo_pickup, object.attributes);
            ammo_pickups.push_back(ammo_pickup);
        }
        else
        {
        }
    }
}
