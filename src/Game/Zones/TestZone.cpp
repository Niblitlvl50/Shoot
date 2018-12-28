
#include "TestZone.h"

#include "Factories.h"
#include "Enemies/Enemy.h"
#include "AIKnowledge.h"
#include "DamageController.h"
#include "GameObjects/IGameObjectFactory.h"

#include "Effects/SmokeEffect.h"
#include "Effects/GibSystem.h"
#include "Effects/ParticleExplosion.h"
#include "Hud/FPSElement.h"
#include "Hud/WeaponStatusElement.h"
#include "Hud/Overlay.h"
#include "Hud/PickupDrawer.h"
#include "Hud/WaveDrawer.h"
#include "Explosion.h"

#include "EventHandler/EventHandler.h"
#include "Events/GameEventFuncFwd.h"
#include "Events/SpawnEntityEvent.h"
#include "Events/SpawnPhysicsEntityEvent.h"
#include "Events/RemoveEntityEvent.h"
#include "Events/ShockwaveEvent.h"
#include "Events/DamageEvent.h"
#include "Events/SpawnConstraintEvent.h"

#include "RenderLayers.h"

#include "WorldFile.h"
#include "World/World.h"

#include "Navigation/NavMesh.h"
#include "Navigation/NavmeshFactory.h"
#include "Navigation/NavMeshVisualizer.h"

#include "UpdateTasks/ListenerPositionUpdater.h"
#include "UpdateTasks/HealthbarUpdater.h"
#include "UpdateTasks/CameraViewportReporter.h"
#include "UpdateTasks/PickupUpdater.h"

#include "Audio/AudioFactory.h"
#include "Physics/IBody.h"

using namespace game;

TestZone::TestZone(const ZoneCreationContext& context)
    : PhysicsZone(math::Vector(0.0f, 0.0f), 0.25f)
    , m_event_handler(*context.event_handler)
    , m_damage_controller(context.damage_controller)
    , m_dispatcher(std::make_shared<MessageDispatcher>())
      //m_connection(m_dispatcher.get())
{
    using namespace std::placeholders;
    
    const game::SpawnEntityFunc& spawnEntityFunc = std::bind(&TestZone::SpawnEntity, this, _1);
    const game::SpawnPhysicsEntityFunc& spawnPhysicsFunc = std::bind(&TestZone::SpawnPhysicsEntity, this, _1);
    const game::RemoveEntityFunc& removeFunc = std::bind(&TestZone::OnRemoveEntity, this, _1);
    const game::ShockwaveFunc& shockwaveFunc = std::bind(&TestZone::OnShockwaveEvent, this, _1);
    const game::DamageFunc& damageFunc = std::bind(&TestZone::OnDamageEvent, this, _1);
    const game::SpawnConstraintFunc& constraintFunc = std::bind(&TestZone::OnSpawnConstraint, this, _1);
    const game::DespawnConstraintFunc& despawnConstraintFunc = std::bind(&TestZone::OnDespawnConstraint, this, _1);

    m_spawn_entity_token = m_event_handler.AddListener(spawnEntityFunc);
    m_spawn_physics_entity_token = m_event_handler.AddListener(spawnPhysicsFunc);
    m_remove_entity_by_id_token = m_event_handler.AddListener(removeFunc);
    m_shockwave_event_token = m_event_handler.AddListener(shockwaveFunc);
    m_damage_event_token = m_event_handler.AddListener(damageFunc);
    m_spawnConstraintToken = m_event_handler.AddListener(constraintFunc);
    m_despawnConstraintToken = m_event_handler.AddListener(despawnConstraintFunc);

    m_background_music = mono::AudioFactory::CreateSound("res/sound/ingame_phoenix.wav", true, true);
}

TestZone::~TestZone()
{
    m_event_handler.RemoveListener(m_spawn_entity_token);
    m_event_handler.RemoveListener(m_spawn_physics_entity_token);
    m_event_handler.RemoveListener(m_remove_entity_by_id_token);
    m_event_handler.RemoveListener(m_shockwave_event_token);
    m_event_handler.RemoveListener(m_damage_event_token);
    m_event_handler.RemoveListener(m_spawnConstraintToken);
    m_event_handler.RemoveListener(m_despawnConstraintToken);
}

