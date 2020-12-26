
#pragma once

#include "System/System.h"

#include "Zone/ZoneBase.h"
#include "Events/EventFwd.h"

#include "UI/MainMenuOptions.h"
#include "UI/UIContext.h"

#include "ObjectProxies/IObjectProxy.h"
#include "Grabber.h"
#include "SnapPoint.h"

#include "ObjectFactory.h"

#include <memory>

class ImGuiInputHandler;

namespace editor
{
    class PathEntity;
    class PolygonEntity;
    struct Config;

    constexpr uint32_t NO_SELECTION = std::numeric_limits<uint32_t>::max();

    class Editor : public mono::ZoneBase
    {
    public:

        Editor(
            System::IWindow* window,
            mono::IEntityManager& entity_manager,
            mono::EventHandler& event_handler,
            mono::SystemContext& system_context,
            Config& editor_config
        );

        virtual ~Editor();

        void OnLoad(mono::ICamera* camera, mono::IRenderer* renderer) override;
        int OnUnload() override;
        void PostUpdate() override;

        void SwitchWorld(const std::string& new_world_filename);
        void LoadWorld(const std::string& world_filename);

        void Save();
        void ImportEntity();
        void ExportEntity();

        void AddPath(std::unique_ptr<editor::PathEntity> path);

        void SelectProxyObject(IObjectProxy* proxy_object);
        void PreselectProxyObject(IObjectProxy* proxy_object);
        void TeleportToProxyObject(IObjectProxy* proxy_object);
        void TeleportToSelectedProxyObject();
        IObjectProxy* FindProxyObject(const math::Vector& position);
        IObjectProxy* FindProxyObject(uint32_t proxy_id) const;
        uint32_t GetSelectedObjectId() const;

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
        void AddComponentUI();

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

        bool DrawLevelMetadata() const;
        void EnableDrawLevelMetadata(bool enable);

        const mono::Color::RGBA& BackgroundColor() const;
        void SetBackgroundColor(const mono::Color::RGBA& color);

        bool SnapToGrid() const;
        void EnableSnapToGrid(bool enable);
        math::Vector GridSize() const;

        void EnableDrawAllObjects(bool enable);
        bool DrawAllObjects() const;

        void DuplicateSelected();
        void ReExportEntities();

    private:

        System::IWindow* m_window;
        mono::IEntityManager& m_entity_manager;
        mono::EventHandler& m_event_handler;
        mono::SystemContext& m_system_context;
        Config& m_editor_config;

        std::string m_world_filename;
        std::string m_new_world_filename;

        mono::ICamera* m_camera;
        mono::IRenderer* m_renderer;

        editor::UIContext m_context;
        ObjectFactory m_object_factory;

        std::unique_ptr<ImGuiInputHandler> m_input_handler;
        std::unique_ptr<class UserInputController> m_user_input_controller;
        std::unique_ptr<class ComponentDetailVisualizer> m_component_detail_visualizer;

        uint32_t m_selected_id;
        uint32_t m_preselected_id;
        std::vector<IObjectProxyPtr> m_proxies;
        std::vector<editor::Grabber> m_grabbers;
        std::vector<SnapPoint> m_snap_points;
    };
}
