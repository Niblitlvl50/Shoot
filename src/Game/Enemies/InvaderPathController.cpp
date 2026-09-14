
#include "InvaderPathController.h"

#include "Entity/TargetSystem.h"
#include "Behaviour/PathBehaviour.h"
#include "Weapons/IWeapon.h"
#include "Weapons/WeaponSystem.h"

#include "Paths/IPath.h"
#include "Paths/PathSystem.h"
#include "Paths/PathFactory.h"
#include "Math/MathFunctions.h"
#include "Math/Matrix.h"

#include "SystemContext.h"
#include "TransformSystem/TransformSystem.h"
#include "Physics/PhysicsSystem.h"
#include "Rendering/Sprite/SpriteSystem.h"
#include "Rendering/Sprite/Sprite.h"
#include "Rendering/Sprite/SpriteProperties.h"
#include "Rendering/Color.h"
#include "Debug/IDebugDrawer.h"

namespace tweak_values
{
    constexpr float attack_distance = 5.0f;
    constexpr float path_speed = 1.0f;
}

using namespace game;

InvaderPathController::InvaderPathController(uint32_t entity_id, mono::SystemContext* system_context, mono::EventHandler* event_handler)
    : m_entity_id(entity_id)
    , m_fire_count(0)
    , m_fire_cooldown_s(0.0f)
{
    m_transform_system = system_context->GetSystem<mono::TransformSystem>();
    m_path_system = system_context->GetSystem<mono::PathSystem>();
    m_target_system = system_context->GetSystem<TargetSystem>();

    mono::PhysicsSystem* physics_system = system_context->GetSystem<mono::PhysicsSystem>();
    m_body = physics_system->GetBody(entity_id);

    mono::SpriteSystem* sprite_system = system_context->GetSystem<mono::SpriteSystem>();
    m_sprite = sprite_system->GetSprite(entity_id);

    game::WeaponSystem* weapon_system = system_context->GetSystem<game::WeaponSystem>();
    m_weapon = weapon_system->CreatePrimaryWeapon(entity_id, WeaponFaction::ENEMY);

    m_path_behaviour.Init(m_body);
    m_path_behaviour.SetTrackingSpeed(tweak_values::path_speed);

    m_spawn_position = m_transform_system->GetWorldPosition(entity_id);

    const InvaderStateMachine::StateTable state_table = {
        InvaderStateMachine::MakeState(States::FIND_PATH,   &InvaderPathController::ToFindPath,   &InvaderPathController::FindPath,   this),
        InvaderStateMachine::MakeState(States::FOLLOW_PATH, &InvaderPathController::ToFollowPath, &InvaderPathController::FollowPath, this),
    };
    m_states.SetStateTableAndState(state_table, States::FIND_PATH);
}

InvaderPathController::~InvaderPathController()
{ }

void InvaderPathController::Update(const mono::UpdateContext& update_context)
{
    m_states.UpdateState(update_context);
}

void InvaderPathController::DrawDebugInfo(IDebugDrawer* debug_drawer) const
{
    const math::Vector world_position = m_transform_system->GetWorldPosition(m_entity_id);

    const PathDebugData debug_data = m_path_behaviour.GetDebugData();
    if(debug_data.has_path)
    {
        debug_drawer->DrawLine(*debug_data.path_points, 1.0f, mono::Color::BLUE);
        debug_drawer->DrawPoint(debug_data.target_position, 8.0f, mono::Color::GREEN);
        debug_drawer->DrawLine(world_position, debug_data.target_position, 1.0f, mono::Color::GREEN);
    }

    debug_drawer->DrawCircle(world_position, tweak_values::attack_distance, mono::Color::RED);

    const char* state_string = (m_states.ActiveState() == States::FIND_PATH) ? "Find Path" : "Follow Path";
    debug_drawer->DrawWorldText(state_string, world_position, mono::Color::OFF_WHITE);
}

const char* InvaderPathController::GetDebugCategory() const
{
    return "Invader Path";
}

void InvaderPathController::ToFindPath()
{ }

void InvaderPathController::FindPath(const mono::UpdateContext& update_context)
{
    uint32_t best_entity_id = mono::INVALID_ID;
    float best_dist_sq = math::INF;

    m_path_system->ForEach([&](const mono::PathComponent& comp, uint32_t path_entity_id) {
        const math::Vector path_pos = m_transform_system->GetWorldPosition(path_entity_id);
        const float dist_sq = math::DistanceBetweenSquared(m_spawn_position, path_pos);
        if(dist_sq < best_dist_sq)
        {
            best_dist_sq = dist_sq;
            best_entity_id = path_entity_id;
        }
    });

    if(best_entity_id != mono::INVALID_ID)
    {
        const mono::PathComponent* comp = m_path_system->GetPath(best_entity_id);
        const math::Matrix path_transform = m_transform_system->GetWorld(best_entity_id);

        std::vector<math::Vector> world_points;
        world_points.reserve(comp->points.size());
        for(const math::Vector& local_pt : comp->points)
            world_points.push_back(math::Transformed(path_transform, local_pt));

        mono::IPathPtr path = mono::CreatePath(world_points, comp->type);
        m_path_behaviour.SetPath(std::move(path));
    }

    m_states.TransitionTo(States::FOLLOW_PATH);
}

void InvaderPathController::ToFollowPath()
{ }

void InvaderPathController::FollowPath(const mono::UpdateContext& update_context)
{
    m_path_behaviour.Run(update_context.delta_s);

    const math::Vector velocity = m_body->GetVelocity();
    const bool is_going_left = (velocity.x < 0.0f);
    is_going_left ?
        m_sprite->SetProperty(mono::SpriteProperty::FLIP_HORIZONTAL) : m_sprite->ClearProperty(mono::SpriteProperty::FLIP_HORIZONTAL);

    if(m_fire_cooldown_s > 0.0f)
    {
        m_fire_cooldown_s -= update_context.delta_s;
        return;
    }

    const math::Vector world_position = m_transform_system->GetWorldPosition(m_entity_id);
    m_aquired_target = m_target_system->AquireTarget(TargetFaction::Player, world_position, tweak_values::attack_distance);
    if(!m_aquired_target->IsValid())
        return;

    const bool did_fire = (m_weapon->Fire(world_position, m_aquired_target->Position(), update_context.timestamp) == WeaponState::FIRE);
    m_fire_count += did_fire ? 1 : 0;

    if(m_fire_count == 5)
    {
        m_fire_cooldown_s = 2.0f;
        m_fire_count = 0;
    }
}
