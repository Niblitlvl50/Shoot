
#include "SummonerController.h"

#include "Math/MathFunctions.h"
#include "Physics/PhysicsSystem.h"
#include "Physics/IBody.h"
#include "Rendering/Sprite/ISprite.h"
#include "Rendering/Sprite/Sprite.h"
#include "Rendering/Sprite/SpriteSystem.h"
#include "Rendering/Lights/LightSystem.h"
#include "TransformSystem/TransformSystem.h"
#include "EntitySystem/EntitySystem.h"
#include "EntitySystem/Entity.h"
#include "Debug/IDebugDrawer.h"
#include "System/File.h"
#include "Util/Random.h"
#include "SystemContext.h"

#include "Entity/TargetSystem.h"
#include "Navigation/NavigationSystem.h"

#include "nlohmann/json.hpp"

#include <algorithm>

namespace tweak_values
{
    constexpr float activate_distance = 9.0f;
    constexpr float preferred_distance = 5.5f;
    constexpr float prepare_duration_s = 1.5f;
    constexpr float summon_cooldown_s = 7.0f;
    constexpr float summon_radius = 1.5f;
    constexpr float move_speed = 0.6f;
    constexpr float degrees_per_second = 180.0f;
}

using namespace game;

SummonerController::SummonerController(uint32_t entity_id, mono::SystemContext* system_context, mono::EventHandler* event_handler)
    : m_entity_id(entity_id)
    , m_max_minions(3)
    , m_prepare_timer_s(0.0f)
    , m_cooldown_timer_s(0.0f)
{
    m_transform_system = system_context->GetSystem<mono::TransformSystem>();
    m_navigation_system = system_context->GetSystem<game::NavigationSystem>();
    m_target_system = system_context->GetSystem<game::TargetSystem>();

    mono::EntitySystem* entity_system = system_context->GetSystem<mono::EntitySystem>();
    m_entity_manager = entity_system;

    mono::PhysicsSystem* physics_system = system_context->GetSystem<mono::PhysicsSystem>();
    mono::IBody* body = physics_system->GetBody(entity_id);

    m_homing_movement.SetBody(body);
    m_homing_movement.SetForwardVelocity(tweak_values::move_speed);
    m_homing_movement.SetAngularVelocity(tweak_values::degrees_per_second);

    m_tracking_movement.Init(body, m_navigation_system);
    m_tracking_movement.SetTrackingSpeed(tweak_values::move_speed);

    mono::SpriteSystem* sprite_system = system_context->GetSystem<mono::SpriteSystem>();
    m_sprite = sprite_system->GetSprite(entity_id);

    m_idle_anim_id = m_sprite->GetAnimationIdFromName("idle");
    m_cast_anim_id = m_sprite->GetAnimationIdFromName("attack");
    if(m_cast_anim_id == -1)
        m_cast_anim_id = m_idle_anim_id;

    file::FilePtr config_file = file::OpenAsciiFile("res/configs/summoner_config.json");
    if(config_file)
    {
        const std::vector<byte>& file_data = file::FileRead(config_file);
        const nlohmann::json& json = nlohmann::json::parse(file_data);
        m_minion_entity_file = json.value("minion_entity", "res/entities/monster_bat_medium.entity");
        m_max_minions = json.value("max_minions", 3);
    }
    else
    {
        m_minion_entity_file = "res/entities/monster_bat_medium.entity";
    }

    const SummonerStateMachine::StateTable state_table = {
        SummonerStateMachine::MakeState(States::IDLE,    &SummonerController::ToIdle,    &SummonerController::Idle,    this),
        SummonerStateMachine::MakeState(States::PREPARE, &SummonerController::ToPrepare, &SummonerController::Prepare, this),
        SummonerStateMachine::MakeState(States::SUMMON,  &SummonerController::ToSummon,  &SummonerController::Summon,  this),
        SummonerStateMachine::MakeState(States::COOLDOWN,&SummonerController::ToCooldown,&SummonerController::Cooldown,this),
    };
    m_states.SetStateTableAndState(state_table, States::IDLE);
}

void SummonerController::Update(const mono::UpdateContext& update_context)
{
    m_states.UpdateState(update_context);
}

