
#include "SpriteOffsetDrawer.h"
#include "Math/Quad.h"
#include "Rendering/IRenderer.h"
#include "Rendering/Color.h"
#include "TransformSystem/TransformSystem.h"

using namespace animator;

SpriteOffsetDrawer::SpriteOffsetDrawer(
    mono::TransformSystem* transform_system,
    const mono::SpriteData* sprite_data,
    uint32_t entity_id,
    const bool& offset_mode)
    : m_transform_system(transform_system)
    , m_sprite_data(sprite_data)
    , m_sprite_id(entity_id)
    , m_offset_mode(offset_mode)
{ }

void SpriteOffsetDrawer::Draw(mono::IRenderer& renderer) const
{
    if(!m_offset_mode)
        return;

    constexpr mono::Color::RGBA default_color(1.0f, 0.0f, 0.0f);
    constexpr mono::Color::RGBA color_highlighted(0.0f, 0.5f, 1.0f, 1.0f);

    const math::Vector position = math::GetPosition(m_transform_system->GetWorld(m_sprite_id));
    const math::Quad world_bb = m_transform_system->GetWorldBoundingBox(m_sprite_id);

    const std::vector<math::Vector>& cross = {
        position + math::Vector(-0.25f, 0.0f),
        position + math::Vector(0.25f, 0.0f),
        position + math::Vector(0.0f, -0.25f),
        position + math::Vector(0.0f, 0.25f)
    };

    renderer.DrawLines(cross, color_highlighted, 2.0f);
    renderer.DrawPoints({ position }, color_highlighted, 12.0f);

    renderer.DrawQuad(world_bb, default_color, 1.0f);
}

math::Quad SpriteOffsetDrawer::BoundingBox() const
{
    return math::InfQuad;
}
