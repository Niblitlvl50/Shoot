
#pragma once

#include "Rendering/IDrawable.h"
#include "Rendering/RenderPtrFwd.h"

#include <vector>
#include <unordered_map>

struct Attribute;

namespace mono
{
    class TransformSystem;
}

namespace editor
{
    class IObjectProxy;

    using ComponentDrawFunc =
        void(*)(mono::IRenderer& renderer, const math::Vector& position, const std::vector<Attribute>& component_attributes);
    using ComponentDrawMap = std::unordered_map<uint32_t, ComponentDrawFunc>;

    class ComponentDetailVisualizer : public mono::IDrawable
    {
    public:

        ComponentDetailVisualizer(const ComponentDrawMap& draw_funcs, const mono::TransformSystem* transform_system);
        void SetObjectProxy(IObjectProxy* object_proxy);

        virtual void doDraw(mono::IRenderer& renderer) const;
        virtual math::Quad BoundingBox() const;

    private:
        const ComponentDrawMap m_component_draw_funcs;
        const mono::TransformSystem* m_transform_system;
        IObjectProxy* m_object_proxy = nullptr;
    };
}
