
#include "TargetSystemDrawer.h"
#include "TargetSystem.h"
#include "Math/Quad.h"
#include "Math/Matrix.h"
#include "Rendering/Color.h"
#include "Rendering/IRenderer.h"

#include "FontIds.h"

#include <cstdio>

using namespace game;

TargetSystemDrawer::TargetSystemDrawer(const bool& enable_drawing, TargetSystem* target_system)
    : m_enable_drawing(enable_drawing)
    , m_target_system(target_system)
{ }

void TargetSystemDrawer::Draw(mono::IRenderer& renderer) const
{
    if(!m_enable_drawing)
        return;

    const std::vector<ITargetPtr>& targets = m_target_system->GetActiveTargets();
    for(const ITargetPtr& target : targets)
    {
        if(!target->IsValid())
            continue;

        const TargetFaction faction = m_target_system->GetFaction(target->TargetId());
        const int priority = m_target_system->GetPriority(target->TargetId());
        const math::Vector& target_position = target->Position();

        const auto transform_scope = mono::MakeTransformScope(math::CreateMatrixWithPosition(target_position), &renderer);

        const mono::Color::RGBA& target_color = (faction == TargetFaction::Enemies) ? mono::Color::RED : mono::Color::BLUE;
        renderer.DrawCircle(math::ZeroVec, 0.5f, 16, 1.0f, target_color);

        char buffer[16] = { 0 };
        std::snprintf(buffer, std::size(buffer), "%d", priority);
        renderer.RenderText(FontId::PIXELETTE_TINY, buffer, target_color, mono::FontCentering::HORIZONTAL_VERTICAL);
    }
}

math::Quad TargetSystemDrawer::BoundingBox() const
{
    return math::InfQuad;
}
