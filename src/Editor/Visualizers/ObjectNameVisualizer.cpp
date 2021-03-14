
#include "ObjectNameVisualizer.h"
#include "FontIds.h"
#include "Math/Quad.h"
#include "Math/Matrix.h"
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
        const math::Vector position = proxy->GetPosition() + math::Vector(0.0f, -1.0f);
        const math::Matrix world_transform = math::CreateMatrixWithPosition(position);
        const auto scope = mono::MakeTransformScope(world_transform, &renderer);

        renderer.RenderText(shared::FontId::PIXELETTE_TINY, proxy->Name(), mono::Color::WHITE, mono::FontCentering::HORIZONTAL_VERTICAL);
    }
}

math::Quad ObjectNameVisualizer::BoundingBox() const
{
    return math::InfQuad;
}
