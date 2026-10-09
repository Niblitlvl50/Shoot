
#include "TrainZoneSystemDrawer.h"
#include "TrainZoneSystem.h"
#include "Player/PlayerInfo.h"
#include "FontIds.h"

#include "Math/Quad.h"
#include "Math/Matrix.h"
#include "Math/MathFunctions.h"
#include "Rendering/Color.h"
#include "Rendering/IRenderer.h"
#include "Rendering/Text/TextFunctions.h"
#include "TransformSystem/TransformSystem.h"
#include "EntitySystem/IEntityManager.h"

#include <algorithm>
#include <cstdio>
#include <limits>

namespace tweak_values
{
    constexpr game::FontId label_font = game::FontId::RUSSOONE_TINY;
    constexpr mono::Color::RGBA label_color = mono::Color::OFF_WHITE;
    constexpr mono::Color::RGBA background_color = mono::Color::RGBA(0.1f, 0.1f, 0.1f, 0.6f);
    constexpr float label_padding = 0.1f;
    constexpr float label_offset_y = 0.4f;

    // The label is fully shown within show_distance and fades out over fade_distance beyond it.
    constexpr float show_distance = 3.0f;
    constexpr float fade_distance = 1.0f;

    constexpr float progress_width = 1.2f;
    constexpr float progress_height = 0.15f;
    constexpr float progress_offset_y = 0.4f;
    constexpr mono::Color::RGBA progress_background_color = mono::Color::RGBA(0.1f, 0.1f, 0.1f, 0.7f);
    constexpr mono::Color::RGBA progress_color = mono::Color::RGBA(0.95f, 0.8f, 0.3f, 1.0f);
}

using namespace game;

TrainZoneSystemDrawer::TrainZoneSystemDrawer(
    const TrainZoneSystem* train_zone_system,
    const mono::TransformSystem* transform_system)
    : m_train_zone_system(train_zone_system)
    , m_transform_system(transform_system)
{ }

void TrainZoneSystemDrawer::Draw(mono::IRenderer& renderer) const
{
    const auto draw_cargo_label = [&, this](uint32_t cargo_entity_id, const CargoComponent& cargo) {

        if(cargo.loaded || cargo.delivered || cargo.name.empty())
            return;

        const math::Vector cargo_position = m_transform_system->GetWorldPosition(cargo_entity_id);

        float closest_player_distance = std::numeric_limits<float>::max();
        for(const PlayerInfo& player_info : g_players)
        {
            if(player_info.player_state != PlayerState::ALIVE)
                continue;

            closest_player_distance = std::min(closest_player_distance, math::DistanceBetween(cargo_position, player_info.position));
        }

        const float alpha =
            1.0f - std::clamp((closest_player_distance - tweak_values::show_distance) / tweak_values::fade_distance, 0.0f, 1.0f);
        if(alpha <= 0.0f)
            return;

        char label[256] = {};
        std::snprintf(label, std::size(label), "%s", cargo.name.c_str());

        const math::Quad cargo_bounds = m_transform_system->GetWorldBoundingBox(cargo_entity_id);
        const math::Vector label_position(cargo_position.x, math::Top(cargo_bounds) + tweak_values::label_offset_y);

        const math::Matrix transform = math::CreateMatrixWithPosition(label_position);
        const auto transform_scope = mono::MakeTransformScope(transform, &renderer);

        const mono::TextMeasurement text_measurement = mono::MeasureString(tweak_values::label_font, label);
        const math::Vector half_size = (text_measurement.size / 2.0f) + math::Vector(tweak_values::label_padding, tweak_values::label_padding);

        mono::Color::RGBA background_color = tweak_values::background_color;
        background_color.alpha *= alpha;
        renderer.DrawFilledQuad(math::Quad(-half_size, half_size), background_color);

        mono::Color::RGBA label_color = tweak_values::label_color;
        label_color.alpha *= alpha;
        renderer.RenderText(tweak_values::label_font, label, label_color, mono::FontCentering::HORIZONTAL_VERTICAL);
    };

    m_train_zone_system->ForEachCargo(draw_cargo_label);

    for(const TrainZoneSystem::LoadingProgress& progress : m_train_zone_system->GetLoadingProgress())
    {
        const math::Vector car_position = m_transform_system->GetWorldPosition(progress.car_entity_id);
        const math::Quad car_bounds = m_transform_system->GetWorldBoundingBox(progress.car_entity_id);
        const math::Vector bar_position(car_position.x, math::Top(car_bounds) + tweak_values::progress_offset_y);

        const math::Matrix transform = math::CreateMatrixWithPosition(bar_position);
        const auto transform_scope = mono::MakeTransformScope(transform, &renderer);

        const float half_width = tweak_values::progress_width / 2.0f;
        const float half_height = tweak_values::progress_height / 2.0f;
        const float fill_width = tweak_values::progress_width * std::clamp(progress.fraction, 0.0f, 1.0f);

        renderer.DrawFilledQuad(
            math::Quad(-half_width, -half_height, half_width, half_height), tweak_values::progress_background_color);
        renderer.DrawFilledQuad(
            math::Quad(-half_width, -half_height, -half_width + fill_width, half_height), tweak_values::progress_color);
    }
}

math::Quad TrainZoneSystemDrawer::BoundingBox() const
{
    return math::InfQuad;
}
