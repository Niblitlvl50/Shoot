
#include "ComponentDetailVisualizer.h"

#include "ObjectProxies/IObjectProxy.h"

#include "Math/Quad.h"
#include "TransformSystem/TransformSystem.h"

#include "EntitySystem/ObjectAttribute.h"
#include "Component.h"

using namespace editor;

ComponentDetailVisualizer::ComponentDetailVisualizer(
    const ComponentDrawMap& draw_funcs, const mono::TransformSystem* transform_system)
    : m_component_draw_funcs(draw_funcs)
    , m_transform_system(transform_system)
{ }

void ComponentDetailVisualizer::SetObjectProxies(const std::vector<IObjectProxy*>& proxies)
{
    m_object_proxies = proxies;
}

void ComponentDetailVisualizer::Draw(mono::IRenderer& renderer) const
{
    for(const IObjectProxy* proxy : m_object_proxies)
    {
        const uint32_t entity_id = proxy->Id();

        const math::Matrix& entity_transform = m_transform_system->GetTransform(entity_id);
        const math::Vector& position = math::GetPosition(entity_transform);
        const float rotation = math::GetZRotation(entity_transform);

        for(const Component& component : proxy->GetComponents())
        {
            const auto it = m_component_draw_funcs.find(component.hash);
            if(it != m_component_draw_funcs.end())
                it->second(renderer, position, rotation, component.properties);
        }
    }
}

math::Quad ComponentDetailVisualizer::BoundingBox() const
{
    return math::InfQuad;
}
