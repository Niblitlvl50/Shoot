
#include "StatusEffectSystem.h"
#include "Entity/EntityAnnotationSystem.h"

#include "Physics/IBody.h"
#include "Physics/PhysicsSystem.h"
#include "System/File.h"
#include "EntitySystem/IEntityManager.h"

#include "nlohmann/json.hpp"

#include <vector>

using namespace game;

StatusEffectSystem::StatusEffectSystem(
    mono::PhysicsSystem* physics_system, game::EntityAnnotationSystem* annotation_system, mono::IEntityManager* entity_manager)
    : m_physics_system(physics_system)
    , m_annotation_system(annotation_system)
    , m_entity_manager(entity_manager)
{
    file::FilePtr config_file = file::OpenAsciiFile("res/configs/status_effect_config.json");
    if(config_file)
    {
        const std::vector<byte>& file_data = file::FileRead(config_file);
        const nlohmann::json& json = nlohmann::json::parse(file_data);
        m_slow_annotation_entity = json["slow_annotation_entity"];
    }
}

void StatusEffectSystem::ApplySlowEffect(uint32_t entity_id, float multiplier, float duration_s)
{
    auto it = m_slow_effects.find(entity_id);
    if(it != m_slow_effects.end())
    {
        it->second.multiplier = multiplier;
        it->second.remaining_s = duration_s;
        return;
    }

    uint32_t annotation_id = mono::INVALID_ID;
    if(m_annotation_system && !m_slow_annotation_entity.empty())
        annotation_id = m_annotation_system->AddAnnotation(entity_id, m_slow_annotation_entity, AnnotationCorner::TopLeft);

    // The annotation is a lifetime dependency of entity_id, so it gets released
    // automatically if entity_id dies while still under this effect. Drop our
    // bookkeeping when that happens so a later expiry doesn't try to release the
    // (already gone) annotation again.
    const uint32_t callback_id = m_entity_manager->AddReleaseCallback(
        entity_id,
        mono::ReleasePhase::PRE_RELEASE,
        [this](uint32_t released_entity_id, mono::ReleasePhase)
        {
            m_slow_effects.erase(released_entity_id);
        });

    m_slow_effects[entity_id] = { multiplier, duration_s, annotation_id, callback_id };
}

void StatusEffectSystem::RemoveEffect(uint32_t entity_id)
{
    const auto it = m_slow_effects.find(entity_id);
    if(it == m_slow_effects.end())
        return;

    m_entity_manager->RemoveReleaseCallback(entity_id, it->second.release_callback_id);

    if(it->second.annotation_id != mono::INVALID_ID)
        m_annotation_system->RemoveAnnotation(it->second.annotation_id);

    m_slow_effects.erase(it);
}

void StatusEffectSystem::ClearEffects(uint32_t entity_id)
{
    RemoveEffect(entity_id);
}

const char* StatusEffectSystem::Name() const
{
    return "statuseffectsystem";
}

void StatusEffectSystem::Update(const mono::UpdateContext& update_context)
{
    std::vector<uint32_t> expired;

    for(auto& [entity_id, effect] : m_slow_effects)
    {
        mono::IBody* body = m_physics_system->GetBody(entity_id);
        if(body)
            body->SetVelocity(body->GetVelocity() * effect.multiplier);

        effect.remaining_s -= update_context.delta_s;
        if(effect.remaining_s <= 0.0f)
            expired.push_back(entity_id);
    }

    for(uint32_t id : expired)
        RemoveEffect(id);
}
