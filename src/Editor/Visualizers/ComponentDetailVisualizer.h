
#pragma once

#include "MonoFwd.h"
#include "Math/MathFwd.h"
#include "Rendering/IDrawable.h"

#include <cstdint>
#include <vector>
#include <unordered_map>

struct Attribute;

namespace editor
{
    class IObjectProxy;

    using ComponentDrawFunc = void(*)(mono::IRenderer& renderer, const std::vector<Attribute>& component_attributes, const math::Quad& entity_bb);
    using ComponentDrawMap = std::unordered_map<uint32_t, ComponentDrawFunc>;

    class ComponentDetailVisualizer : public mono::IDrawable
    {
    public:

        ComponentDetailVisualizer(const ComponentDrawMap& draw_funcs, const mono::TransformSystem* transform_system);
        void SetObjectProxies(const std::vector<IObjectProxy*>& proxies);

        void Draw(mono::IRenderer& renderer) const override;
        math::Quad BoundingBox() const override;

    private:
        const ComponentDrawMap m_component_draw_funcs;
        const mono::TransformSystem* m_transform_system;
        std::vector<IObjectProxy*> m_object_proxies;
    };
}
