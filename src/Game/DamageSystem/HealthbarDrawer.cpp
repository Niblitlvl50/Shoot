
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

    struct DamageWord
    {
        const char* word = nullptr;
        mono::Color::RGBA color = mono::Color::WHITE;
    };
   

    // White/Grey
    static const mono::Color::RGBA DamageColor_WhiteSmoke = mono::Color::MakeFromBytes(245, 245, 245); // https://coolors.co/f5f5f5
    static const mono::Color::RGBA DamageColor_AntiflashWhite = mono::Color::MakeFromBytes(235, 235, 235); // https://coolors.co/ebebeb
    static const mono::Color::RGBA DamageColor_Platinum = mono::Color::MakeFromBytes(224, 224, 224); // https://coolors.co/e0e0e0
    static const mono::Color::RGBA DamageColor_Timberwolf = mono::Color::MakeFromBytes(214, 214, 214); // https://coolors.co/d6d6d6
    static const mono::Color::RGBA DamageColor_Silver = mono::Color::MakeFromBytes(199, 199, 199); // https://coolors.co/c7c7c7

    // Greens
    static const mono::Color::RGBA DamageColor_SpringBud = mono::Color::MakeFromBytes(162, 231, 35); // https://coolors.co/a2e723
    static const mono::Color::RGBA DamageColor_SGBUSGreen = mono::Color::MakeFromBytes(95, 232, 35); // https://coolors.co/5fe823
    static const mono::Color::RGBA DamageColor_Emerald = mono::Color::MakeFromBytes(35, 206, 107); // https://coolors.co/23ce6b
    static const mono::Color::RGBA DamageColor_LightGreen = mono::Color::MakeFromBytes(169, 253, 172); // https://coolors.co/a9fdac
    static const mono::Color::RGBA DamageColor_Zomp = mono::Color::MakeFromBytes(50, 162, 135); // https://coolors.co/32a287

    // Red-ish
    static const mono::Color::RGBA DamageColor_Tangelo = mono::Color::MakeFromBytes(242, 89, 24); // https://coolors.co/f25918
    static const mono::Color::RGBA DamageColor_Poppy = mono::Color::MakeFromBytes(214, 64, 69); // https://coolors.co/d64045
    static const mono::Color::RGBA DamageColor_Crayola = mono::Color::MakeFromBytes(237, 37, 78); // https://coolors.co/ed254e
    static const mono::Color::RGBA DamageColor_AmaranthPurple = mono::Color::MakeFromBytes(159, 32, 66); // https://coolors.co/9f2042
    static const mono::Color::RGBA DamageColor_Madder = mono::Color::MakeFromBytes(164, 22, 35); // https://coolors.co/a41623
    //static const mono::Color::RGBA DamageColor_Rose = mono::Color::MakeFromBytes(247, 37, 133); // https://coolors.co/f72585

    // Blues/Purples
    static const mono::Color::RGBA DamageColor_MajorelleBlue = mono::Color::MakeFromBytes(98, 60, 234); // https://coolors.co/623cea
    static const mono::Color::RGBA DamageColor_AirSuperiorityBlue = mono::Color::MakeFromBytes(110, 164, 191); // https://coolors.co/6ea4bf
    static const mono::Color::RGBA DamageColor_Tekhelet = mono::Color::MakeFromBytes(65, 51, 122); // https://coolors.co/41337a
    //static const mono::Color::RGBA DamageColor_Turquise = mono::Color::MakeFromBytes(115, 238, 220); // https://coolors.co/73eedc
    static const mono::Color::RGBA DamageColor_NeonBlue = mono::Color::MakeFromBytes(67, 97, 238); // https://coolors.co/4361ee
    static const mono::Color::RGBA DamageColor_VividSkyBlue = mono::Color::MakeFromBytes(76, 201, 240); // https://coolors.co/4cc9f0
    //static const mono::Color::RGBA DamageColor_Grape = mono::Color::MakeFromBytes(114, 9, 183); // https://coolors.co/7209b7
    
    // Yellowish
    static const mono::Color::RGBA DamageColor_Saffron = mono::Color::MakeFromBytes(238, 186, 11); // https://coolors.co/eeba0b
    static const mono::Color::RGBA DamageColor_Pantone = mono::Color::MakeFromBytes(248, 94, 0); // https://coolors.co/f85e00
    static const mono::Color::RGBA DamageColor_BurntSienna = mono::Color::MakeFromBytes(221, 110, 66); // https://coolors.co/dd6e42
    static const mono::Color::RGBA DamageColor_Maize = mono::Color::MakeFromBytes(255, 242, 117); // https://coolors.co/fff275
    static const mono::Color::RGBA DamageColor_Citrine = mono::Color::MakeFromBytes(234, 214, 55); // https://coolors.co/ead637

    // Blacks
    //static const mono::Color::RGBA DamageColor_Night = mono::Color::MakeFromBytes(15, 15, 15); // https://coolors.co/0f0f0f

    static const DamageWord damage_words[] = {
        { "hit",    DamageColor_WhiteSmoke      },
        { "bop",    DamageColor_AntiflashWhite  },     // sound of a hit
        { "thud",   DamageColor_Platinum        },     // to hit with a dull sound
        { "pock",   DamageColor_Timberwolf      },     // dry hit
        { "pof",    DamageColor_Silver          },

        { "pop",    DamageColor_SpringBud       },
        { "plonk",  DamageColor_SGBUSGreen      },     // a dull striking sound
        { "bam",    DamageColor_Emerald         },     // sound of a hard hit
        { "wap",    DamageColor_LightGreen      },     // hit/blow
        { "blaw",   DamageColor_Zomp            },

        { "whack",  DamageColor_Tangelo         },     // to strike sharply
        { "splat",  DamageColor_Poppy           },     // landing with a smacking sound
        { "pow",    DamageColor_Crayola         },     // sound of a blow
        { "zap",    DamageColor_AmaranthPurple  },
        { "bwak",   DamageColor_Madder          },     // sound of punch or kick from DBZ

        { "biff",   DamageColor_MajorelleBlue       },     // sound of an uppercut
        { "bonk",   DamageColor_AirSuperiorityBlue  },     // something heavy hitting something else
        { "whap",   DamageColor_Tekhelet            },     // to beat or strike
        { "klam",   DamageColor_NeonBlue            },     // sound of punch/hit from DBZ
        { "swah",   DamageColor_VividSkyBlue        },     // sound of a karate chop from DBZ

        { "slap",   DamageColor_Saffron     },
        { "bump",   DamageColor_Pantone     },     // heavy dull blow
        { "wham",   DamageColor_BurntSienna },     // a heavy blow
        { "smack",  DamageColor_Maize       },
        { "slam",   DamageColor_Citrine     },     

        { "smash",  mono::Color::WHITE },
        { "wreck",  mono::Color::WHITE },
        { "burn",   mono::Color::WHITE },
        { "crush",  mono::Color::WHITE },
        { "maul",   mono::Color::WHITE },

        { "bash",   mono::Color::WHITE },
    };

    const DamageWord& DamageToWord(int damage)
    {
        const int index = std::clamp(damage / 5, 0, (int)std::size(damage_words));
        return damage_words[index];
    }

    float GetNormalizedDamage(int damage)
    {
        return std::clamp(
            damage * (10.0f / float(std::size(damage_words))), 0.0f, 1.0f);
    }
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
        const math::Vector& world_position = m_transform_system->GetWorldPosition(damage_event.id_damaged_entity);
        const math::Vector& instigator_position = m_transform_system->GetWorldPosition(damage_event.id_who_did_damage);
        const math::Vector& delta_position_norm = math::Normalized(world_position - instigator_position);
        const math::Vector& offset = math::Vector(
            mono::Random(-0.2f, 0.2f),
            mono::Random(0.0f, 0.15f));
        const math::Matrix& world_transform = math::CreateMatrixWithPositionScale(world_position + offset, 0.35f);

        const DamageWord& damage_word = DamageToWord(damage_event.damage);
        if(game::g_debug_draw_damage_words)
            std::snprintf(text_buffer, std::size(text_buffer), "%s", damage_word.word);
        else
            std::snprintf(text_buffer, std::size(text_buffer), "%d", damage_event.damage);

        mono::Entity damage_number_entity = m_entity_system->CreateEntity(
            "damage_number", { TRANSFORM_COMPONENT, TEXT_COMPONENT, TRANSLATION_COMPONENT });
        m_transform_system->SetTransform(damage_number_entity.id, world_transform);

        mono::TextComponent text_data;
        text_data.text = text_buffer;
        text_data.font_id = damage_event.critical_hit ? FontId::RUSSOONE_SMALL : FontId::RUSSOONE_TINY;
        text_data.tint = mono::Color::WHITE;
        text_data.center_flags = mono::FontCentering::HORIZONTAL_VERTICAL;
        text_data.draw_shadow = true;
        text_data.shadow_offset = math::Vector(0.015f, 0.015f);
        text_data.shadow_color = mono::Color::BLACK;
        m_text_system->SetTextData(damage_number_entity.id, text_data);

        m_animation_system->AddTranslationComponent(
            damage_number_entity.id,
            0,
            damage_number_time_to_live_s,
            math::EaseOutCubic,
            math::EaseOutCubic,
            game::AnimationMode::ONE_SHOT,
            math::Vector(delta_position_norm.x * 0.1f, delta_position_norm.y * 0.2f));

        DamageNumber damage_number;
        damage_number.entity_id = damage_number_entity.id;
        damage_number.time_to_live_s = damage_number_time_to_live_s;
        damage_number.gradient = mono::Color::MakeGradient<3>(
            { 0.0f, 0.7f, 1.0f },
            { damage_word.color, mono::Color::MakeWithAlpha(damage_word.color, 0.2f), mono::Color::MakeWithAlpha(mono::Color::WHITE, 0.0f) }
        );

        m_damage_numbers.push_back(std::move(damage_number));
    }

    const auto update_and_remove_if_done = [this, &update_context](DamageNumber& damage_number)
    {
        damage_number.time_to_live_s -= update_context.delta_s;

        const float inverse_alpha_value = 1.0f - (damage_number.time_to_live_s / damage_number_time_to_live_s);
        m_text_system->SetTextColor(damage_number.entity_id, mono::Color::ColorFromGradient(damage_number.gradient, inverse_alpha_value));

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
