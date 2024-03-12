
#include "HealthbarDrawer.h"
#include "FontIds.h"
#include "DamageSystem/DamageSystem.h"
#include "Debug/GameDebugVariables.h"

#include "Entity/AnimationSystem.h"
#include "Entity/Component.h"

#include "Rendering/IRenderer.h"
#include "Rendering/Color.h"
#include "Rendering/RenderBuffer/BufferFactory.h"
#include "Rendering/Text/TextBufferFactory.h"
#include "Rendering/Text/TextFunctions.h"
#include "Rendering/Text/TextSystem.h"
#include "Math/Quad.h"
#include "Math/EasingFunctions.h"
#include "Util/Algorithm.h"
#include "Util/Random.h"
#include "TransformSystem/TransformSystem.h"
#include "EntitySystem/IEntityManager.h"

#include "Rendering/RenderSystem.h"
#include "Rendering/Sprite/ISprite.h"

#include <limits>
#include <string>

using namespace game;

namespace
{
    struct Healthbar
    {
        math::Vector position;
        float health_percentage;
        float width;
        float height;
        bool boss;
        uint32_t last_damaged_timestamp;
        std::string name;
    };

    constexpr mono::Color::RGBA healthbar_red = mono::Color::RGBA(1.0f, 0.3f, 0.3f, 1.0f);
    constexpr float damage_number_time_to_live_s = 0.75f;

    void GenerateHealthbarVertices(
        const std::vector<Healthbar>& healthbars,
        std::vector<math::Vector>& out_vertices,
        std::vector<mono::Color::RGBA>& out_colors,
        std::vector<uint16_t>& out_indices,
        std::vector<math::Vector>& out_boss_icon_positions)
    {
        for(const Healthbar& bar : healthbars)
        {
            const uint16_t base_index = out_vertices.size();

            const float percentage_length = bar.width * bar.health_percentage;
            const float half_length = bar.width / 2.0f;
            const float half_thickness = bar.height / 2.0f;

            const math::Vector& top_left = bar.position - math::Vector(half_length, half_thickness);
            const math::Vector& bottom_left = bar.position - math::Vector(half_length, -half_thickness);

            const math::Vector& top_mid_right = top_left + math::Vector(percentage_length, 0.0f);
            const math::Vector& bottom_mid_right = bottom_left + math::Vector(percentage_length, 0.0f);

            const math::Vector& top_right = top_left + math::Vector(bar.width, 0.0f);
            const math::Vector& bottom_right = bottom_left + math::Vector(bar.width, 0.0f);

            out_vertices.emplace_back(bottom_left);
            out_vertices.emplace_back(top_left);
            out_vertices.emplace_back(top_mid_right);
            out_vertices.emplace_back(bottom_mid_right);
            out_vertices.emplace_back(top_mid_right);
            out_vertices.emplace_back(bottom_mid_right);
            out_vertices.emplace_back(top_right);
            out_vertices.emplace_back(bottom_right);

            out_colors.emplace_back(healthbar_red);
            out_colors.emplace_back(healthbar_red);
            out_colors.emplace_back(healthbar_red);
            out_colors.emplace_back(healthbar_red);
            out_colors.emplace_back(mono::Color::GRAY);
            out_colors.emplace_back(mono::Color::GRAY);
            out_colors.emplace_back(mono::Color::GRAY);
            out_colors.emplace_back(mono::Color::GRAY);

            out_indices.push_back(base_index + 0);
            out_indices.push_back(base_index + 1);
            out_indices.push_back(base_index + 2);

            out_indices.push_back(base_index + 0);
            out_indices.push_back(base_index + 2);
            out_indices.push_back(base_index + 3);

            out_indices.push_back(base_index + 5);
            out_indices.push_back(base_index + 4);
            out_indices.push_back(base_index + 6);

            out_indices.push_back(base_index + 5);
            out_indices.push_back(base_index + 6);
            out_indices.push_back(base_index + 7);

            if(bar.boss)
            {
                const math::Vector icon_position = bar.position - math::Vector(half_length, 0.0f);
                out_boss_icon_positions.push_back(icon_position);
            }
        }
    }

    // https://www.writtensound.com/
    // https://www.writtensound.com/index.php?term=hard+hit

