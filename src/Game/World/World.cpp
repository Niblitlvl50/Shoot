
#include "World.h"
#include "CollisionConfiguration.h"
#include "Enemies/Enemy.h"
#include "Pickups/Ammo.h"

#include "RenderLayers.h"
#include "ObjectAttribute.h"
#include "DefinedAttributes.h"
#include "Prefabs.h"

#include "StaticTerrainBlock.h"
#include "StaticPrefab.h"
#include "StaticBackground.h"

#include "Navigation/NavmeshFactory.h"

#include "Zone/IPhysicsZone.h"
#include "Math/Matrix.h"
#include "StringFunctions.h"

#include <algorithm>
#include <string>


void game::LoadWorld(
    mono::IPhysicsZone* zone,
    const std::vector<world::PolygonData>& polygons,
    const std::vector<world::PrefabData>& prefabs,
    std::vector<ExcludeZone>& exclude_zones)
{
    size_t count = 0;

    for(const world::PolygonData& polygon : polygons)
        count += polygon.vertices.size();

    auto static_terrain = std::make_shared<StaticTerrainBlock>(count, polygons.size());

    for(const world::PolygonData& polygon : polygons)
        static_terrain->AddPolygon(polygon);

    zone->AddDrawable(static_terrain, BACKGROUND);

    const std::vector<PrefabDefinition>& prefab_definitions = LoadPrefabDefinitions();

    for(const world::PrefabData& prefab : prefabs)
    {
        const PrefabDefinition* prefab_definition = FindPrefabFromName(prefab.name, prefab_definitions);

        std::vector<math::Vector> collision_polygon;
        collision_polygon.reserve(prefab_definition->collision_shape.size());

        math::Matrix translation;
        math::Translate(translation, prefab.position);

        const math::Matrix& transform = translation;

        for(const math::Vector& collision_vertex : prefab_definition->collision_shape)
            collision_polygon.push_back(math::Transform(transform, collision_vertex));

        zone->AddPhysicsEntity(std::make_shared<StaticPrefab>(
            prefab.position, prefab_definition->sprite_file.c_str(), collision_polygon), LayerId::PREFABS);
    
        ExcludeZone exclude_zone;
        exclude_zone.polygon_vertices = collision_polygon;
        exclude_zones.push_back(exclude_zone);
    }

    //zone->AddDrawable(std::make_unique<StaticBackground>(), BACKGROUND);
}

namespace
{
    void LoadAttributes(game::SpawnPoint& spawn_point, const std::vector<Attribute>& attributes)
    {
        world::FindAttribute(world::POSITION_ATTRIBUTE, attributes, spawn_point.position);
        world::FindAttribute(world::RADIUS_ATTRIBUTE, attributes, spawn_point.radius);
    }

    void LoadAttributes(game::EnemyPtr& enemy, const std::vector<Attribute>& attributes)
    {
        math::Vector position;
        float rotation = 0.0f;

        world::FindAttribute(world::POSITION_ATTRIBUTE, attributes, position);
        world::FindAttribute(world::ROTATION_ATTRIBUTE, attributes, rotation);

        enemy->SetPosition(position);
        enemy->SetRotation(rotation);
    }

    void LoadAttributes(game::Ammo& ammo_pickup, const std::vector<Attribute>& attributes)
    {
        world::FindAttribute(world::POSITION_ATTRIBUTE, attributes, ammo_pickup.position);
        world::FindAttribute(world::PICKUP_TYPE_ATTRIBUTE, attributes, ammo_pickup.type);
        world::FindAttribute(world::AMOUNT_ATTRIBUTE, attributes, ammo_pickup.value);
    }
}

void game::LoadWorldObjects(
    const std::vector<world::WorldObject>& objects,
    IEnemyFactory* enemy_factory,
    IGameObjectFactory* gameobject_factory,
    std::vector<game::EnemyPtr>& enemies,
    std::vector<mono::IPhysicsEntityPtr>& gameobjects,
    std::vector<SpawnPoint>& spawn_points,
    std::vector<math::Vector>& player_points,
    std::vector<game::Ammo>& ammo_pickups)
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
            world::FindAttribute(world::POSITION_ATTRIBUTE, object.attributes, position);
            player_points.push_back(position);
        }
        else if(name == "ammo_pickup")
        {
            game::Ammo ammo_pickup;
            LoadAttributes(ammo_pickup, object.attributes);
            ammo_pickups.push_back(ammo_pickup);
        }
        else
        {
            math::Vector position;
            world::FindAttribute(world::POSITION_ATTRIBUTE, object.attributes, position);

            game::EnemyPtr enemy = enemy_factory->CreateFromName(name.c_str(), position, object.attributes);
            if(enemy)
            {
                LoadAttributes(enemy, object.attributes);
                enemies.push_back(enemy);
            }
            else
            {
                mono::IPhysicsEntityPtr gameobject = gameobject_factory->CreateGameObject(name.c_str(), object.attributes);
                if(gameobject)
                {
                    gameobjects.push_back(gameobject);
                }
            }
        }
    }
}
