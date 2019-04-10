
#pragma once

#include "System/System.h"

#include "Zone/ZoneBase.h"
#include "Events/EventFwd.h"
#include "EventHandler/EventToken.h"

#include "UI/MainMenuOptions.h"
#include "UI/UIContext.h"

#include "ObjectProxies/IObjectProxy.h"
#include "Grabber.h"
#include "SnapPoint.h"

#include "EntityRepository.h"
#include "ObjectFactory.h"


class ImGuiInputHandler;
class ImGuiRenderer;
class IEntityManager;

namespace editor
{
    class PathEntity;
    class PolygonEntity;
    class Prefab;

    class Editor : public mono::ZoneBase
    {
    public:

        Editor(
            System::IWindow* window,
            IEntityManager& entity_manager,
            mono::EventHandler& event_handler,
            mono::SystemContext& system_context,
            const char* world_filename
        );

        virtual ~Editor();

        void OnLoad(mono::ICameraPtr& camera) override;
        int OnUnload() override;
        void Accept(mono::IRenderer& renderer) override;

        void Load();
        void Save();
        void ImportEntity();
        void ExportEntity();

        bool OnSurfaceChanged(const event::SurfaceChangedEvent& event);

        void AddPolygon(const std::shared_ptr<editor::PolygonEntity>& polygon);
        void AddPath(const std::shared_ptr<editor::PathEntity>& path);
        void AddPrefab(const std::shared_ptr<editor::Prefab>& prefab);

        void SelectProxyObject(IObjectProxy* proxy_object);
        IObjectProxy* FindProxyObject(const math::Vector& position);
        IObjectProxy* FindProxyObject(uint32_t proxy_id) const;

        void SelectGrabber(const math::Vector& position);
        Grabber* FindGrabber(const math::Vector& position);

        void UpdateSnappers();
        void UpdateGrabbers();

        SnapPair FindSnapPosition(const math::Vector& position) const;

        float GetPickingDistance() const;

        void NewEntity();
        void OnDeleteObject();

        void AddComponent(uint32_t component_hash);
        void DeleteComponent(uint32_t index);

        void EntityComponentUpdated(uint32_t entity_id, uint32_t component_hash);

        void OnContextMenu(int index);
        void SelectItemCallback(int index);
        void EditorMenuCallback(EditorMenuOptions index);
        void ToolsMenuCallback(ToolsMenuOptions index);

        bool DrawObjectNames() const;
        void EnableDrawObjectNames(bool enable);

        bool DrawSnappers() const;
        void EnableDrawSnappers(bool enable);

        bool DrawOutline() const;
        void EnableDrawOutline(bool enable);

        const mono::Color::RGBA& BackgroundColor() const;
        void SetBackgroundColor(const mono::Color::RGBA& color);

        void DuplicateSelected();

    private:

        System::IWindow* m_window;
        IEntityManager& m_entity_manager;
        mono::EventHandler& m_event_handler;
        mono::SystemContext& m_system_context;
        const char* m_world_filename;

        mono::ICameraPtr m_camera;

        editor::UIContext m_context;
        EntityRepository m_entity_repository;
        ObjectFactory m_object_factory;

        std::unique_ptr<ImGuiInputHandler> m_input_handler;
        std::shared_ptr<ImGuiRenderer> m_gui_renderer;
        std::shared_ptr<class UserInputController> m_user_input_controller;
        std::shared_ptr<class ComponentDetailVisualizer> m_component_detail_visualizer;

        uint32_t m_selected_id;
        std::vector<IObjectProxyPtr> m_proxies;
        std::vector<editor::Grabber> m_grabbers;
        std::vector<SnapPoint> m_snap_points;
        
        mono::EventToken<event::SurfaceChangedEvent> m_surface_changed_token;
    };
}