    constexpr const char* damage_words[] = {
        "pow",      // sound of a blow
        "bam",      // sound of a hard hit
        "blaw",
        "hit",
        "pof",
        "smack",
        "blast",
        "thud",     // to hit with a dull sound
        "whack",    // to strike sharply
        "wap",      // hit/blow
        "wham",     // a heavy blow
        "whap",     // to beat or strike
        "slap",
        "splat",    // landing with a smacking sound
        "slam",     
        "smash",
        "crash",
        "ruin",
        "tear",
        "wreck",
        "burn",
        "crush",
        "maul",
        "pop",
        "zap",
        "bam",
        "bash",
        "bwak",     // sound of punch or kick from DBZ
        "bump",     // heavy dull blow
        "biff",     // sound of an uppercut
        "bonk",     // something heavy hitting something else
        "bop",      // sound of a hit
        "klam",     // sound of punch/hit from DBZ
        "plonk",    // a dull striking sound
        "pock",     // dry hit
        "swah",     // sound of a karate chop from DBZ
    };

    const char* DamageToWord(int damage)
    {
        const int index = std::clamp(damage / 10, 0, (int)std::size(damage_words));
        return damage_words[index];
    }

    float GetNormalizedDamage(int damage)
    {
        return std::clamp(
            damage * (10.0f / float(std::size(damage_words))), 0.0f, 1.0f);
    }

    constexpr int g_selected_font = FontId::RUSSOONE_TINY;
    //constexpr int g_selected_font = FontId::PIXELETTE_TINY;
}

HealthbarDrawer::HealthbarDrawer(
    game::DamageSystem* damage_system, game::AnimationSystem* animation_system, mono::TextSystem* text_system, mono::TransformSystem* transform_system, mono::IEntityManager* entity_system)
    : m_damage_system(damage_system)
    , m_animation_system(animation_system)
    , m_text_system(text_system)
    , m_transform_system(transform_system)
    , m_entity_system(entity_system)
{
    m_boss_icon_sprite = mono::RenderSystem::GetSpriteFactory()->CreateSprite("res/sprites/squid.sprite");
    m_sprite_buffers = mono::BuildSpriteDrawBuffers(m_boss_icon_sprite->GetSpriteData());

    constexpr uint16_t indices[] = {
        0, 1, 2, 0, 2, 3
    };
    m_indices = mono::CreateElementBuffer(mono::BufferType::STATIC, 6, indices);
}

void HealthbarDrawer::Update(const mono::UpdateContext& update_context)
{
    char text_buffer[256] = { 0 };

    for(const DamageEvent& damage_event : m_damage_system->GetDamageEventsThisFrame())
    {
        //const bool was_destroyed = (damage_event.damage_result & DamageType::DESTROYED);
        const math::Quad& world_bb = m_transform_system->GetWorldBoundingBox(damage_event.id);

        const float random_offset_x = mono::Random(-0.2f, 0.2f);
        const float random_offset_y = mono::Random(0.0f, 0.15f);
        const math::Vector offset = math::Vector(math::Width(world_bb) * random_offset_x, math::Height(world_bb) * random_offset_y);
        const math::Matrix& world_transform = math::CreateMatrixWithPositionScale(math::TopLeft(world_bb) + offset, 0.5f);

        if(game::g_debug_draw_damage_words)
        {
            std::snprintf(text_buffer, std::size(text_buffer), "%s", DamageToWord(damage_event.damage));
        }
        else
        {
            std::snprintf(text_buffer, std::size(text_buffer), "%d", damage_event.damage);
        }

        mono::Entity damage_number_entity = m_entity_system->CreateEntity("damage_number", { TRANSFORM_COMPONENT, TEXT_COMPONENT, TRANSLATION_COMPONENT });
        m_transform_system->SetTransform(damage_number_entity.id, world_transform);

        mono::TextComponent text_data;
        text_data.text = text_buffer;
        text_data.font_id = g_selected_font;
        text_data.tint = mono::Color::RED;
        text_data.center_flags = mono::FontCentering::HORIZONTAL_VERTICAL;
        text_data.draw_shadow = true;
        text_data.shadow_offset = math::Vector(0.01f, 0.01f);
        text_data.shadow_color = mono::Color::BLACK;
        m_text_system->SetTextData(damage_number_entity.id, text_data);

        m_animation_system->AddTranslationComponent(
            damage_number_entity.id, 0, damage_number_time_to_live_s, math::EaseOutCubic, game::AnimationMode::ONE_SHOT, math::Vector(0.0f, 0.2f));

        DamageNumber damage_number;
        damage_number.entity_id = damage_number_entity.id;
        damage_number.time_to_live_s = damage_number_time_to_live_s;
        m_damage_numbers.push_back(std::move(damage_number));
    }

    static const mono::Color::Gradient<3> damage_gradient = mono::Color::MakeGradient<3>(
        { 0.0f, 0.7f, 1.0f },
        { mono::Color::GOLDEN_YELLOW, mono::Color::MakeWithAlpha(healthbar_red, 0.2f), mono::Color::MakeWithAlpha(healthbar_red, 0.0f) }
    );

    const auto update_and_remove_if_done = [this, &update_context](DamageNumber& damage_number)
    {
        damage_number.time_to_live_s -= update_context.delta_s;

        const float inverse_alpha_value = 1.0f - (damage_number.time_to_live_s / damage_number_time_to_live_s);
        m_text_system->SetTextColor(damage_number.entity_id, mono::Color::ColorFromGradient(damage_gradient, inverse_alpha_value));

        const bool time_to_destroy = (damage_number.time_to_live_s <= 0.0f);
        if(time_to_destroy)
            m_entity_system->ReleaseEntity(damage_number.entity_id);

        return time_to_destroy;
    };
    mono::remove_if(m_damage_numbers, update_and_remove_if_done);
}

