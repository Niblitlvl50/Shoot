
#include "ObjectDetailVisualizer.h"
#include "ObjectProxies/IObjectProxy.h"

#include "Entity/IEntity.h"
#include "Math/Quad.h"
#include "Rendering/IRenderer.h"
#include "Rendering/Color.h"


#include "DefinedAttributes.h"
#include "ObjectAttribute.h"

using namespace editor;

ObjectDetailVisualizer::ObjectDetailVisualizer()
{

}

void ObjectDetailVisualizer::SetObjectProxy(IObjectProxy* object_proxy)
{
    m_object_proxy = object_proxy;
    //m_object_proxy
}

void ObjectDetailVisualizer::doDraw(mono::IRenderer& renderer) const
{
    if(!m_object_proxy)
        return;


    const std::vector<Attribute>& attributes = m_object_proxy->GetAttributes();
    const math::Vector& position = m_object_proxy->Entity()->Position();

    for(const Attribute& attribute : attributes)
    {
        if(attribute.id == world::RADIUS_ATTRIBUTE || attribute.id == world::TRIGGER_RADIUS_ATTRIBUTE)
        {
            const float radius = attribute.attribute;

            constexpr mono::Color::RGBA color(1.0f, 0.0f, 1.0f);
            renderer.DrawCircle(position, radius, 20, 1.0f, color);
        }
    }
}

math::Quad ObjectDetailVisualizer::BoundingBox() const
{
    return math::InfQuad;
}
