
#include "TeleportSystem.h"

#include "GameCamera/CameraSystem.h"
#include "Rendering/RenderSystem.h"
#include "TriggerSystem/TriggerSystem.h"
#include "TransformSystem/TransformSystem.h"

#include "Camera/ICamera.h"
#include "EntitySystem/Entity.h"
#include "Rendering/IRenderer.h"
#include "Math/EasingFunctions.h"

#include "Player/PlayerInfo.h"


using namespace game;

TeleportSystem::TeleportSystem(
    CameraSystem* camera_system,
    mono::TriggerSystem* trigger_system,
    mono::RenderSystem* render_system,
    mono::TransformSystem* transform_system)
    : m_camera_system(camera_system)
    , m_trigger_system(trigger_system)
    , m_render_system(render_system)
    , m_transform_system(transform_system)
{ }

const char* TeleportSystem::Name() const
{
    return "teleportsystem";
}

void TeleportSystem::Update(const mono::UpdateContext& update_context)
{
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

    const mono::TriggerCallback trigger_callback = [this, entity_id](uint32_t trigger_id) {
        HandleTeleport(entity_id);
    };
    teleport_info.trigger_handle = m_trigger_system->RegisterTriggerCallback(trigger_hash, trigger_callback, mono::INVALID_ID);
}

void TeleportSystem::HandleTeleport(uint32_t entity_id)
{
    const math::Vector teleport_position = m_transform_system->GetWorldPosition(entity_id);

    const mono::ScreenFadeCallback fade_callback = [this, teleport_position](mono::ScreenFadeState state) {
        if(state == mono::ScreenFadeState::FADE_OUT)
            TeleportPlayers(teleport_position);
    };
    m_render_system->TriggerScreenFade(mono::ScreenFadeState::FADE_OUT_PAUSE_IN, 1.0f, 0.5f, fade_callback);
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
