
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

        virtual const char* Name() const;
        virtual unsigned int Id() const;
        virtual mono::IEntityPtr Entity();
        virtual void SetSelected(bool selected);
        virtual bool Intersects(const math::Vector& position) const;
        virtual std::vector<Grabber> GetGrabbers() const;
        virtual std::vector<SnapPoint> GetSnappers() const;
        virtual void UpdateUIContext(UIContext& context);
        virtual std::vector<Attribute> GetAttributes() const;
        virtual void SetAttributes(const std::vector<Attribute>& attributes);
        virtual void Visit(IObjectVisitor& visitor);

        std::shared_ptr<PolygonEntity> m_polygon;
    };
}
