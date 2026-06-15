
#include "StatusEffectSystem.h"
#include "Entity/EntityAnnotationSystem.h"

#include "Physics/IBody.h"
#include "Physics/PhysicsSystem.h"
#include "System/File.h"

#include "nlohmann/json.hpp"

#include <vector>

using namespace game;

StatusEffectSystem::StatusEffectSystem(mono::PhysicsSystem* physics_system, game::EntityAnnotationSystem* annotation_system)
    : m_physics_system(physics_system)
    , m_annotation_system(annotation_system)
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

    m_slow_effects[entity_id] = { multiplier, duration_s, annotation_id };
}

void StatusEffectSystem::ClearEffects(uint32_t entity_id)
{
    auto it = m_slow_effects.find(entity_id);
    if(it != m_slow_effects.end())
    {
        if(it->second.annotation_id != mono::INVALID_ID)
            m_annotation_system->RemoveAnnotation(it->second.annotation_id);
        m_slow_effects.erase(it);
    }
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
    {
        auto it = m_slow_effects.find(id);
        if(it->second.annotation_id != mono::INVALID_ID)
            m_annotation_system->RemoveAnnotation(it->second.annotation_id);
        m_slow_effects.erase(it);
    }
}
