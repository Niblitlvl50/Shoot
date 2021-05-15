
#pragma once

#include "MonoFwd.h"
#include "IObjectProxy.h"
#include <string>

namespace editor
{
    class Editor;

    class PathProxy : public editor::IObjectProxy
    {
    public:

        PathProxy(
            uint32_t entity_id,
            const std::string& name,
            const std::string& folder,
            const std::vector<Component>& components,
            mono::IEntityManager* entity_manager,
            mono::TransformSystem* transform_system,
            Editor* editor);

        ~PathProxy();

        const char* Name() const override;
        uint32_t Id() const override;
        void SetSelected(bool selected) override;
        bool Intersects(const math::Vector& position) const override;
        bool Intersects(const math::Quad& world_bb) const override;
        std::vector<Grabber> GetGrabbers() override;
        std::vector<SnapPoint> GetSnappers() const override;
        void UpdateUIContext(UIContext& context) override;
        std::string GetFolder() const override;
        const std::vector<Component>& GetComponents() const override;
        std::vector<Component>& GetComponents() override;

        math::Vector GetPosition() const override;
        void SetPosition(const math::Vector& position) override;
        float GetRotation() const override;
        void SetRotation(float rotation) override;
        math::Quad GetBoundingBox() const override;

        std::unique_ptr<IObjectProxy> Clone() const override;
        void Visit(IObjectVisitor& visitor) override;

        const uint32_t m_entity_id;
        std::string m_name;
        std::string m_folder;
        std::vector<Component> m_components;
        mono::IEntityManager* m_entity_manager;
        mono::TransformSystem* m_transform_system;
        Editor* m_editor;
    };
}
