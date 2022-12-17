
#pragma once

#include "MonoFwd.h"
#include "IObjectProxy.h"
#include <vector>
#include <string>

struct Component;

namespace editor
{
    class Editor;

    class ComponentProxy : public IObjectProxy
    {
    public:

        ComponentProxy(
            uint32_t entity_id,
            const std::vector<Component>& components,
            mono::IEntityManager* entity_manager,
            mono::TransformSystem* transform_system,
            Editor* editor);
        
        ~ComponentProxy();

        uint32_t Uuid() const override;
        uint32_t Id() const override;
        std::string Name() const override;
        std::string GetFolder() const override;
        bool IsLocked() const override;

        bool Intersects(const math::Vector& position) const override;
        bool Intersects(const math::Quad& world_bb) const override;
        std::vector<Grabber> GetGrabbers() override;

        void SetName(const std::string& name);
        void SetFolder(const std::string& folder);

        const std::vector<Component>& GetComponents() const override;
        std::vector<Component>& GetComponents() override;

        void ComponentChanged(Component& component, uint32_t attribute_hash) override;

        uint32_t GetEntityProperties() const override;
        void SetEntityProperties(uint32_t properties);

        math::Vector GetPosition() const override;
        void SetPosition(const math::Vector& position) override;
        float GetRotation() const override;
        void SetRotation(float rotation) override;
        math::Quad GetBoundingBox() const override;

        std::unique_ptr<IObjectProxy> Clone() const override;

    private:

        const uint32_t m_entity_id;
        std::vector<Component> m_components;
        mono::IEntityManager* m_entity_manager;
        mono::TransformSystem* m_transform_system;
        Editor* m_editor;
    };
}