void SummonerController::DrawDebugInfo(IDebugDrawer* debug_drawer) const
{
    const math::Vector world_position = m_transform_system->GetWorldPosition(m_entity_id);
    debug_drawer->DrawCircle(world_position, tweak_values::activate_distance, mono::Color::GREEN);
    debug_drawer->DrawCircle(world_position, tweak_values::preferred_distance, mono::Color::CYAN);
    debug_drawer->DrawCircle(world_position, tweak_values::summon_radius, mono::Color::MAGENTA);

    const char* state_string = nullptr;
    switch(m_states.ActiveState())
    {
    case States::IDLE:     state_string = "Idle"; break;
    case States::PREPARE:  state_string = "Prepare"; break;
    case States::SUMMON:   state_string = "Summon"; break;
    case States::COOLDOWN: state_string = "Cooldown"; break;
    }
    debug_drawer->DrawWorldText(state_string, world_position, mono::Color::OFF_WHITE);
}

const char* SummonerController::GetDebugCategory() const
{
    return "Summoner";
}

int SummonerController::CountActiveMinions() const
{
    int count = 0;
    for(uint32_t minion_id : m_summoned_ids)
    {
        const mono::Entity* entity = m_entity_manager->GetEntity(minion_id);
        if(entity && entity->id != mono::INVALID_ID)
            count++;
    }
    return count;
}

void SummonerController::ToIdle()
{
    m_sprite->SetAnimation(m_idle_anim_id);
    m_sprite->SetShade(mono::Color::WHITE);

    // Clean up dead minion IDs.
    const auto is_dead = [this](uint32_t id) {
        const mono::Entity* entity = m_entity_manager->GetEntity(id);
        return !entity || entity->id == mono::INVALID_ID;
    };
    m_summoned_ids.erase(std::remove_if(m_summoned_ids.begin(), m_summoned_ids.end(), is_dead), m_summoned_ids.end());
}

void SummonerController::Idle(const mono::UpdateContext& update_context)
{
    const math::Vector world_position = m_transform_system->GetWorldPosition(m_entity_id);

    m_player_target = m_target_system->AquireTarget(TargetFaction::Player, world_position, tweak_values::activate_distance);
    if(!m_player_target->IsValid())
        return;

    // Move toward player until within preferred distance.
    const float distance = math::DistanceBetween(world_position, m_player_target->Position());
    if(distance > tweak_values::preferred_distance)
    {
        m_tracking_movement.Run(update_context, m_player_target->Position());
        return;
    }

    // In range — summon if we have room and the cooldown is done.
    if(m_cooldown_timer_s <= 0.0f && CountActiveMinions() < m_max_minions)
        m_states.TransitionTo(States::PREPARE);
}

void SummonerController::ToPrepare()
{
    m_prepare_timer_s = 0.0f;
    m_sprite->SetShade(mono::Color::MAGENTA);
    m_sprite->SetAnimation(m_cast_anim_id);
}

void SummonerController::Prepare(const mono::UpdateContext& update_context)
{
    m_prepare_timer_s += update_context.delta_s;
    if(m_prepare_timer_s >= tweak_values::prepare_duration_s)
        m_states.TransitionTo(States::SUMMON);
}

void SummonerController::ToSummon()
{
    const math::Vector world_position = m_transform_system->GetWorldPosition(m_entity_id);
    const float angle = mono::Random(0.0f, 6.2831853f);
    const float r = mono::Random(0.5f, tweak_values::summon_radius);
    const math::Vector spawn_pos = world_position + math::Vector(std::cos(angle) * r, std::sin(angle) * r);

    const mono::Entity minion = m_entity_manager->SpawnEntity(m_minion_entity_file.c_str());
    m_transform_system->SetTransform(minion.id, math::CreateMatrixWithPosition(spawn_pos), mono::TransformState::CLIENT);
    m_entity_manager->SetLifetimeDependency(m_entity_id, minion.id);
    m_summoned_ids.push_back(minion.id);

    m_states.TransitionTo(States::COOLDOWN);
}

void SummonerController::Summon(const mono::UpdateContext& update_context)
{ }

void SummonerController::ToCooldown()
{
    m_cooldown_timer_s = tweak_values::summon_cooldown_s;
    m_sprite->SetShade(mono::Color::WHITE);
    m_sprite->SetAnimation(m_idle_anim_id);
}

void SummonerController::Cooldown(const mono::UpdateContext& update_context)
{
    m_cooldown_timer_s -= update_context.delta_s;
    if(m_cooldown_timer_s <= 0.0f)
        m_states.TransitionTo(States::IDLE);
}
