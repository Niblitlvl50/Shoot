
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
#include "ImGuiImpl/ImGuiInputHandler.h"
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
#include "Visualizers/ScaleVisualizer.h"
#include "Visualizers/GrabberVisualizer.h"
#include "Visualizers/SnapperVisualizer.h"
#include "Visualizers/ObjectNameVisualizer.h"
#include "Visualizers/ObjectDetailVisualizer.h"

#include "Algorithm.h"
#include <algorithm>

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

        for(const PrefabDefinition& def : repository.m_prefabs)
        {
            const mono::ISpritePtr sprite = mono::CreateSprite(def.sprite_file.c_str());
            const mono::ITexturePtr sprite_texture = sprite->GetTexture();

            textures.insert(std::make_pair(sprite_texture->Id(), sprite_texture));

            editor::UIEntityItem item;
            item.texture_id = sprite_texture->Id();
            item.icon = sprite->GetTextureCoords();
            item.tooltip = def.name;
            
            context.prefab_items.push_back(item);
        }
    }

    constexpr unsigned int NO_SELECTION = std::numeric_limits<unsigned int>::max();
}

using namespace editor;

Editor::Editor(System::IWindow* window, mono::EventHandler& event_handler, const char* file_name)
    : m_window(window),
      m_eventHandler(event_handler),
      m_fileName(file_name),
      m_object_factory(this),
      m_seleced_id(NO_SELECTION)
{
    using namespace std::placeholders;

    m_context.context_menu_callback = std::bind(&Editor::OnContextMenu, this, _1);
    m_context.delete_callback = std::bind(&Editor::OnDeleteObject, this);
    m_context.editor_menu_callback = std::bind(&Editor::EditorMenuCallback, this, _1);
    m_context.tools_menu_callback = std::bind(&Editor::ToolsMenuCallback, this, _1);
    m_context.drop_callback = std::bind(&Editor::DropItemCallback, this, _1, _2);
    m_context.draw_object_names_callback = std::bind(&Editor::EnableDrawObjectNames, this, _1);
    m_context.draw_snappers_callback = std::bind(&Editor::EnableDrawSnappers, this, _1);
    m_context.background_color_callback = std::bind(&Editor::SetBackgroundColor, this, _1);

    const event::SurfaceChangedEventFunc surface_func = std::bind(&Editor::OnSurfaceChanged, this, _1);
    m_surfaceChangedToken = m_eventHandler.AddListener(surface_func);

    m_entityRepository.LoadDefinitions();

    std::unordered_map<unsigned int, mono::ITexturePtr> textures;
    SetupIcons(m_context, m_entityRepository, textures);

    const System::Size& size = m_window->Size();
    const math::Vector window_size(size.width, size.height);
    m_guiRenderer = std::make_shared<ImGuiRenderer>("res/editor_imgui.ini", window_size, textures);
    m_input_handler = std::make_unique<ImGuiInputHandler>(event_handler);

    Load();
}

Editor::~Editor()
{
    m_eventHandler.RemoveListener(m_surfaceChangedToken);    
    Save();
}

void Editor::OnLoad(mono::ICameraPtr& camera)
{
    m_camera = camera;
    m_userInputController = std::make_shared<editor::UserInputController>(camera, m_window, this, &m_context, m_eventHandler);
    m_object_detail_visualizer = std::make_shared<editor::ObjectDetailVisualizer>();

    AddUpdatable(std::make_shared<editor::ImGuiInterfaceDrawer>(m_context));

    AddDrawable(std::make_shared<GridVisualizer>(), RenderLayer::BACKGROUND);
    AddDrawable(std::make_shared<GrabberVisualizer>(m_grabbers), RenderLayer::GRABBERS);
    AddDrawable(std::make_shared<SnapperVisualizer>(m_context.draw_snappers, m_snap_points), RenderLayer::GRABBERS);
    AddDrawable(std::make_shared<ScaleVisualizer>(camera), RenderLayer::UI);
    AddDrawable(std::make_shared<ObjectNameVisualizer>(m_context.draw_object_names, m_proxies), RenderLayer::UI);
    AddDrawable(m_object_detail_visualizer, RenderLayer::OBJECTS);
    AddDrawable(m_guiRenderer, RenderLayer::UI);
}

int Editor::OnUnload()
{
    return 0;
}

void Editor::Accept(mono::IRenderer& renderer)
{
    using LayerDrawable = std::pair<int, mono::IDrawablePtr>;

    const auto sort_on_y = [](const LayerDrawable& first, const LayerDrawable& second) {
        if(first.first == second.first)
            return first.second->BoundingBox().mA.y > second.second->BoundingBox().mA.y;
        
        return first.first < second.first;
    };

    std::sort(m_drawables.begin(), m_drawables.end(), sort_on_y);
    ZoneBase::Accept(renderer);
}

void Editor::Load()
{
    m_proxies = LoadWorld(m_fileName, m_object_factory);
    for(auto& proxy : m_proxies)
    {
        const bool is_polygon = (strcmp(proxy->Name(), "polygonobject") == 0);
        const RenderLayer layer = is_polygon ? RenderLayer::POLYGONS : RenderLayer::OBJECTS;
        AddEntity(proxy->Entity(), layer);
    }

    UpdateSnappers();
}

void Editor::Save()
{
    SaveWorld(m_fileName, m_proxies);
    m_context.notifications.emplace_back(m_context.default_icon, "Saved...", 2000);
}

