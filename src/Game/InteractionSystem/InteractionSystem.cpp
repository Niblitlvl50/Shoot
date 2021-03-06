
#include "InteractionSystem.h"
#include "Player/PlayerInfo.h"
#include "TriggerSystem/TriggerSystem.h"

#include "Math/MathFunctions.h"
#include "TransformSystem/TransformSystem.h"
#include "Util/Hash.h"

using namespace game;

namespace
{
    constexpr uint32_t NO_HASH = std::numeric_limits<uint32_t>::max();
}

InteractionSystem::InteractionSystem(
    uint32_t n, mono::TransformSystem* transform_system, game::TriggerSystem* trigger_system)
    : m_transform_system(transform_system)
    , m_trigger_system(trigger_system)
{
    m_components.resize(n);
    m_active.resize(n, false);
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

void InteractionSystem::AddComponent(uint32_t entity_id, uint32_t interaction_hash, shared::InteractionType interaction_type)
{
    AddComponent(entity_id, interaction_hash, NO_HASH, interaction_type);
}

void InteractionSystem::AddComponent(uint32_t entity_id, uint32_t on_interaction_hash, uint32_t off_interaction_hash, shared::InteractionType interaction_type)
{
    InteractionComponent& component = m_components[entity_id];
    component.on_interaction_hash = on_interaction_hash;
    component.off_interaction_hash = off_interaction_hash;
    component.type = interaction_type;
    component.triggered = false;
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
    m_interaction_data.active.clear();
    m_interaction_data.deactivated.clear();

    const game::PlayerArray active_players = game::GetActivePlayers();

    const auto collect_active_interactions = [&, this](uint32_t interaction_id, InteractionComponent& interaction) {
        
        math::Quad interaction_bb = m_transform_system->GetWorldBoundingBox(interaction_id);
        math::ResizeQuad(interaction_bb, 0.25f);

        for(const PlayerInfo* player_info : active_players)
        {
            if(!player_info)
                continue;

            const math::Quad player_bb = m_transform_system->GetWorldBoundingBox(player_info->entity_id);
            const bool overlaps = math::QuadOverlaps(interaction_bb, player_bb);
            if(overlaps)
            {
                m_interaction_data.active.push_back({ interaction_id, player_info->entity_id, interaction.type });

                const auto it = std::find(m_player_triggers.begin(), m_player_triggers.end(), player_info->entity_id);
                if(it != m_player_triggers.end())
                {
                    const bool has_off_hash = (interaction.off_interaction_hash != NO_HASH);
                    const uint32_t hash =
                        (interaction.triggered && has_off_hash) ? interaction.off_interaction_hash : interaction.on_interaction_hash;
                    m_trigger_system->EmitTrigger(hash);

                    interaction.triggered = !interaction.triggered;
                }
            }
        }
    };

    ForEach(collect_active_interactions);

    const auto comp = [](const InteractionAndTrigger& left, const InteractionAndTrigger& right) {
        return left.interaction_id < right.interaction_id;
    };

    std::set_difference(
        m_previous_active_interactions.begin(),
        m_previous_active_interactions.end(),
        m_interaction_data.active.begin(),
        m_interaction_data.active.end(),
        std::back_inserter(m_interaction_data.deactivated),
        comp);

    m_previous_active_interactions = m_interaction_data.active;
    m_player_triggers.clear();
}

void InteractionSystem::TryTriggerInteraction(uint32_t entity_id)
{
    m_player_triggers.push_back(entity_id);
}

const FrameInteractionData& InteractionSystem::GetFrameInteractionData() const
{
    return m_interaction_data;
}
