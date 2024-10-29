
#include "InteractionSystem.h"
#include "Player/PlayerInfo.h"
#include "TriggerSystem/TriggerSystem.h"

#include "Math/MathFunctions.h"
#include "TransformSystem/TransformSystem.h"
#include "System/Hash.h"

#include <iterator>

using namespace game;

InteractionSystem::InteractionSystem(
    uint32_t n, mono::TransformSystem* transform_system, mono::TriggerSystem* trigger_system)
    : m_transform_system(transform_system)
    , m_trigger_system(trigger_system)
    , m_components(n)
{
    m_component_details.resize(n, { false, true, nullptr });
}

InteractionComponent* InteractionSystem::AllocateComponent(uint32_t entity_id)
{
    InteractionComponent component = {};
    return m_components.Set(entity_id, std::move(component));
}

void InteractionSystem::ReleaseComponent(uint32_t entity_id)
{
    m_component_details[entity_id] = { false, true, nullptr };
    m_components.Release(entity_id);
}

void InteractionSystem::AddComponent(
    uint32_t entity_id,
    uint32_t interaction_hash,
    InteractionType interaction_type,
    bool draw_name,
    const std::string& interaction_sound)
{
    AddComponent(entity_id, interaction_hash, hash::NO_HASH, interaction_type, draw_name, interaction_sound);
}

void InteractionSystem::AddComponent(
    uint32_t entity_id,
    uint32_t on_interaction_hash,
    uint32_t off_interaction_hash,
    InteractionType interaction_type,
    bool draw_name,
    const std::string& interaction_sound)
{
    InteractionComponent* component = m_components.Get(entity_id);
    component->on_interaction_hash = on_interaction_hash;
    component->off_interaction_hash = off_interaction_hash;
    component->type = interaction_type;
    component->draw_name = draw_name;
    component->sound = audio::CreateNullSound();
    
    if(!interaction_sound.empty())
        component->sound = audio::CreateSound(interaction_sound.c_str(), audio::SoundPlayback::ONCE);

    // Internal data
    InteractionComponentDetails& details = m_component_details[entity_id];
    details.triggered = false;
    details.enabled = true;
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
        
        InteractionComponentDetails& details = m_component_details[interaction_id];
        if(!details.enabled)
            return;

        math::Quad interaction_bb = m_transform_system->GetWorldBoundingBox(interaction_id);
        math::ResizeQuad(interaction_bb, 0.25f);

        for(const PlayerInfo* player_info : active_players)
        {
            if(!player_info)
                continue;

            const math::Quad player_bb = m_transform_system->GetWorldBoundingBox(player_info->entity_id);
            const bool overlaps = math::QuadOverlaps(interaction_bb, player_bb);
            if(!overlaps)
                continue;

            m_interaction_data.active.push_back(
                { interaction_id, player_info->entity_id, interaction.type, player_info->last_used_input, interaction.draw_name }
            );

            const auto find_player_func = [player_info](const PlayerTriggerData& player_trigger_data) {
                return player_trigger_data.player_entity_id == player_info->entity_id;
            };

            const auto it = std::find_if(m_player_triggers.begin(), m_player_triggers.end(), find_player_func);
            if(it != m_player_triggers.end())
            {
                const bool has_on_hash = (interaction.on_interaction_hash != hash::NO_HASH);
                const bool has_off_hash = (interaction.off_interaction_hash != hash::NO_HASH);

                if(has_on_hash || has_off_hash)
                {
                    const uint32_t hash =
                        (details.triggered && has_off_hash) ? interaction.off_interaction_hash : interaction.on_interaction_hash;
                    m_trigger_system->EmitTrigger(hash);
                    if(details.callback)
                        details.callback(interaction_id, interaction.type);
                    details.triggered = !details.triggered;
                }

                if(it->callback != nullptr)
                    it->callback(interaction_id, interaction.type);

                interaction.sound->Play();
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

void InteractionSystem::TryTriggerInteraction(uint32_t entity_id, const InteractionCallback& callback)
{
    m_player_triggers.push_back({ entity_id, callback});
}

bool InteractionSystem::CanPlayerTriggerInteraction(uint32_t player_entity_id)
{
    for(const InteractionAndTrigger& active_interaction : m_interaction_data.active)
    {
        if(active_interaction.trigger_id == player_entity_id)
            return true;
    }

    return false;
}

void InteractionSystem::SetInteractionEnabled(uint32_t entity_id, bool enabled)
{
    m_component_details[entity_id].enabled = enabled;
}

void InteractionSystem::SetInteractionCallback(uint32_t entity_id, const InteractionCallback& callback)
{
    m_component_details[entity_id].callback = callback;
}

const FrameInteractionData& InteractionSystem::GetFrameInteractionData() const
{
    return m_interaction_data;
}
