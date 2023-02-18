
#include "TeleportSystem.h"

#include "GameCamera/CameraSystem.h"
#include "TriggerSystem/TriggerSystem.h"
#include "TransformSystem/TransformSystem.h"

#include "Camera/ICamera.h"
#include "EntitySystem/Entity.h"
#include "Rendering/IRenderer.h"
#include "Math/EasingFunctions.h"

#include "Player/PlayerInfo.h"


namespace tweak_values
{
    constexpr float fade_duration_s = 1.0f;
}

using namespace game;

TeleportSystem::TeleportSystem(
    CameraSystem* camera_system, TriggerSystem* trigger_system, mono::TransformSystem* transform_system)
    : m_camera_system(camera_system)
    , m_trigger_system(trigger_system)
    , m_transform_system(transform_system)
{
    const TeleportStateMachine::StateTable state_table = {
        TeleportStateMachine::MakeState(States::IDLE, &TeleportSystem::ToIdle, &TeleportSystem::Idle, this),
        TeleportStateMachine::MakeState(States::FADE_OUT, &TeleportSystem::ToFadeOut, &TeleportSystem::FadeOut, this),
        TeleportStateMachine::MakeState(States::FADE_IN, &TeleportSystem::ToFadeIn, &TeleportSystem::FadeIn, this),
    };
    m_states.SetStateTableAndState(state_table, States::IDLE);
}

const char* TeleportSystem::Name() const
{
    return "teleportsystem";
}

void TeleportSystem::Update(const mono::UpdateContext& update_context)
{
    m_states.UpdateState(update_context);
}

void TeleportSystem::AllocateTeleportPlayer(uint32_t entity_id)
{
    m_teleport_infos[entity_id] = { mono::INVALID_ID, mono::INVALID_ID };
}

void TeleportSystem::ReleaseTeleportPlayer(uint32_t entity_id)
{
    TeleportInfo& teleport_info = m_teleport_infos[entity_id];
    if(teleport_info.trigger_handle != mono::INVALID_ID)
        m_trigger_system->RemoveTriggerCallback(teleport_info.trigger_hash, teleport_info.trigger_handle, mono::INVALID_ID);

    m_teleport_infos.erase(entity_id);
}

void TeleportSystem::UpdateTeleportPlayer(uint32_t entity_id, uint32_t trigger_hash)
{
    TeleportInfo& teleport_info = m_teleport_infos[entity_id];
    if(teleport_info.trigger_handle != mono::INVALID_ID)
        m_trigger_system->RemoveTriggerCallback(teleport_info.trigger_hash, teleport_info.trigger_handle, mono::INVALID_ID);

    const game::TriggerCallback trigger_callback = [this, entity_id](uint32_t trigger_id) {
        HandleTeleport(entity_id);
    };
    teleport_info.trigger_handle = m_trigger_system->RegisterTriggerCallback(trigger_hash, trigger_callback, mono::INVALID_ID);
}

void TeleportSystem::ToIdle()
{}
void TeleportSystem::Idle(const mono::UpdateContext& update_context)
{}

void TeleportSystem::ToFadeOut()
{
    m_fade_timer = 0.0f;
}
void TeleportSystem::FadeOut(const mono::UpdateContext& update_context)
{
    m_alpha = math::EaseOutCubic(m_fade_timer, tweak_values::fade_duration_s, 1.0f, -1.0f);
    m_fade_timer += update_context.delta_s;

    if(m_fade_timer > tweak_values::fade_duration_s)
        m_states.TransitionTo(States::FADE_IN);
}

void TeleportSystem::ToFadeIn()
{
    TeleportPlayers(m_saved_teleport_position);
    m_fade_timer = 0.0f;
}
void TeleportSystem::FadeIn(const mono::UpdateContext& update_context)
{
    m_alpha = math::EaseInCubic(m_fade_timer, tweak_values::fade_duration_s, 0.0f, 1.0f);
    m_fade_timer += update_context.delta_s;
    
    if(m_fade_timer > tweak_values::fade_duration_s)
        m_states.TransitionTo(States::IDLE);
}

void TeleportSystem::HandleTeleport(uint32_t entity_id)
{
    m_saved_teleport_position = m_transform_system->GetWorldPosition(entity_id);
    m_states.TransitionTo(States::FADE_OUT);
}

void TeleportSystem::TeleportPlayers(const math::Vector& world_position)
{
    const game::PlayerArray spawned_players = game::GetSpawnedPlayers();
    for(const game::PlayerInfo* player_info : spawned_players)
    {
        if(!player_info)
            continue;

        math::Matrix& player_transform = m_transform_system->GetTransform(player_info->entity_id);
        math::Position(player_transform, world_position);
        m_transform_system->SetTransformState(player_info->entity_id, mono::TransformState::CLIENT);
    }

    mono::ICamera* camera = m_camera_system->GetActiveCamera();
    camera->SetPosition(world_position);
    camera->SetTargetPosition(world_position);
}

bool TeleportSystem::ShouldApplyFadeAlpha() const
{
    return (m_states.ActiveState() != States::IDLE);
}

float TeleportSystem::GetFadeAlpha() const
{
    return m_alpha;
}


TeleportSystemDrawer::TeleportSystemDrawer(const TeleportSystem* teleport_system)
    : m_teleport_system(teleport_system)
{ }
void TeleportSystemDrawer::Draw(mono::IRenderer& renderer) const
{
    const bool should_apply_fade = m_teleport_system->ShouldApplyFadeAlpha();
    if(should_apply_fade)
        renderer.SetScreenFadeAlpha(m_teleport_system->GetFadeAlpha());
}
math::Quad TeleportSystemDrawer::BoundingBox() const
{
    return math::InfQuad;
}
