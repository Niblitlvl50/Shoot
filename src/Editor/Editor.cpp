
#include "Editor.h"

#include "Rendering/ICamera.h"
#include "Rendering/Texture/TextureFactory.h"
#include "Rendering/Texture/ITexture.h"

#include "Rendering/Sprite/ISprite.h"
#include "Rendering/Sprite/SpriteFactory.h"

#include "Events/EventFuncFwd.h"
#include "Events/SurfaceChangedEvent.h"
#include "EventHandler/EventHandler.h"

#include "UserInputController.h"
#include "UI/ImGuiInterfaceDrawer.h"
#include "ImGuiImpl/ImGuiRenderer.h"
#include "EditorConfig.h"
#include "Textures.h"
#include "RenderLayers.h"
#include "WorldSerializer.h"

#include "ObjectProxies/PolygonProxy.h"
#include "ObjectProxies/PathProxy.h"
#include "ObjectProxies/PrefabProxy.h"

#include "Objects/Polygon.h"
#include "Objects/Path.h"
#include "Objects/Prefab.h"

#include "Visualizers/GridVisualizer.h"
#include "Visualizers/GrabberVisualizer.h"

namespace
{
    void SetupIcons(
        editor::UIContext& context,
        editor::EntityRepository& repository,
        std::unordered_map<unsigned int, mono::ITexturePtr>& textures)
    {
        mono::ITexturePtr texture = mono::CreateTexture("res/textures/placeholder.png");
        textures.insert(std::make_pair(texture->Id(), texture));

        context.tools_texture_id = texture->Id();
        context.default_icon = math::Quad(0.0f, 0.0f, 1.0f, 1.0f);

        for(const editor::EntityDefinition& def : repository.m_entities)
        {
            const mono::ISpritePtr sprite = mono::CreateSprite(def.sprite_file.c_str());
            const mono::ITexturePtr sprite_texture = sprite->GetTexture();

            textures.insert(std::make_pair(sprite_texture->Id(), sprite_texture));

            editor::UIEntityItem item;
            item.texture_id = sprite_texture->Id();
            item.icon = sprite->GetTextureCoords();
            item.tooltip = def.name;
            context.entity_items.push_back(item);
        }
    }
}

using namespace editor;

Editor::Editor(System::IWindow* window, mono::EventHandler& event_handler, const char* file_name)
    : m_window(window),
      m_eventHandler(event_handler),
      m_inputHandler(event_handler),
      m_fileName(file_name),
      m_object_factory(this),
      m_seleced_id(-1)
{
    using namespace std::placeholders;

    m_context.context_menu_callback = std::bind(&Editor::OnContextMenu, this, _1);
    m_context.delete_callback = std::bind(&Editor::OnDeleteObject, this);
    m_context.editor_menu_callback = std::bind(&Editor::EditorMenuCallback, this, _1);
    m_context.tools_menu_callback = std::bind(&Editor::ToolsMenuCallback, this, _1);
    m_context.drop_callback = std::bind(&Editor::DropItemCallback, this, _1, _2);

    const event::SurfaceChangedEventFunc surface_func = std::bind(&Editor::OnSurfaceChanged, this, _1);
    m_surfaceChangedToken = m_eventHandler.AddListener(surface_func);

    m_entityRepository.LoadDefinitions();

    std::unordered_map<unsigned int, mono::ITexturePtr> textures;
    SetupIcons(m_context, m_entityRepository, textures);

    const System::Size& size = m_window->Size();
    const math::Vector window_size(size.width, size.height);
    m_guiRenderer = std::make_shared<ImGuiRenderer>("res/editor_imgui.ini", window_size, textures);

    Load();
}

Editor::~Editor()
{
    m_eventHandler.RemoveListener(m_surfaceChangedToken);

    editor::Config config;
    config.cameraPosition = m_camera->GetPosition();
    config.cameraViewport = m_camera->GetViewport();
    editor::SaveConfig("res/editor_config.json", config);
    
    Save();
}

void Editor::OnLoad(mono::ICameraPtr& camera)
{
    m_camera = camera;
    
    editor::Config config;
    const bool config_loaded = editor::LoadConfig("res/editor_config.json", config);
    if(config_loaded)
    {
        camera->SetPosition(config.cameraPosition);
        camera->SetViewport(config.cameraViewport);
    }

    m_userInputController = std::make_shared<editor::UserInputController>(camera, m_window, this, &m_context, m_eventHandler);

    AddUpdatable(std::make_shared<editor::ImGuiInterfaceDrawer>(m_context));
    AddDrawable(m_guiRenderer, RenderLayer::UI);
    AddDrawable(std::make_shared<GridVisualizer>(camera), RenderLayer::BACKGROUND);
    AddDrawable(std::make_shared<GrabberVisualizer>(m_grabbers), RenderLayer::GRABBERS);
}

