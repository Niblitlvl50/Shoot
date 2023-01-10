
#pragma once

#include "MonoFwd.h"
#include "Math/MathFwd.h"
#include <vector>
#include <memory>
#include <string>

struct Component;

namespace editor
{
    class IObjectProxy
    {
    public:

        virtual ~IObjectProxy()
        { }

        virtual uint32_t Uuid() const = 0;
        virtual uint32_t Id() const = 0;
        virtual uint32_t GetEntityProperties() const = 0;

        virtual std::string Name() const = 0;
        virtual std::string GetFolder() const = 0;
        virtual bool IsLocked() const = 0;

        virtual bool Intersects(const math::Vector& position) const = 0;
        virtual bool Intersects(const math::Quad& world_bb) const = 0;
        virtual std::vector<struct Grabber> GetGrabbers() = 0;

        virtual const std::vector<Component>& GetComponents() const = 0;
        virtual std::vector<Component>& GetComponents() = 0;

        virtual Component* GetComponentFromHash(uint32_t component_hash) = 0;

        virtual void ComponentChanged(Component& component, uint32_t attribute_hash) = 0;

        virtual math::Vector GetPosition() const = 0;
        virtual void SetPosition(const math::Vector& position) = 0;

        virtual float GetRotation() const = 0;
        virtual void SetRotation(float rotation) = 0;

        virtual math::Quad GetBoundingBox() const = 0;

        virtual std::unique_ptr<IObjectProxy> Clone() const = 0;
    };
}

using IObjectProxyPtr = std::unique_ptr<editor::IObjectProxy>;
