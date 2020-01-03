
#include "CaptureTheFlagLogic.h"
#include "DamageSystem.h"
#include "Player/PlayerDaemon.h"
#include "TransformSystem/TransformSystem.h"
#include "Math/MathFunctions.h"
#include "Math/Matrix.h"

#include "Rendering/IRenderer.h"
#include "Rendering/Color.h"

#include "FontIds.h"

using namespace game;

CaptureTheFlagLogic::CaptureTheFlagLogic(
    const std::vector<FlagDropzonePair>& flags,
    mono::TransformSystem* transform_system,
    game::DamageSystem* damage_system,
    const PlayerDaemon* player_daemon)
    : m_transform_system(transform_system)
    , m_damage_system(damage_system)
    , m_player_daemon(player_daemon)
{
    m_flags.reserve(flags.size());

    for(const FlagDropzonePair& pair : flags)
    {
        const math::Matrix& original_transform = transform_system->GetWorld(pair.flag_entity_id);

        FlagData flag_data;
        flag_data.state = FlagState::NONE;
        flag_data.flag_entity_id = pair.flag_entity_id;
        flag_data.dropzone_entity_id = pair.dropzone_entity_id;
        flag_data.spawn_position = math::GetPosition(original_transform);
        flag_data.owning_entity_id = -1;
        flag_data.callback_handle = -1;
        flag_data.score = 0;

        m_flags.push_back(flag_data);

        printf("flag spwn position: %f %f\n", flag_data.spawn_position.x, flag_data.spawn_position.y);
    }

    m_score.red = 0;
    m_score.blue = 0;
}

void CaptureTheFlagLogic::doUpdate(const mono::UpdateContext& update_context)
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

    if(m_flags.size() == 2)
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
                m_damage_system->RemoveCallback(flag_owner_entity_id, flag.callback_handle);

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

            const game::DestroyedCallback entity_destroyed_func = [this](uint32_t entity_id) {
                DropFlag(entity_id);
            };

            flag.callback_handle = m_damage_system->SetDestroyedCallback(flag.owning_entity_id, entity_destroyed_func);
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