void Editor::OnUnload()
{ }

void Editor::Load()
{
    auto polygons = LoadPolygons(m_fileName, m_object_factory);
    for(auto& proxy : polygons)
    {
        AddEntity(proxy->Entity(), RenderLayer::OBJECTS);
        
        m_polygons.push_back(proxy.get());
        m_proxies.push_back(std::move(proxy));
    }

    auto paths = LoadPaths("res/world.paths", m_object_factory);
    for(auto& proxy : paths)
    {
        AddEntity(proxy->Entity(), RenderLayer::OBJECTS);
        
        m_paths.push_back(proxy.get());
        m_proxies.push_back(std::move(proxy));
    }

    auto prefabs = LoadPrefabs("res/world.prefabs", m_object_factory);
    for(auto& proxy : prefabs)
    {
        AddEntity(proxy->Entity(), RenderLayer::OBJECTS);
        
        m_prefabs.push_back(proxy.get());
        m_proxies.push_back(std::move(proxy));
    }

    //auto objects = LoadObjects("res/world.objects", m_object_factory);
    auto objects = LoadObjects2("hello.bin", m_object_factory);
    for(auto& proxy : objects)
    {
        AddEntity(proxy->Entity(), RenderLayer::OBJECTS);

        m_objects.push_back(proxy.get());
        m_proxies.push_back(std::move(proxy));
    }
}

void Editor::Save()
{
    SavePolygons(m_fileName, m_polygons);
    SavePaths("res/world.paths", m_paths);
    SavePrefabs("res/world.prefabs", m_prefabs);

    SaveObjects2("hello.bin", m_objects);
}

bool Editor::OnSurfaceChanged(const event::SurfaceChangedEvent& event)
{
    if(event.width > 0 && event.height > 0)
        m_guiRenderer->SetWindowSize(math::Vector(event.width, event.height));

    return false;
}

void Editor::AddPolygon(const std::shared_ptr<editor::PolygonEntity>& polygon)
{
    AddEntity(polygon, RenderLayer::OBJECTS);
    
    auto proxy = std::make_unique<PolygonProxy>(polygon);    
    m_polygons.push_back(proxy.get());
    m_proxies.push_back(std::move(proxy));
}

void Editor::AddPath(const std::shared_ptr<editor::PathEntity>& path)
{
    AddEntity(path, RenderLayer::OBJECTS);

    auto path_proxy = std::make_unique<PathProxy>(path, this);
    m_paths.push_back(path_proxy.get());
    m_proxies.push_back(std::move(path_proxy));
}

void Editor::AddPrefab(const std::shared_ptr<editor::Prefab>& prefab)
{
    AddEntity(prefab, RenderLayer::OBJECTS);

    auto prefab_proxy = std::make_unique<PrefabProxy>(prefab);
    m_prefabs.push_back(prefab_proxy.get());
    m_proxies.push_back(std::move(prefab_proxy));
}

void Editor::SelectProxyObject(IObjectProxy* proxy_object)
{
    m_seleced_id = -1;
    m_context.proxy_object = proxy_object;

    for(auto& proxy : m_proxies)
        proxy->SetSelected(false);

    if(proxy_object)
    {
        proxy_object->SetSelected(true);
        m_seleced_id = proxy_object->Id();
    }

    m_snap_points.clear();

    for(const auto& proxy : m_proxies)
    {
        // Skip the selected one, we dont want that one
        if(proxy->Id() == m_seleced_id)
            continue;

        const std::vector<SnapPoint>& snappers = proxy->GetSnappers();
        m_snap_points.insert(m_snap_points.end(), snappers.begin(), snappers.end());
    }

    UpdateGrabbers();
}

IObjectProxy* Editor::FindProxyObject(const math::Vector& position)
{
    for(auto& proxy : m_proxies)
    {
        if(proxy->Intersects(position))
            return proxy.get();
    }

    return nullptr;
}

void Editor::SelectGrabber(const math::Vector& position)
{
    for(auto& grabber : m_grabbers)
        grabber.hoover = false;

    editor::Grabber* grabber = FindGrabber(position);
    if(grabber)
        grabber->hoover = true;
}

