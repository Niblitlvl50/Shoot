
#pragma once

#include "IObjectProxy.h"
#include <memory>

namespace editor
{
    class PolygonEntity;

    class PolygonProxy : public editor::IObjectProxy
    {
    public:

        PolygonProxy(std::unique_ptr<PolygonEntity> polygon);
        ~PolygonProxy();

        const char* Name() const override;
        unsigned int Id() const override;
        mono::IEntity* Entity() override;
        void SetSelected(bool selected) override;
        bool Intersects(const math::Vector& position) const override;
        std::vector<Grabber> GetGrabbers() const override;
        std::vector<SnapPoint> GetSnappers() const override;
        void UpdateUIContext(UIContext& context) override;
        const std::vector<Component>& GetComponents() const override;
        std::vector<Component>& GetComponents() override;
        float GetRotation() const override;
        void SetRotation(float rotation) override;
        math::Vector GetPosition() const override;
        void SetPosition(const math::Vector& position) override;
        std::unique_ptr<IObjectProxy> Clone() const override;
        void Visit(IObjectVisitor& visitor) override;

        std::unique_ptr<PolygonEntity> m_polygon;
        std::vector<Component> m_components;
    };
}
