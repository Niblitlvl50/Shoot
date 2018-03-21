
#include "TestZone.h"

#include "Factories.h"
#include "Enemies/Enemy.h"
#include "AIKnowledge.h"

#include "Effects/SmokeEffect.h"
#include "Effects/GibSystem.h"
#include "Hud/FPSElement.h"
#include "Hud/PlayerStatsElement.h"
#include "Hud/Overlay.h"
#include "Explosion.h"

#include "EventHandler/EventHandler.h"
#include "Events/GameEventFuncFwd.h"
#include "Events/SpawnEntityEvent.h"
#include "Events/SpawnPhysicsEntityEvent.h"
#include "Events/RemoveEntityEvent.h"
#include "Events/ShockwaveEvent.h"
#include "Events/DamageEvent.h"
#include "Events/SpawnConstraintEvent.h"

#include "EntityProperties.h"
#include "RenderLayers.h"

#include "WorldFile.h"
#include "World.h"

#include "Navigation/NavMesh.h"
#include "Navigation/NavmeshFactory.h"
#include "Navigation/NavMeshVisualizer.h"

#include "UpdateTasks/ListenerPositionUpdater.h"
#include "UpdateTasks/HealthbarUpdater.h"
#include "UpdateTasks/CameraViewportReporter.h"

#include "Audio/AudioFactory.h"
#include "Physics/IBody.h"

using namespace game;

TestZone::TestZone(mono::EventHandler& eventHandler)
    : PhysicsZone(math::Vector(0.0f, 0.0f), 0.8f),
      m_event_handler(eventHandler),
      m_dispatcher(std::make_shared<MessageDispatcher>())
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

    mSpawnEntityToken = m_event_handler.AddListener(spawnEntityFunc);
    mSpawnPhysicsEntityToken = m_event_handler.AddListener(spawnPhysicsFunc);
    mRemoveEntityByIdToken = m_event_handler.AddListener(removeFunc);
    mShockwaveEventToken = m_event_handler.AddListener(shockwaveFunc);
    mDamageEventToken = m_event_handler.AddListener(damageFunc);
    m_spawnConstraintToken = m_event_handler.AddListener(constraintFunc);
    m_despawnConstraintToken = m_event_handler.AddListener(despawnConstraintFunc);

    m_backgroundMusic = mono::AudioFactory::CreateSound("res/sound/ingame_phoenix.wav", true, true);
}

TestZone::~TestZone()
{
    m_event_handler.RemoveListener(mSpawnEntityToken);
    m_event_handler.RemoveListener(mSpawnPhysicsEntityToken);
    m_event_handler.RemoveListener(mRemoveEntityByIdToken);
    m_event_handler.RemoveListener(mShockwaveEventToken);
    m_event_handler.RemoveListener(mDamageEventToken);
    m_event_handler.RemoveListener(m_spawnConstraintToken);
    m_event_handler.RemoveListener(m_despawnConstraintToken);
}

void TestZone::OnLoad(mono::ICameraPtr& camera)
{
    {
        File::FilePtr world_file = File::OpenBinaryFile("res/world.world");
        
        world::LevelFileHeader world_header;
        world::ReadWorld(world_file, world_header);
        game::LoadWorld(this, world_header.polygons);
        
        // Nav mesh
        m_navmesh.points = game::GenerateMeshPoints(math::Vector(-100, -50), 200, 150, 3, world_header.polygons);
        m_navmesh.nodes = game::GenerateMeshNodes(m_navmesh.points, 5, world_header.polygons);
        game::g_navmesh = &m_navmesh;
        
        //AddDrawable(std::make_shared<NavmeshVisualizer>(m_navmesh, m_event_handler), BACKGROUND);
    }

    {
        File::FilePtr world_objects_file = File::OpenAsciiFile("res/world.objects.bin");

        world::WorldObjectsHeader world_objects_header;
        world::ReadWorldObjects2(world_objects_file, world_objects_header);

        std::vector<EnemyPtr> enemies;
        std::vector<SpawnPoint> spawn_points;
        std::vector<math::Vector> player_points;

        game::LoadWorldObjects(world_objects_header.objects, enemy_factory, enemies, spawn_points, player_points);

        for(const auto& enemy : enemies)
            AddPhysicsEntityWithCallback(enemy, MIDDLEGROUND, nullptr);

        m_spawner = std::make_unique<Spawner>(spawn_points, m_event_handler);
        m_player_daemon = std::make_unique<PlayerDaemon>(player_points, m_event_handler);
        m_player_daemon->SetCamera(camera);
        m_player_daemon->SpawnPlayer1();
    }

    AddUpdatable(std::make_shared<ListenerPositionUpdater>());
    AddUpdatable(std::make_shared<CameraViewportReporter>(camera));
    AddUpdatable(std::make_shared<HealthbarUpdater>(m_healthbars, m_damageController, *this));
    
    auto hud_overlay = std::make_shared<UIOverlayDrawer>();
    hud_overlay->AddElement(std::make_unique<FPSElement>(math::Vector(10, 10)));
    hud_overlay->AddElement(std::make_unique<PlayerStatsElement>(player_one, math::Vector(10, 0)));
    hud_overlay->AddElement(std::make_unique<PlayerStatsElement>(player_two, math::Vector(200, 0)));
    
    AddDrawable(hud_overlay, FOREGROUND);
    AddDrawable(std::make_shared<HealthbarDrawer>(m_healthbars), FOREGROUND);
    AddEntityWithCallback(std::make_shared<SmokeEffect>(math::Vector(-10.0f, 10.0f)), BACKGROUND, nullptr);
    AddUpdatable(m_dispatcher);

    m_gib_system = std::make_shared<GibSystem>();
    AddDrawable(m_gib_system, BACKGROUND);
    AddUpdatable(m_gib_system);

    //m_backgroundMusic->Play();
}

