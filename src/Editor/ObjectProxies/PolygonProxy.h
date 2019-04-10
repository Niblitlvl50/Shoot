
#pragma once

#include "IObjectProxy.h"
#include <memory>

namespace editor
{
    class PolygonEntity;

    class PolygonProxy : public editor::IObjectProxy
    {
    public:

        PolygonProxy(const std::shared_ptr<PolygonEntity>& polygon);
        ~PolygonProxy();

        virtual const char* Name() const;
        virtual unsigned int Id() const;
        virtual mono::IEntityPtr Entity();
        virtual void SetSelected(bool selected);
        virtual bool Intersects(const math::Vector& position) const;
        virtual std::vector<Grabber> GetGrabbers() const;
        virtual std::vector<SnapPoint> GetSnappers() const;
        virtual void UpdateUIContext(UIContext& context);
        virtual const std::vector<Component>& GetComponents() const;
        virtual std::vector<Component>& GetComponents();
        virtual std::unique_ptr<IObjectProxy> Clone() const;
        virtual void Visit(IObjectVisitor& visitor);

        std::shared_ptr<PolygonEntity> m_polygon;
        std::vector<Component> m_components;
    };
}
