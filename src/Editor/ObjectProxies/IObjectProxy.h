
#pragma once

#include "MonoPtrFwd.h"
#include "Math/MathFwd.h"
#include <vector>

struct ID_Attribute;

namespace editor
{
    class IObjectProxy
    {
    public:

        virtual ~IObjectProxy()
        { }

        virtual const char* Name() const = 0;
        virtual unsigned int Id() const = 0;
        virtual mono::IEntityPtr Entity() = 0;

        virtual void SetSelected(bool selected) = 0;
        virtual bool Intersects(const math::Vector& position) const = 0;
        virtual std::vector<struct Grabber> GetGrabbers() const = 0;
        virtual std::vector<struct SnapPoint> GetSnappers() const = 0;

        virtual void UpdateUIContext(struct UIContext& context) = 0;

        virtual std::vector<ID_Attribute> GetAttributes() const = 0;
        virtual void SetAttributes(const std::vector<ID_Attribute>& attributes) = 0;

        virtual void Visit(class IObjectVisitor& visitor) = 0;
    };
}

using IObjectProxyPtr = std::unique_ptr<editor::IObjectProxy>;