int TestZone::OnUnload()
{
    //m_beacon.Stop();
    game::g_navmesh = nullptr;
    return 0;
}

bool TestZone::SpawnEntity(const game::SpawnEntityEvent& event)
{
    AddEntityWithCallback(event.entity, event.layer, event.destroyed_func);
    return true;
}

bool TestZone::SpawnPhysicsEntity(const game::SpawnPhysicsEntityEvent& event)
{
    AddPhysicsEntityWithCallback(event.entity, event.layer, event.destroyed_func);
    return true;
}

bool TestZone::OnRemoveEntity(const game::RemoveEntityEvent& event)
{
    mono::IPhysicsEntityPtr physics_entity = FindPhysicsEntityFromId(event.id);
    if(physics_entity)
    {
        SchedulePreFrameTask(std::bind(&TestZone::RemovePhysicsEntity, this, physics_entity));
        return true;
    }

    mono::IEntityPtr entity = FindEntityFromId(event.id);
    if(entity)
        SchedulePreFrameTask(std::bind(&TestZone::RemoveEntity, this, entity));

    return true;
}

bool TestZone::OnShockwaveEvent(const game::ShockwaveEvent& event)
{
    const auto shockwave_func = [&event](const mono::IBodyPtr& body) {
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

    const DamageResult& result = m_damageController.ApplyDamage(entity->Id(), event.damage);
    if(!result.success)
        return false;

    if(result.health_left <= 0)
    {
        m_damageController.RemoveRecord(entity->Id());
        m_gib_system->EmitGibsAt(entity->Position(), event.direction);
        SchedulePreFrameTask(std::bind(&TestZone::RemovePhysicsEntity, this, entity));
    }

    return true;
}

void TestZone::AddEntityWithCallback(const mono::IEntityPtr& entity, int layer, DestroyedFunction destroyed_func)
{
    const bool damagable = entity->HasProperty(EntityProperties::DAMAGABLE);
    if(damagable)
        m_damageController.CreateRecord(entity->Id(), nullptr);

    PhysicsZone::AddEntity(entity, layer);
}

void TestZone::AddPhysicsEntityWithCallback(const mono::IPhysicsEntityPtr& entity, int layer, DestroyedFunction destroyed_func)
{
    const bool damagable = entity->HasProperty(EntityProperties::DAMAGABLE);
    if(damagable)
        m_damageController.CreateRecord(entity->Id(), destroyed_func);

    PhysicsZone::AddPhysicsEntity(entity, layer);
}

void TestZone::RemovePhysicsEntity(const mono::IPhysicsEntityPtr& entity)
{
    const bool damagable = entity->HasProperty(EntityProperties::DAMAGABLE);
    if(damagable)
        m_damageController.RemoveRecord(entity->Id());

    PhysicsZone::RemovePhysicsEntity(entity);
}

void TestZone::RemoveEntity(const mono::IEntityPtr& entity)
{
    PhysicsZone::RemoveEntity(entity);

    const bool damagable = entity->HasProperty(EntityProperties::DAMAGABLE);
    if(damagable)
        m_damageController.RemoveRecord(entity->Id());
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
