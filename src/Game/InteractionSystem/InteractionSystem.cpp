
#include "InteractionSystem.h"
#include "PlayerInfo.h"
#include "TriggerSystem/TriggerSystem.h"

#include "Math/MathFunctions.h"
#include "TransformSystem/TransformSystem.h"
#include "Util/Hash.h"

using namespace game;

InteractionSystem::InteractionSystem(
    uint32_t n, mono::TransformSystem* transform_system, game::TriggerSystem* trigger_system)
    : m_transform_system(transform_system)
    , m_trigger_system(trigger_system)
{
    m_components.resize(n);
    m_active.resize(n, false);
    m_active_interactions.reserve(8);
}

InteractionComponent* InteractionSystem::AllocateComponent(uint32_t entity_id)
{
    m_active[entity_id] = true;
    return &m_components[entity_id];
}

void InteractionSystem::ReleaseComponent(uint32_t entity_id)
{
    m_active[entity_id] = false;
}

void InteractionSystem::AddComponent(uint32_t entity_id, uint32_t interaction_hash)
{
    m_components[entity_id].interaction_hash = interaction_hash;
}

uint32_t InteractionSystem::Id() const
{
    return mono::Hash(Name());
}

const char* InteractionSystem::Name() const
{
    return "interactionsystem";
}

void InteractionSystem::Update(const mono::UpdateContext& update_context)
{
    m_active_interactions.clear();
    m_triggered_interactions.clear();

    const game::PlayerArray active_players = game::GetActivePlayers();

    const auto collect_active_interactions = [&, this](uint32_t entity_id, const InteractionComponent& interaction) {
        
        math::Quad interaction_bb = m_transform_system->GetWorldBoundingBox(entity_id);
        math::ResizeQuad(interaction_bb, 0.5f);

        for(const PlayerInfo* player_info : active_players)
        {
            if(!player_info)
                continue;

            const math::Quad player_bb = m_transform_system->GetWorldBoundingBox(player_info->entity_id);
            const bool overlaps = math::QuadOverlaps(interaction_bb, player_bb);
            if(overlaps)
            {
                m_active_interactions.push_back(entity_id);

                const auto it = std::find(m_player_triggers.begin(), m_player_triggers.end(), player_info->entity_id);
                if(it != m_player_triggers.end())
                {
                    m_triggered_interactions.push_back(entity_id);
                    m_trigger_system->EmitTrigger(interaction.interaction_hash);
                }
            }
        }
    };

    ForEach(collect_active_interactions);

    m_player_triggers.clear();
}

void InteractionSystem::TryTriggerInteraction(uint32_t entity_id)
{
    m_player_triggers.push_back(entity_id);
}

const std::vector<uint32_t>& InteractionSystem::GetActiveInteractions() const
{
    return m_active_interactions;
}

const std::vector<uint32_t>& InteractionSystem::GetTriggeredInteractions() const
{
    return m_triggered_interactions;
}
