
#include "CaptureTheFlagLogic.h"
#include "Player/PlayerDaemon.h"
#include "TransformSystem/TransformSystem.h"

#include "Math/MathFunctions.h"

using namespace game;

CaptureTheFlagLogic::CaptureTheFlagLogic(
    const std::vector<FlagDropzonePair>& flags, mono::TransformSystem* transform_system, const PlayerDaemon* player_daemon)
    : m_transform_system(transform_system)
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

        m_flags.push_back(flag_data);
    }
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
}

void CaptureTheFlagLogic::CheckForPickup(const std::vector<uint32_t>& player_ids, FlagData& flag) const
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

            m_transform_system->GetTransform(flag.flag_entity_id) = math::Matrix();
            m_transform_system->ChildTransform(flag.flag_entity_id, flag.owning_entity_id);

            printf("Pick up flag %u\n", flag.flag_entity_id);

            break;
        }
    }
}

void CaptureTheFlagLogic::CheckForFlagDrop(FlagData& flag) const
{
    const math::Matrix& flag_transform = m_transform_system->GetWorld(flag.flag_entity_id);
    const math::Quad& dropzone_bb = m_transform_system->GetWorldBoundingBox(flag.dropzone_entity_id);

    const bool drop_flag = math::PointInsideQuad(math::GetPosition(flag_transform), dropzone_bb);
    if(drop_flag)
    {
        flag.state = FlagState::NONE;
        flag.owning_entity_id = -1;

        m_transform_system->UnchildTransform(flag.flag_entity_id);
        m_transform_system->GetWorld(flag.flag_entity_id) = math::CreateMatrixWithPosition(flag.spawn_position);

        printf("Drop flag! %u\n", flag.flag_entity_id);
    }
}
