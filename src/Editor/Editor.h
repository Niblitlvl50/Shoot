
#pragma once

#include "System/System.h"
#include "ISelectionHandler.h"
#include "Zone/ZoneBase.h"
#include "Events/EventFwd.h"

#include "UI/MainMenuOptions.h"
#include "UI/UIContext.h"

#include "ObjectProxies/IObjectProxy.h"
#include "Grabber.h"
#include "SnapPoint.h"

#include <memory>
#include <stack>

class ImGuiInputHandler;

namespace mono
{
    class StaticBackground;
}

namespace editor
{
    constexpr uint32_t NO_SELECTION = std::numeric_limits<uint32_t>::max();

    enum class EditorMode
    {
        DEFAULT,
        REFERENCE_PICKING,
    };

    class Editor : public mono::ZoneBase, public editor::ISelectionHandler
    {
    public:

        Editor(
            System::IWindow* window,
            mono::IEntityManager& entity_manager,
            mono::EventHandler& event_handler,
            mono::SystemContext& system_context,
            struct Config& editor_config,
            uint32_t max_entities
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

        void AddPath(const std::vector<math::Vector>& path_points);

        void SetSelection(const Selection& selected_ids) override;
        const Selection& GetSelection() const override;
        void AddToSelection(const Selection& selected_ids) override;
        void RemoveFromSelection(const Selection& selected_ids) override;
        void ClearSelection() override;
        void UpdateSelection();
        void MoveSelectionToHere();

        void SetSelectionPoint(const math::Vector& selection_point);
        void SetSelectionBox(const math::Quad& selection_box);

        void PreselectProxyObject(IObjectProxy* proxy_object);

        void TeleportToProxyObject(const IObjectProxy* proxy_object);
        void TeleportToProxyObject(const std::vector<const IObjectProxy*>& proxies);
        void TeleportToSelectedProxyObject();
        IObjectProxy* FindProxyObject(const math::Vector& position);
        std::vector<IObjectProxy*> FindProxiesFromBox(const math::Quad& world_bb) const;
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
        void AddComponentUI();

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

        bool DrawLights() const;
        void EnableLights(bool enable);

        const mono::Color::RGBA& BackgroundColor() const;
        void SetBackgroundColor(const mono::Color::RGBA& color);
        const mono::Color::RGBA& AmbientShade() const;
        void SetAmbientShade(const mono::Color::RGBA& color);

        void SetBackgroundTexture(const math::Vector& size, const std::string& background_texture);

        bool DrawGrid() const;
        void EnableDrawGrid(bool enable);

        bool SnapToGrid() const;
        void EnableSnapToGrid(bool enable);
        math::Vector GridSize() const;

        void EnableDrawAllObjects(bool enable);
        bool DrawAllObjects() const;

        void DuplicateSelected();
        void ReExportEntities();

        void SetPickingTarget(uint32_t* target_data);

        void EnterMode(EditorMode new_mode);
        void PopMode();

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

        std::unique_ptr<ImGuiInputHandler> m_input_handler;
        std::unique_ptr<class UserInputController> m_user_input_controller;
        std::unique_ptr<class ComponentDetailVisualizer> m_component_detail_visualizer;
        class SelectionVisualizer* m_selection_visualizer; 
        std::unique_ptr<mono::StaticBackground> m_static_background;

        Selection m_selected_ids;
        uint32_t m_preselected_id;
        std::stack<EditorMode> m_mode_stack;
        uint32_t* m_pick_target;

        std::vector<IObjectProxyPtr> m_proxies;
        std::vector<editor::Grabber> m_grabbers;
        std::vector<SnapPoint> m_snap_points;
    };
}
