
#pragma once

#include "IObjectProxy.h"
#include <memory>

namespace editor
{
    class PathEntity;
    class Editor;

    class PathProxy : public editor::IObjectProxy
    {
    public:

        PathProxy(const std::shared_ptr<PathEntity>& path, Editor* editor);
        ~PathProxy();

        const char* Name() const override;
        unsigned int Id() const override;
        mono::IEntityPtr Entity() override;
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
        
        std::shared_ptr<PathEntity> m_path;
        Editor* m_editor;

        std::vector<Component> m_components;
    };
}
