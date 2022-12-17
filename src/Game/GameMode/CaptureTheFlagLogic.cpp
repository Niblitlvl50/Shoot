
#include "CaptureTheFlagLogic.h"
#include "DamageSystem/DamageSystem.h"
#include "Player/PlayerDaemonSystem.h"
#include "TransformSystem/TransformSystem.h"
#include "Math/MathFunctions.h"
#include "Math/Matrix.h"

#include "Rendering/IRenderer.h"
#include "Rendering/Color.h"
#include "Rendering/Sprite/SpriteSystem.h"
#include "Rendering/Sprite/Sprite.h"

#include "EntitySystem/IEntityManager.h"

#include <iostream>

using namespace game;

CaptureTheFlagLogic::CaptureTheFlagLogic(
    const std::vector<uint32_t>& flags,
    mono::TransformSystem* transform_system,
    mono::SpriteSystem* sprite_system,
    game::DamageSystem* damage_system,
    const PlayerDaemonSystem* player_daemon,
    mono::IEntityManager* entity_manager)
    : m_transform_system(transform_system)
    , m_damage_system(damage_system)
    , m_player_daemon(player_daemon)
    , m_entity_manager(entity_manager)
{
    m_flags.reserve(flags.size());

    constexpr mono::Color::RGBA dropzone_shades[] = {
        mono::Color::RED,
        mono::Color::BLUE,
        mono::Color::GREEN
    };

    std::vector<uint32_t> dropzones = flags;
    std::rotate(dropzones.begin(), dropzones.begin() + 1, dropzones.end());

    for(size_t index = 0; index < flags.size(); ++index)
    {
        const uint32_t flag_id = flags[index];
        const uint32_t dropzone_id = dropzones[index];

        const mono::Entity flag_entity = m_entity_manager->CreateEntity("res/entities/flag.entity");
        const math::Matrix& flag_world_transform = transform_system->GetWorld(flag_id);
        math::Matrix& flag_transform = m_transform_system->GetTransform(flag_entity.id);
        flag_transform = flag_world_transform;

        const mono::Entity dropzone_entity = m_entity_manager->CreateEntity("res/entities/dropzone.entity");
        const math::Matrix& dropzone_world_transform = transform_system->GetWorld(dropzone_id);
        math::Matrix& dropzone_transform = m_transform_system->GetTransform(dropzone_entity.id);
        dropzone_transform = dropzone_world_transform;

        const size_t shade_index = index % std::size(dropzone_shades);
        mono::Sprite* dropzone_sprite = sprite_system->GetSprite(dropzone_entity.id);
        dropzone_sprite->SetShade(dropzone_shades[shade_index]);

        FlagData flag_data;
        flag_data.state = FlagState::NONE;
        flag_data.flag_entity_id = flag_entity.id;
        flag_data.dropzone_entity_id = dropzone_entity.id;
        flag_data.spawn_position = math::GetPosition(flag_world_transform);
        flag_data.owning_entity_id = -1;
        flag_data.callback_handle = -1;
        flag_data.score = 0;

        m_flags.push_back(flag_data);
    }

    m_score.red = 0;
    m_score.blue = 0;
}

void CaptureTheFlagLogic::Start()
{
    const std::vector<uint32_t>& player_ids = m_player_daemon->GetPlayerIds();
    if(player_ids.empty())
        return;
}

void CaptureTheFlagLogic::End()
{

}

void CaptureTheFlagLogic::Update(const mono::UpdateContext& update_context)
{
    const std::vector<uint32_t>& player_ids = m_player_daemon->GetPlayerIds();
    if(player_ids.empty())
        return;

    for(FlagData& flag : m_flags)
    {
        if(flag.state == FlagState::NONE)
            CheckForPickup(player_ids, flag);
        else if(flag.state == FlagState::PICKED_UP)
            CheckForFlagDrop(flag);
    }

    if(m_flags.size() >= 2)
    {
        m_score.red = m_flags[0].score;
        m_score.blue = m_flags[1].score;
    }
}

void CaptureTheFlagLogic::DropFlag(uint32_t flag_owner_entity_id)
{
    for(FlagData& flag : m_flags)
    {
        if(flag.owning_entity_id == flag_owner_entity_id)
        {
            m_transform_system->UnchildTransform(flag.flag_entity_id);

            if(flag.callback_handle != std::numeric_limits<uint32_t>::max())
                m_damage_system->RemoveDamageCallback(flag_owner_entity_id, flag.callback_handle);

            flag.state = FlagState::NONE;
            flag.owning_entity_id = -1;
            flag.callback_handle = -1;

            return;
        }
    }
}

void CaptureTheFlagLogic::CheckForPickup(const std::vector<uint32_t>& player_ids, FlagData& flag)
{
    for(int player_id : player_ids)
    {
        const math::Matrix& flag_transform = m_transform_system->GetWorld(flag.flag_entity_id);
        const math::Quad& player_bb = m_transform_system->GetWorldBoundingBox(player_id);

        const bool pickup_flag = math::PointInsideQuad(math::GetPosition(flag_transform), player_bb);
        if(pickup_flag)
        {
            flag.state = FlagState::PICKED_UP;
            flag.owning_entity_id = player_id;

            math::Matrix& transform = m_transform_system->GetTransform(flag.flag_entity_id);
            math::Identity(transform);

            m_transform_system->ChildTransform(flag.flag_entity_id, flag.owning_entity_id);

            const game::DamageCallback entity_destroyed_func = [this](uint32_t entity_id, int damage, uint32_t id_who_did_damage, DamageType type) {
                DropFlag(entity_id);
            };

            flag.callback_handle = m_damage_system->SetDamageCallback(flag.owning_entity_id, DamageType::DESTROYED, entity_destroyed_func);
            break;
        }
    }
}

void CaptureTheFlagLogic::CheckForFlagDrop(FlagData& flag)
{
    const math::Matrix& flag_transform = m_transform_system->GetWorld(flag.flag_entity_id);
    const math::Quad& dropzone_bb = m_transform_system->GetWorldBoundingBox(flag.dropzone_entity_id);

    const bool drop_flag = math::PointInsideQuad(math::GetPosition(flag_transform), dropzone_bb);
    if(drop_flag)
    {
        DropFlag(flag.owning_entity_id);

        flag.score++;

        math::Matrix& world_transform = m_transform_system->GetTransform(flag.flag_entity_id);
        world_transform = math::CreateMatrixWithPosition(flag.spawn_position);
    }
}

void CaptureTheFlagLogic::ResetScore()
{
    for(FlagData& flag : m_flags)
        flag.score = 0;

    m_score.red = 0;
    m_score.blue = 0;
}

const CaptureTheFlagScore& CaptureTheFlagLogic::Score() const
{
    return m_score;
}
