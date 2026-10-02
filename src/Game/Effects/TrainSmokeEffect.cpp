
#include "TrainSmokeEffect.h"
#include "Entity/AnimationSystem.h"
#include "Entity/Component.h"

#include "Particle/ParticleSystem.h"
#include "TransformSystem/TransformSystem.h"
#include "Util/Random.h"
#include "Util/Algorithm.h"
#include "EntitySystem/IEntityManager.h"
#include "Entity/Component.h"
#include "Rendering/Sprite/SpriteSystem.h"
#include "Rendering/Sprite/Sprite.h"

#include "Math/EasingFunctions.h"

using namespace game;

namespace
{
    void TrainSmokeGenerator(const mono::ParticleGeneratorContext& context, mono::ParticlePoolComponentView& component_view)
    {
        const float x_variation = mono::Random(-0.02f, 0.02f);
        const float y_variation = mono::Random(-0.02f, 0.02f);
        const float x_velocity_variation = mono::Random(-0.1f, 0.1f);
        const float y_velocity_variation = mono::Random(2.0f, 2.5f);
        const float size = mono::Random(64.0f, 70.0f);
        const float end_size = mono::Random(80.0f, 120.0f);
        const float life = mono::Random(2.0f, 2.8f);

        component_view.position = context.position + math::Vector(x_variation, y_variation);
        component_view.rotation = 0.0f;
        component_view.velocity = math::Vector(x_velocity_variation, y_velocity_variation);
        component_view.angular_velocity = 0.0f; //mono::Random(-1.1f, 1.1f);

        using namespace mono::Color;
        component_view.color = mono::Color::MakeWithAlpha(OFF_WHITE, 0.25f);
        component_view.gradient = mono::Color::MakeGradient<4>(
            { 0.0f, 0.25f, 1.0f, 1.0f },
            { component_view.color, mono::Color::MakeWithAlpha(component_view.color, 0.75f), RGBA(1.0f, 1.0f, 1.0f, 0.0f), RGBA() }
        );
        component_view.start_size = size;
        component_view.end_size = end_size;
        component_view.size = size;

        component_view.start_life = life;
        component_view.life = life;
    }

    constexpr float g_smoke_entity_time_to_live_s = 0.75f;
    constexpr float g_smoke_entity_time_to_live_variation_s = 0.25f;
}

TrainSmokeEffect::TrainSmokeEffect(
    mono::IEntityManager* entity_system,
    mono::TransformSystem* transform_system,
    mono::SpriteSystem* sprite_system,
    game::AnimationSystem* animation_system,
    uint32_t parent_entity_id)
    : m_entity_system(entity_system)
    , m_transform_system(transform_system)
    , m_sprite_system(sprite_system)
    , m_animation_system(animation_system)
    , m_parent_entity_id(parent_entity_id)
    , m_emit_rate_per_s(5.0f)
    , m_emit_counter(0.0f)
{
}

void TrainSmokeEffect::UpdateEmitterSpeed(float emit_rate_per_s)
{
    m_emit_rate_per_s = emit_rate_per_s;
}

void TrainSmokeEffect::UpdateSmoke(const mono::UpdateContext& update_context)
{
    if(update_context.paused)
        return;

    m_emit_counter += m_emit_rate_per_s * update_context.delta_s;
    
    while(m_emit_counter >= 1.0f)
    {
        mono::Entity spawned_entity = m_entity_system->CreateEntity("TrainSmoke", { TRANSFORM_COMPONENT, SPRITE_COMPONENT, TRANSLATION_COMPONENT });
        
        const char* sprite_file = mono::Chance(50) ? "res/sprites/smoke_white_1.sprite" : "res/sprites/smoke_white_2.sprite";
        
        const float rotation_array[] = { 0.0f, math::PI_2(), math::PI(), math::PI_2() * 3.0f };
        const float rotation = rotation_array[mono::RandomInt(0, 3)];
        const float entity_time_to_live_s = g_smoke_entity_time_to_live_s + mono::Random(-g_smoke_entity_time_to_live_variation_s, g_smoke_entity_time_to_live_variation_s);

        mono::SpriteComponents sprite_component;
        sprite_component.sprite_file = sprite_file;
        sprite_component.shade = mono::Color::MakeWithAlpha(mono::Color::OFF_WHITE, 0.0f);
        sprite_component.random_start_frame = true;
        sprite_component.animation_id = 0;
        sprite_component.layer = 0;
        sprite_component.sort_offset = 0.0f;
        sprite_component.properties = 0;
        m_sprite_system->SetSpriteData(spawned_entity.id, sprite_component);

        const math::Vector& parent_position = m_transform_system->GetWorldPosition(m_parent_entity_id);
        m_transform_system->SetTransform(
            spawned_entity.id,
            math::CreateMatrixWithPositionRotationScale(parent_position + math::Vector(0.0f, 0.3f), rotation, math::Vector(0.5f, 0.5f)));

        m_animation_system->AddTranslationComponent(
            spawned_entity.id,
            0,
            entity_time_to_live_s,
            math::EaseOutCubic,
            math::EaseOutCubic,
            game::AnimationMode::ONE_SHOT,
            math::Vector(mono::Random(-0.1f, 0.1f), 0.5f));

        m_animation_system->AddScaleComponent(
            spawned_entity.id,
            0,
            entity_time_to_live_s,
            math::EaseInOutCubic,
            game::AnimationMode::ONE_SHOT,
            0.5f,
            mono::Random(0.1f, 0.2f));

        SmokeEntity smoke_entity;
        smoke_entity.entity_id = spawned_entity.id;
        smoke_entity.time_to_live_s = entity_time_to_live_s;
        smoke_entity.time_to_live_counter_s = entity_time_to_live_s;
        m_smoke_entities.push_back(std::move(smoke_entity));

        m_emit_counter -= 1.0f;
    }

    const auto update_and_remove_if_done = [this, &update_context](SmokeEntity& smoke_entity)
    {
        smoke_entity.time_to_live_counter_s -= update_context.delta_s;

        mono::Sprite* sprite = m_sprite_system->GetSprite(smoke_entity.entity_id);
        sprite->SetShade(mono::Color::MakeWithAlpha(mono::Color::OFF_WHITE, smoke_entity.time_to_live_counter_s / smoke_entity.time_to_live_s));

        const bool time_to_destroy = (smoke_entity.time_to_live_counter_s <= 0.0f);
        if(time_to_destroy)
            m_entity_system->ReleaseEntity(smoke_entity.entity_id);

        return time_to_destroy;
    };
    mono::remove_if(m_smoke_entities, update_and_remove_if_done);
}
