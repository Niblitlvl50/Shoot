
#include "TestZone.h"

#include "Factories.h"
#include "Enemies/Enemy.h"

#include "Effects/SmokeEffect.h"
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

#include "UpdateTasks/ListenerPositionUpdater.h"
#include "UpdateTasks/HealthbarUpdater.h"
#include "UpdateTasks/CameraViewportReporter.h"

#include "Audio/AudioFactory.h"
#include "Physics/IBody.h"

using namespace game;

TestZone::TestZone(mono::EventHandler& eventHandler)
    : PhysicsZone(math::Vector(0.0f, 0.0f), 0.9f),
      mEventHandler(eventHandler)
{
    using namespace std::placeholders;
    
    const game::SpawnEntityFunc& spawnEntityFunc = std::bind(&TestZone::SpawnEntity, this, _1);
    const game::SpawnPhysicsEntityFunc& spawnPhysicsFunc = std::bind(&TestZone::SpawnPhysicsEntity, this, _1);
    const game::RemoveEntityFunc& removeFunc = std::bind(&TestZone::OnRemoveEntity, this, _1);
    const game::ShockwaveFunc& shockwaveFunc = std::bind(&TestZone::OnShockwaveEvent, this, _1);
    const game::DamageFunc& damageFunc = std::bind(&TestZone::OnDamageEvent, this, _1);
    const game::SpawnConstraintFunc& constraintFunc = std::bind(&TestZone::OnSpawnConstraint, this, _1);
    const game::DespawnConstraintFunc& despawnConstraintFunc = std::bind(&TestZone::OnDespawnConstraint, this, _1);

    mSpawnEntityToken = mEventHandler.AddListener(spawnEntityFunc);
    mSpawnPhysicsEntityToken = mEventHandler.AddListener(spawnPhysicsFunc);
    mRemoveEntityByIdToken = mEventHandler.AddListener(removeFunc);
    mShockwaveEventToken = mEventHandler.AddListener(shockwaveFunc);
    mDamageEventToken = mEventHandler.AddListener(damageFunc);
    m_spawnConstraintToken = mEventHandler.AddListener(constraintFunc);
    m_despawnConstraintToken = mEventHandler.AddListener(despawnConstraintFunc);

    m_backgroundMusic = mono::AudioFactory::CreateSound("res/sound/ingame_phoenix.wav", true, true);
}

TestZone::~TestZone()
{
    mEventHandler.RemoveListener(mSpawnEntityToken);
    mEventHandler.RemoveListener(mSpawnPhysicsEntityToken);
    mEventHandler.RemoveListener(mRemoveEntityByIdToken);
    mEventHandler.RemoveListener(mShockwaveEventToken);
    mEventHandler.RemoveListener(mDamageEventToken);
    mEventHandler.RemoveListener(m_spawnConstraintToken);
    mEventHandler.RemoveListener(m_despawnConstraintToken);
}

void TestZone::OnLoad(mono::ICameraPtr& camera)
{
    File::FilePtr world_file = File::OpenBinaryFile("res/world.world");
    
    world::LevelFileHeader world_header;
    world::ReadWorld(world_file, world_header);
    game::LoadWorld(this, world_header.polygons);

    File::FilePtr world_objects_file = File::OpenAsciiFile("res/world.objects.bin");

    world::WorldObjectsHeader world_objects_header;
    world::ReadWorldObjects2(world_objects_file, world_objects_header);

    std::vector<EnemyPtr> enemies;
    std::vector<SpawnPoint> spawn_points;
    std::vector<math::Vector> player_points;

    game::LoadWorldObjects(world_objects_header.objects, enemy_factory, enemies, spawn_points, player_points);

    for(const auto& enemy : enemies)
        AddPhysicsEntity(enemy, MIDDLEGROUND);

    m_spawner = std::make_unique<Spawner>(spawn_points, mEventHandler);
    m_player_daemon = std::make_unique<PlayerDaemon>(player_points, mEventHandler, this);
    m_player_daemon->SetCamera(camera);    

    AddUpdatable(std::make_shared<ListenerPositionUpdater>());
    AddUpdatable(std::make_shared<CameraViewportReporter>(camera));
    AddUpdatable(std::make_shared<HealthbarUpdater>(m_healthbars, m_damageController, *this));
    
    auto hud_overlay = std::make_shared<UIOverlayDrawer>();
    hud_overlay->AddElement(std::make_unique<FPSElement>());
    hud_overlay->AddElement(std::make_unique<PlayerStatsElement>(player_one, math::Vector(10, 0)));
    hud_overlay->AddElement(std::make_unique<PlayerStatsElement>(player_two, math::Vector(200, 0)));
    
    AddDrawable(hud_overlay, FOREGROUND);
    AddDrawable(std::make_shared<HealthbarDrawer>(m_healthbars), FOREGROUND);

    AddEntity(std::make_shared<SmokeEffect>(math::Vector(-10.0f, 10.0f)), BACKGROUND);

    m_backgroundMusic->Play();
}

void TestZone::OnUnload()
{ }

bool TestZone::SpawnEntity(const game::SpawnEntityEvent& event)
{
    AddEntity(event.entity, FOREGROUND);
    return true;
}

bool TestZone::SpawnPhysicsEntity(const game::SpawnPhysicsEntityEvent& event)
{
    AddPhysicsEntity(event.entity, FOREGROUND);
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
        SchedulePreFrameTask(std::bind(&TestZone::RemovePhysicsEntity, this, entity));

        game::ExplosionConfiguration config;
        config.position = entity->Position();
        config.scale = 1.5f;
        config.sprite_file = "res/sprites/explosion.sprite";

        AddEntity(std::make_shared<Explosion>(config, mEventHandler), FOREGROUND);
    }

    return true;
}

void TestZone::AddPhysicsEntity(const mono::IPhysicsEntityPtr& entity, int layer)
{
    const bool damagable = entity->HasProperty(EntityProperties::DAMAGABLE);
    if(damagable)
        m_damageController.CreateRecord(entity->Id());

    PhysicsZone::AddPhysicsEntity(entity, layer);
}

void TestZone::RemovePhysicsEntity(const mono::IPhysicsEntityPtr& entity)
{
    const bool damagable = entity->HasProperty(EntityProperties::DAMAGABLE);
    if(damagable)
        m_damageController.RemoveRecord(entity->Id());

    PhysicsZone::RemovePhysicsEntity(entity);
}

void TestZone::AddEntity(const mono::IEntityPtr& entity, int layer)
{
    PhysicsZone::AddEntity(entity, layer);

    const bool damagable = entity->HasProperty(EntityProperties::DAMAGABLE);
    if(damagable)
        m_damageController.CreateRecord(entity->Id());
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
