
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

namespace editor
{
    class PathEntity;
    class PolygonEntity;
    class Prefab;

    class Editor : public mono::ZoneBase
    {
    public:

        Editor(System::IWindow* window, mono::EventHandler& event_handler, const char* file_name);
        virtual ~Editor();

        void OnLoad(mono::ICameraPtr& camera) override;
        int OnUnload() override;

        void Load();
        void Save();

        bool OnSurfaceChanged(const event::SurfaceChangedEvent& event);

        void AddPolygon(const std::shared_ptr<editor::PolygonEntity>& polygon);
        void AddPath(const std::shared_ptr<editor::PathEntity>& path);
        void AddPrefab(const std::shared_ptr<editor::Prefab>& prefab);

        void SelectProxyObject(IObjectProxy* proxy_object);
        IObjectProxy* FindProxyObject(const math::Vector& position);

        void SelectGrabber(const math::Vector& position);
        Grabber* FindGrabber(const math::Vector& position);

        void UpdateSnappers();
        void UpdateGrabbers();

        SnapPair FindSnapPosition(const math::Vector& position) const;

        float GetPickingDistance() const;

        void OnDeleteObject();
        void OnContextMenu(int index);
        void EditorMenuCallback(EditorMenuOptions index);
        void ToolsMenuCallback(ToolsMenuOptions index);
        void DropItemCallback(const std::string& id, const math::Vector& position);

        bool DrawObjectNames() const;
        void EnableDrawObjectNames(bool enable);

        bool DrawSnappers() const;
        void EnableDrawSnappers(bool enable);

        const mono::Color::RGBA& BackgroundColor() const;
        void SetBackgroundColor(const mono::Color::RGBA& color);

        void DuplicateSelected();

    private:

        System::IWindow* m_window;
        mono::EventHandler& m_eventHandler;
        const char* m_fileName;
        mono::ICameraPtr m_camera;

        editor::UIContext m_context;
        EntityRepository m_entityRepository;
        ObjectFactory m_object_factory;

        std::unique_ptr<ImGuiInputHandler> m_input_handler;
        std::shared_ptr<ImGuiRenderer> m_guiRenderer;
        std::shared_ptr<class UserInputController> m_userInputController;
        std::shared_ptr<class ObjectDetailVisualizer> m_object_detail_visualizer;

        std::vector<editor::Grabber> m_grabbers;
        std::vector<editor::SnapPoint> m_snap_points;

        unsigned int m_seleced_id;
        std::vector<IObjectProxyPtr> m_proxies;
        
        mono::EventToken<event::SurfaceChangedEvent> m_surfaceChangedToken;
    };
}