void TestZone::OnLoad(mono::ICameraPtr& camera)
{
    AddUpdatable(m_dispatcher);
    AddUpdatable(std::make_shared<ListenerPositionUpdater>());
    AddUpdatable(std::make_shared<CameraViewportReporter>(camera));
    AddUpdatable(std::make_shared<HealthbarUpdater>(m_healthbars, *m_damage_controller, *this));
    AddUpdatable(std::make_shared<PickupUpdater>(m_pickups, m_event_handler));
    
    auto hud_overlay = std::make_shared<UIOverlayDrawer>();
    hud_overlay->AddChild(std::make_shared<WeaponStatusElement>(g_player_one, math::Vector(10.0f, 10.0f), math::Vector(-50.0f, 10.0f)));
    hud_overlay->AddChild(std::make_shared<WeaponStatusElement>(g_player_two, math::Vector(277.0f, 10.0f), math::Vector(320.0f, 10.0f)));
    hud_overlay->AddChild(std::make_shared<FPSElement>(math::Vector(2.0f, 2.0f)));
    AddEntity(hud_overlay, UI);
    
    AddDrawable(std::make_shared<HealthbarDrawer>(m_healthbars), LayerId::GAMEOBJECTS);
    AddDrawable(std::make_shared<PickupDrawer>(m_pickups), LayerId::GAMEOBJECTS);
    AddDrawable(std::make_shared<WaveDrawer>(m_event_handler), LayerId::UI);

    m_gib_system = std::make_shared<GibSystem>();
    AddDrawable(m_gib_system, BACKGROUND_DECALS);
    AddUpdatable(m_gib_system);

    {
        file::FilePtr world_file = file::OpenBinaryFile("res/world.world");
        world::LevelFileHeader world_header;
        world::ReadWorld(world_file, world_header);

        std::vector<ExcludeZone> exclude_zones;
        game::LoadWorld(this, world_header.polygons, world_header.prefabs, exclude_zones);

        // Nav mesh
        m_navmesh.points = game::GenerateMeshPoints(math::Vector(-100, -50), 200, 150, 3, exclude_zones);
        m_navmesh.nodes = game::GenerateMeshNodes(m_navmesh.points, 5, exclude_zones);
        game::g_navmesh = &m_navmesh;
        
        //AddDrawable(std::make_shared<NavmeshVisualizer>(m_navmesh, m_event_handler), UI);
    }

    {
        file::FilePtr world_objects_file = file::OpenAsciiFile("res/world.objects.bin");

        world::WorldObjectsHeader world_objects_header;
        world::ReadWorldObjectsBinary(world_objects_file, world_objects_header);

        std::vector<EnemyPtr> enemies;
        std::vector<mono::IPhysicsEntityPtr> gameobjects;
        std::vector<SpawnPoint> spawn_points;
        std::vector<math::Vector> player_points;

        game::LoadWorldObjects(
            world_objects_header.objects,
            enemy_factory, gameobject_factory,
            enemies, gameobjects, spawn_points, player_points, m_pickups);

        for(const auto& enemy : enemies)
            AddPhysicsEntity(enemy, LayerId::GAMEOBJECTS);

        for(const auto& gameobject : gameobjects)
            AddPhysicsEntity(gameobject, LayerId::GAMEOBJECTS);

        const std::vector<Wave>& waves = LoadWaveFile("res/waves/wave1.json");
        m_enemy_spawner = std::make_unique<Spawner>(spawn_points, waves, m_event_handler);
        m_player_daemon = std::make_unique<PlayerDaemon>(camera, player_points, m_event_handler);
    }

    //m_background_music->Play();   

    // Test stuff...
    AddEntity(std::make_shared<SmokeEffect>(math::Vector(-10.0f, 10.0f)), GAMEOBJECTS);
    AddEntity(std::make_shared<ParticleExplosion>(math::Vector(-20.0f, 10.0f)), GAMEOBJECTS);
}

void TestZone::Accept(mono::IRenderer& renderer)
{
    using LayerDrawable = std::pair<int, mono::IDrawablePtr>;

    const auto sort_on_y = [](const LayerDrawable& first, const LayerDrawable& second) {
        if(first.first == second.first)
            return first.second->BoundingBox().mA.y > second.second->BoundingBox().mA.y;
        
        return first.first < second.first;
    };

    std::sort(m_drawables.begin(), m_drawables.end(), sort_on_y);
    PhysicsZone::Accept(renderer);
}

int TestZone::OnUnload()
{
    //m_beacon.Stop();
    game::g_navmesh = nullptr;
    return 0;
}

bool TestZone::SpawnEntity(const game::SpawnEntityEvent& event)
{
    AddEntity(event.entity, event.layer);
    return true;
}

bool TestZone::SpawnPhysicsEntity(const game::SpawnPhysicsEntityEvent& event)
{
    AddPhysicsEntity(event.entity, event.layer);
    return true;
}

bool TestZone::OnRemoveEntity(const game::RemoveEntityEvent& event)
{
    mono::IPhysicsEntityPtr physics_entity = FindPhysicsEntityFromId(event.id);
    if(physics_entity)
    {
        RemovePhysicsEntity(physics_entity);
        return true;
    }

    mono::IEntityPtr entity = FindEntityFromId(event.id);
    if(entity)
        RemoveEntity(entity);

    return true;
}

bool TestZone::OnShockwaveEvent(const game::ShockwaveEvent& event)
{
    const auto shockwave_func = [&event](mono::IBody* body) {
        math::Vector unit = body->GetPosition() - event.position;
        const float length = math::Length(unit);
        if(length > 4)
            return;

        math::Normalize(unit);
        
        const math::Vector& impulse = unit * event.magnitude;
        body->ApplyImpulse(impulse, body->GetPosition());
    };

    PhysicsZone::ForEachBody(shockwave_func);

    return true;
}

bool TestZone::OnDamageEvent(const game::DamageEvent& event)
{
    mono::IPhysicsEntityPtr entity = FindPhysicsEntityFromBody(event.body);
    if(!entity)
        return false;

    const DamageResult& result = m_damage_controller->ApplyDamage(entity->Id(), event.damage);
    if(!result.success)
        return false;

    if(result.health_left <= 0)
    {
        m_damage_controller->RemoveRecord(entity->Id());
        m_enemy_spawner->EntityDestroyed(entity->Id());
        m_gib_system->EmitGibsAt(entity->Position(), event.direction);
        RemovePhysicsEntity(entity);
    }

    return true;
}

void TestZone::RemovePhysicsEntity(const mono::IPhysicsEntityPtr& entity)
{
    m_damage_controller->RemoveRecord(entity->Id());
    PhysicsZone::RemovePhysicsEntity(entity);
}

void TestZone::RemoveEntity(const mono::IEntityPtr& entity)
{
    m_damage_controller->RemoveRecord(entity->Id());
    PhysicsZone::RemoveEntity(entity);
}

bool TestZone::OnSpawnConstraint(const game::SpawnConstraintEvent& event)
{
    PhysicsZone::AddConstraint(event.constraint);
    return true;
}

bool TestZone::OnDespawnConstraint(const game::DespawnConstraintEvent& event)
{
    PhysicsZone::RemoveConstraint(event.constraint);
    return true;
}