editor::Grabber* Editor::FindGrabber(const math::Vector& position)
{
    const float threshold = GetPickingDistance();

    for(auto& grabber : m_grabbers)
    {
        const float distance = math::Length(grabber.position - position);
        if(distance <= threshold)
            return &grabber;
    }

    return nullptr;
}

void Editor::UpdateGrabbers()
{
    m_grabbers.clear();

    const unsigned int id = m_seleced_id;

    const auto find_func = [id](const IObjectProxyPtr& proxy) {
        return id == proxy->Id();
    };

    auto it = std::find_if(m_proxies.begin(), m_proxies.end(), find_func);
    if(it != m_proxies.end())
        m_grabbers = (*it)->GetGrabbers();
}

float Editor::GetPickingDistance() const
{
    const System::Size& size = m_window->Size();
    return m_camera->GetViewport().mB.x / size.width * 5.0f;
}

std::pair<int, math::Vector> Editor::FindSnapPosition(const math::Vector& position) const
{
    std::vector<SnapPoint> snappers;

    const unsigned int id = m_seleced_id;
    const auto find_func = [id](const IObjectProxyPtr& proxy) {
        return id == proxy->Id();
    };

    auto it = std::find_if(m_proxies.begin(), m_proxies.end(), find_func);
    if(it != m_proxies.end())
        snappers = (*it)->GetSnappers();

    int best_index = -1;
    float best_distance = math::INF;
    math::Vector snapped_point = position;

    for(size_t index = 0; index < snappers.size(); ++index)
    {
        const SnapPoint& snap_point = snappers[index];
        for(const SnapPoint& other : m_snap_points)
        {
            const float distance = math::Length(snap_point.position - other.position);
            if(distance < 0.2f && distance < best_distance)
            {
                best_index = index;
                best_distance = distance;
                snapped_point = other.position;
            }
        }
    }

    if(best_index != -1)
        snapped_point = position - snapped_point;

    return { best_index, snapped_point };
}

void Editor::OnDeleteObject()
{
    const unsigned int id = m_seleced_id;

    const auto find_func = [id](const IObjectProxyPtr& proxy) {
        return id == proxy->Id();
    };

    auto it = std::find_if(m_proxies.begin(), m_proxies.end(), find_func);
    if(it != m_proxies.end())
    {
        editor::IObjectProxy* proxy = it->get();
        
        auto path_it = std::remove(m_paths.begin(), m_paths.end(), proxy);
        if(path_it != m_paths.end())
            m_paths.erase(path_it);
            
        auto polygon_it = std::remove(m_polygons.begin(), m_polygons.end(), proxy);
        if(polygon_it != m_polygons.end())
            m_polygons.erase(polygon_it);
    
        auto prefab_it = std::remove(m_prefabs.begin(), m_prefabs.end(), proxy);
        if(prefab_it != m_prefabs.end())
            m_prefabs.erase(prefab_it);

        auto object_it = std::remove(m_objects.begin(), m_objects.end(), proxy);
        if(object_it != m_objects.end())
            m_objects.erase(object_it);
    
        m_proxies.erase(it);
    }

    auto entity = FindEntityFromId(id);    
    const auto remove_entity_func = [this, entity] {
        RemoveEntity(entity);
    };

    SchedulePreFrameTask(remove_entity_func);

    m_context.proxy_object = nullptr;
    m_grabbers.clear();
}

void Editor::OnContextMenu(int index)
{
    m_userInputController->HandleContextMenu(index);
}

void Editor::EditorMenuCallback(EditorMenuOptions option)
{
    if(option == EditorMenuOptions::SAVE)
        Save();
}

void Editor::ToolsMenuCallback(ToolsMenuOptions option)
{
    m_userInputController->SelectTool(option);
}

void Editor::DropItemCallback(const std::string& id, const math::Vector& position)
{
    const System::Size& size = m_window->Size();
    const math::Vector window_size(size.width, size.height);
    const math::Vector& world_pos = m_camera->ScreenToWorld(position, window_size);

    IObjectProxyPtr proxy = m_object_factory.CreateObject(id.c_str());
    
    mono::IEntityPtr entity = proxy->Entity();
    entity->SetPosition(world_pos);
    
    AddEntity(entity, RenderLayer::OBJECTS);

    m_objects.push_back(proxy.get());
    m_proxies.push_back(std::move(proxy));
}
