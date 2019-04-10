
#pragma once

#include "IObjectProxy.h"
#include <memory>

namespace editor
{
    class Prefab;

    class PrefabProxy : public IObjectProxy
    {
    public:

        PrefabProxy(const std::shared_ptr<Prefab>& prefab);
        ~PrefabProxy();

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
        
        std::shared_ptr<Prefab> m_prefab;
        std::vector<Component> m_components;
    };
}
