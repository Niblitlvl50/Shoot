
#pragma once

#include "IObjectProxy.h"
#include "ObjectAttribute.h"
#include <memory>

namespace editor
{
    class SpriteEntity;

    class EntityProxy : public editor::IObjectProxy
    {
    public:

        EntityProxy(const std::shared_ptr<SpriteEntity>& entity, const std::vector<unsigned int>& attribute_types);

        virtual const char* Name() const;
        virtual unsigned int Id() const;
        virtual mono::IEntityPtr Entity();
        virtual void SetSelected(bool selected);
        virtual bool Intersects(const math::Vector& position) const;
        virtual std::vector<Grabber> GetGrabbers() const;
        virtual std::vector<SnapPoint> GetSnappers() const;
        virtual void UpdateUIContext(UIContext& context);
        virtual std::vector<ID_Attribute> GetAttributes() const;
        virtual void SetAttributes(const std::vector<ID_Attribute>& attributes);

        virtual void Visit(IObjectVisitor& visitor);
        
    private:
        std::shared_ptr<SpriteEntity> m_entity;
        std::vector<ID_Attribute> m_attributes;
        const std::vector<unsigned int> m_attribute_types;
    };
}
