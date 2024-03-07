
#include "TargetSystemDrawer.h"
#include "TargetSystem.h"
#include "Math/Quad.h"
#include "Rendering/Color.h"
#include "Rendering/IRenderer.h"

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
    
        const math::Vector& target_position = target->Position();
        const mono::Color::RGBA& target_color = (faction == TargetFaction::Enemies) ? mono::Color::RED : mono::Color::BLUE;
        renderer.DrawCircle(target_position, 0.5f, 16, 1.0f, target_color);
    }
}

math::Quad TargetSystemDrawer::BoundingBox() const
{
    return math::InfQuad;
}