bool Editor::OnSurfaceChanged(const event::SurfaceChangedEvent& event)
{
    if(event.width > 0 && event.height > 0)
        m_guiRenderer->SetWindowSize(math::Vector(event.width, event.height));

    return false;
}

void Editor::AddPolygon(const std::shared_ptr<editor::PolygonEntity>& polygon)
{
    AddEntity(polygon, RenderLayer::POLYGONS);
    m_proxies.push_back(std::make_unique<PolygonProxy>(polygon));
}

void Editor::AddPath(const std::shared_ptr<editor::PathEntity>& path)
{
    AddEntity(path, RenderLayer::OBJECTS);
    m_proxies.push_back(std::make_unique<PathProxy>(path, this));
}

void Editor::AddPrefab(const std::shared_ptr<editor::Prefab>& prefab)
{
    AddEntity(prefab, RenderLayer::OBJECTS);
    m_proxies.push_back(std::make_unique<PrefabProxy>(prefab));
}

void Editor::SelectProxyObject(IObjectProxy* proxy_object)
{
    m_seleced_id = NO_SELECTION;
    m_context.proxy_object = proxy_object;
    m_object_detail_visualizer->SetObjectProxy(proxy_object);

    for(auto& proxy : m_proxies)
        proxy->SetSelected(false);

    if(proxy_object)
    {
        proxy_object->SetSelected(true);
        m_seleced_id = proxy_object->Id();
    }

    UpdateSnappers();
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

void Editor::UpdateSnappers()
{
    m_snap_points.clear();

    for(const auto& proxy : m_proxies)
    {
        const std::vector<SnapPoint>& snappers = proxy->GetSnappers();
        m_snap_points.insert(m_snap_points.end(), snappers.begin(), snappers.end());
    }
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

SnapPair Editor::FindSnapPosition(const math::Vector& position) const
{
    SnapPair snap_pair;
    if(!m_context.draw_snappers)
        return snap_pair;

    std::vector<SnapPoint> selected_snappers;

    const unsigned int id = m_seleced_id;
    const auto find_func = [id](const IObjectProxyPtr& proxy) {
        return id == proxy->Id();
    };

    auto it = std::find_if(m_proxies.begin(), m_proxies.end(), find_func);
    if(it != m_proxies.end())
        selected_snappers = (*it)->GetSnappers();

    float best_distance = math::INF;

    for(const SnapPoint& snap_point : selected_snappers)
    {
        for(const SnapPoint& other : m_snap_points)
        {
            if(snap_point.id == other.id)
                continue;

            const float distance = math::Length(snap_point.position - other.position);
            if(distance < 0.1f && distance < best_distance)
            {
                best_distance = distance;

                snap_pair.found_snap = true;
                snap_pair.snap_from = snap_point;
                snap_pair.snap_to = other;
            }
        }
    }

    return snap_pair;
}

void Editor::OnDeleteObject()
{
    if(m_seleced_id == NO_SELECTION)
        return;

    const unsigned int id = m_seleced_id;

    const auto find_func = [id](const IObjectProxyPtr& proxy) {
        return id == proxy->Id();
    };

    mono::remove_if(m_proxies, find_func);

    auto entity = FindEntityFromId(id);    
    const auto remove_entity_func = [this, entity] {
        RemoveEntity(entity);
    };

    SchedulePreFrameTask(remove_entity_func);

    SelectProxyObject(nullptr);
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

    SelectProxyObject(proxy.get());
    m_proxies.push_back(std::move(proxy));
}

bool Editor::DrawObjectNames() const
{
    return m_context.draw_object_names;
}

void Editor::EnableDrawObjectNames(bool enable)
{
    m_context.draw_object_names = enable;
}

bool Editor::DrawSnappers() const
{
    return m_context.draw_snappers;
}

void Editor::EnableDrawSnappers(bool enable)
{
    m_context.draw_snappers = enable;;
}

const mono::Color::RGBA& Editor::BackgroundColor() const
{
    return m_context.background_color;
}

void Editor::SetBackgroundColor(const mono::Color::RGBA& color)
{
    m_context.background_color = color;
    m_window->SetBackgroundColor(color.red, color.green, color.blue);
}

int Editor::ActivePanelIndex() const
{
    return m_context.active_panel_index;
}

void Editor::SetActivePanelIndex(int index)
{
    m_context.active_panel_index = index;
}

void Editor::DuplicateSelected()
{
    if(m_seleced_id == NO_SELECTION)
        return;

    const unsigned int id = m_seleced_id;
    const auto find_func = [id](const IObjectProxyPtr& proxy) {
        return id == proxy->Id();
    };

    auto it = std::find_if(m_proxies.begin(), m_proxies.end(), find_func);
    if(it != m_proxies.end())
    {
        const char* name = (*it)->Name();
        const std::vector<Attribute> attributes = (*it)->GetAttributes();
        const math::Vector position = (*it)->Entity()->Position();

        IObjectProxyPtr proxy = m_object_factory.CreateObject(name);
        if(proxy == nullptr)
        {
            std::printf("Unable to create object of type: '%s'\n", name);
            return;
        }

        proxy->SetAttributes(attributes);

        mono::IEntityPtr entity = proxy->Entity();
        entity->SetPosition(position + math::Vector(0.5f, 0.5f));
        
        AddEntity(entity, RenderLayer::OBJECTS);
        IObjectProxy* proxy_pointer = proxy.get();

        m_proxies.push_back(std::move(proxy));

        SelectProxyObject(proxy_pointer);
    }
}
