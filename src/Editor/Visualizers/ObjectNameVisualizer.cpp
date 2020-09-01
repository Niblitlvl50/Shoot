
#include "ObjectNameVisualizer.h"
#include "FontIds.h"
#include "Math/Quad.h"
#include "Zone/IEntity.h"
#include "Rendering/IRenderer.h"
#include "Rendering/Color.h"

using namespace editor;

ObjectNameVisualizer::ObjectNameVisualizer(const bool& enabled, const std::vector<IObjectProxyPtr>& object_proxies)
    : m_enabled(enabled)
    , m_object_proxies(object_proxies)
{ }

void ObjectNameVisualizer::Draw(mono::IRenderer& renderer) const
{
    if(!m_enabled)
        return;

    for(const IObjectProxyPtr& proxy : m_object_proxies)
    {
        constexpr math::Vector offset(0.0f, -1.0f);
        constexpr mono::Color::RGBA color;
        renderer.DrawText(shared::FontId::PIXELETTE_SMALL, proxy->Name(), proxy->GetPosition() + offset, true, color);
    }
}

math::Quad ObjectNameVisualizer::BoundingBox() const
{
    return math::InfQuad;
}