void HealthbarDrawer::Draw(mono::IRenderer& renderer) const
{
    constexpr uint32_t max_uint = std::numeric_limits<uint32_t>::max();
    const uint32_t timestamp = renderer.GetTimestamp();

    std::vector<Healthbar> healthbars;

    const auto collect_func = [&](uint32_t entity_id, DamageRecord& record) {
        const bool ignore_record = (record.last_damaged_timestamp == max_uint);
        if(ignore_record)
            return;

        const uint32_t delta = timestamp - record.last_damaged_timestamp;
        if(delta > 5000)
            return;

        if(record.health <= 0)
            return;

        Healthbar bar;
        bar.last_damaged_timestamp = record.last_damaged_timestamp;
        bar.health_percentage = float(record.health) / float(record.full_health);
        bar.name = m_entity_system->GetEntityName(entity_id);
        bar.boss = record.is_boss;
        
        const math::Quad& world_bb = m_transform_system->GetWorldBoundingBox(entity_id);
        bar.position = math::BottomCenter(world_bb);
        bar.width = std::max(math::Width(world_bb) * 1.25f, 0.5f);
        bar.height = bar.boss ? 0.075f : 0.05f;
        healthbars.push_back(bar);
    };

    m_damage_system->ForEeach(collect_func);

    const int n_healthbars = healthbars.size();
    if(n_healthbars == 0)
        return;

    const uint32_t vertices_needed = n_healthbars * 8;
    const uint32_t indices_needed = n_healthbars * 12;

    std::vector<math::Vector> vertices;
    vertices.reserve(vertices_needed);

    std::vector<mono::Color::RGBA> colors;
    colors.reserve(vertices_needed);

    std::vector<uint16_t> indices;
    indices.reserve(indices_needed);

    std::vector<math::Vector> boss_icon_positions;

    GenerateHealthbarVertices(healthbars, vertices, colors, indices, boss_icon_positions);
    const uint32_t n_healthbar_indices = indices.size();

    auto vertex_buffer = mono::CreateRenderBuffer(
        mono::BufferType::STATIC, mono::BufferData::FLOAT, 2, vertices_needed, vertices.data());
    auto color_buffer = mono::CreateRenderBuffer(
        mono::BufferType::STATIC, mono::BufferData::FLOAT, 4, vertices_needed, colors.data());
    auto index_buffer = mono::CreateElementBuffer(
        mono::BufferType::STATIC, indices_needed, indices.data());

    if(!healthbars.empty())
        renderer.DrawTrianges(vertex_buffer.get(), color_buffer.get(), index_buffer.get(), 0, n_healthbar_indices);

    // Boss health bars

    for(const math::Vector& icon_position : boss_icon_positions)
    {
        const math::Matrix& world_transform = renderer.GetTransform() * math::CreateMatrixWithPosition(icon_position);
        auto transform_scope = mono::MakeTransformScope(world_transform, &renderer);
        renderer.DrawSprite(m_boss_icon_sprite.get(), &m_sprite_buffers, m_indices.get(), 0);
    }
}

math::Quad HealthbarDrawer::BoundingBox() const
{
    return math::InfQuad;
}
