
#include "Editor.h"
#include "EditorConfig.h"
#include "Camera/ICamera.h"

#include "Rendering/Sprite/ISprite.h"
#include "Rendering/Sprite/ISpriteFactory.h"
#include "Rendering/Sprite/SpriteBatchDrawer.h"
#include "Rendering/ImGui.h"

#include "Events/EventFuncFwd.h"
#include "Events/SurfaceChangedEvent.h"
#include "EventHandler/EventHandler.h"

#include "SystemContext.h"
#include "TransformSystem/TransformSystem.h"

#include "UserInputController.h"
#include "UI/ImGuiInterfaceDrawer.h"
#include "ImGuiImpl/ImGuiInputHandler.h"
#include "Resources.h"
#include "RenderLayers.h"
#include "WorldSerializer.h"
#include "Component.h"

#include "ObjectProxies/PolygonProxy.h"
#include "ObjectProxies/PathProxy.h"

#include "ObjectProxies/ComponentProxy.h"
#include "Entity/IEntityManager.h"

#include "Objects/Polygon.h"
#include "Objects/Path.h"

#include "Visualizers/GridVisualizer.h"
#include "Visualizers/ScaleVisualizer.h"
#include "Visualizers/GrabberVisualizer.h"
#include "Visualizers/SnapperVisualizer.h"
#include "Visualizers/ObjectNameVisualizer.h"
#include "Visualizers/SelectionVisualizer.h"

#include "Visualizers/ComponentDrawFuncs.h"
#include "Visualizers/ComponentDetailVisualizer.h"

#include "Serializer/JsonSerializer.h"

#include "Util/Algorithm.h"
#include "Math/MathFunctions.h"
#include <algorithm>
#include <limits>

namespace
{
    void SetupIcons(editor::UIContext& context)
    {
        context.tools_texture_id = mono::LoadImGuiTexture("res/textures/placeholder.png");
        context.default_icon = math::Quad(0.0f, 0.0f, 1.0f, 1.0f);
    }

    void SetupComponents(editor::UIContext& context)
    {
        for (const Component& component : default_components)
            context.component_items.push_back({component.hash, ComponentNameFromHash(component.hash)});
    }

    class SyncPoint : public mono::IUpdatable
    {
    public:
        SyncPoint(IEntityManager& entity_manager)
            : m_entity_manager(entity_manager)
        { }

        void Update(const mono::UpdateContext& update_context) override
        {
            m_entity_manager.Sync();
        }

        IEntityManager& m_entity_manager;
    };
}

using namespace editor;

Editor::Editor(
    System::IWindow* window,
    IEntityManager& entity_manager,
    mono::EventHandler& event_handler,
    mono::SystemContext& system_context,
    Config& editor_config,
    const char* world_filename)
    : m_window(window)
    , m_entity_manager(entity_manager)
    , m_event_handler(event_handler)
    , m_system_context(system_context)
    , m_editor_config(editor_config)
    , m_world_filename(world_filename)
    , m_object_factory(this)
    , m_selected_id(NO_SELECTION)
    , m_preselected_id(NO_SELECTION)
{
    using namespace std::placeholders;

    m_context.all_proxy_objects = &m_proxies;

    m_context.context_menu_callback = std::bind(&Editor::OnContextMenu, this, _1);
    m_context.modal_selection_callback = std::bind(&Editor::SelectItemCallback, this, _1);

    m_context.delete_callback = std::bind(&Editor::OnDeleteObject, this);
    m_context.select_object_callback = std::bind(&Editor::SelectProxyObject, this, _1);
    m_context.preselect_object_callback = std::bind(&Editor::PreselectProxyObject, this, _1);
    m_context.teleport_to_object_callback = std::bind(&Editor::TeleportToProxyObject, this, _1);

    m_context.add_component = std::bind(&Editor::AddComponent, this, _1);
    m_context.delete_component = std::bind(&Editor::DeleteComponent, this, _1);

    m_context.editor_menu_callback = std::bind(&Editor::EditorMenuCallback, this, _1);
    m_context.tools_menu_callback = std::bind(&Editor::ToolsMenuCallback, this, _1);

    m_context.draw_object_names_callback = std::bind(&Editor::EnableDrawObjectNames, this, _1);
    m_context.draw_snappers_callback = std::bind(&Editor::EnableDrawSnappers, this, _1);
    m_context.background_color_callback = std::bind(&Editor::SetBackgroundColor, this, _1);

    SetupIcons(m_context);
    SetupComponents(m_context);

    editor::LoadAllSprites("res/sprites/all_sprite_files.json");
    editor::LoadAllEntities("res/entities/all_entities.json");
    editor::LoadAllPaths("res/paths/all_paths.json");
    editor::LoadAllTextures("res/textures/all_textures.json");
}

Editor::~Editor()
{ }

void Editor::OnLoad(mono::ICamera* camera)
{
    EnableDrawObjectNames(m_editor_config.draw_object_names);
    EnableDrawSnappers(m_editor_config.draw_snappers);
    EnableDrawOutline(m_editor_config.draw_outline);
    SetBackgroundColor(m_editor_config.background_color);
    camera->SetPosition(m_editor_config.camera_position);
    camera->SetViewport(m_editor_config.camera_viewport);

    m_camera = camera;

    mono::SetImGuiConfig("res/editor_imgui.ini");
    m_input_handler = std::make_unique<ImGuiInputHandler>(m_event_handler);

    mono::TransformSystem* transform_system = m_system_context.GetSystem<mono::TransformSystem>();
    m_user_input_controller =
        std::make_unique<editor::UserInputController>(camera, m_window, this, &m_context, m_event_handler);

    ComponentDrawMap draw_funcs;
    draw_funcs[CIRCLE_SHAPE_COMPONENT] = editor::DrawCircleShapeDetails;
    draw_funcs[BOX_SHAPE_COMPONENT] = editor::DrawBoxShapeDetails;
    draw_funcs[SEGMENT_SHAPE_COMPONENT] = editor::DrawSegmentShapeDetails;
    draw_funcs[SPAWN_POINT_COMPONENT] = editor::DrawSpawnPointDetails;
    draw_funcs[TRIGGER_COMPONENT] = editor::DrawTriggerComponentDetails;

    m_component_detail_visualizer = std::make_unique<ComponentDetailVisualizer>(draw_funcs, transform_system);

    AddUpdatable(new SyncPoint(m_entity_manager));
    AddUpdatable(new editor::ImGuiInterfaceDrawer(m_context));

    AddDrawable(new GridVisualizer(), RenderLayer::BACKGROUND);
    AddDrawable(new GrabberVisualizer(m_grabbers), RenderLayer::GRABBERS);
    AddDrawable(new SnapperVisualizer(m_context.draw_snappers, m_snap_points), RenderLayer::GRABBERS);
    AddDrawable(new ScaleVisualizer(), RenderLayer::UI);
    AddDrawable(new SelectionVisualizer(m_selected_id, m_preselected_id, transform_system), RenderLayer::UI);
    AddDrawable(new ObjectNameVisualizer(m_context.draw_object_names, m_proxies), RenderLayer::UI);
    AddDrawable(m_component_detail_visualizer.get(), RenderLayer::UI);
    AddDrawable(new mono::SpriteBatchDrawer(&m_system_context), RenderLayer::OBJECTS);

    m_proxies = LoadWorld(m_world_filename, m_object_factory, &m_entity_manager, transform_system);
    for(IObjectProxyPtr& proxy : m_proxies)
    {
        auto entity = proxy->Entity();
        if(entity)
            AddEntity(entity, RenderLayer::OBJECTS);
    }

    UpdateSnappers();
}

int Editor::OnUnload()
{
    RemoveDrawable(m_component_detail_visualizer.get());

    for(IObjectProxyPtr& proxy : m_proxies)
    {
        auto entity = proxy->Entity();
        if(entity)
            RemoveEntity(entity);
    }

    m_editor_config.camera_position = m_camera->GetPosition();
    m_editor_config.camera_viewport = m_camera->GetViewport();
    m_editor_config.draw_object_names = DrawObjectNames();
    m_editor_config.draw_snappers = DrawSnappers();
    m_editor_config.draw_outline = DrawOutline();
    m_editor_config.background_color = BackgroundColor();

    Save();
    return 0;
}

void Editor::Save()
{
    SaveWorld(m_world_filename, m_proxies);
    m_context.notifications.emplace_back(m_context.default_icon, "Saved...", 2000);
}

void Editor::ExportEntity()
{
    IObjectProxy* proxy = FindProxyObject(m_selected_id);
    if(!proxy)
        return;

    std::string filename = "res/entities/" + std::string(proxy->Name()) + ".entity";
    std::replace(filename.begin(), filename.end(), ' ', '_');

    editor::JsonSerializer serializer;
    proxy->Visit(serializer);
    serializer.WriteComponentEntities(filename);

    editor::AddNewEntity(filename.c_str());
    m_context.notifications.emplace_back(m_context.default_icon, "Exported entity...", 2000);
}

void Editor::ImportEntity()
{
    m_context.modal_items = GetAllEntities();
    m_context.show_modal_item_selection = true;
}

void Editor::NewEntity()
{
    mono::TransformSystem* transform_system = m_system_context.GetSystem<mono::TransformSystem>();
    mono::Entity new_entity = m_entity_manager.CreateEntity("Unnamed", {});

    const std::vector<Component> components = {
        DefaultComponentFromHash(TRANSFORM_COMPONENT)
    };

    auto proxy = std::make_unique<ComponentProxy>(
        new_entity.id, "unnamed", "", components, &m_entity_manager, transform_system);
    proxy->SetPosition(m_camera->GetPosition());

    m_proxies.push_back(std::move(proxy));
    SelectProxyObject(m_proxies.back().get());
}

void Editor::AddPolygon(std::unique_ptr<editor::PolygonEntity> polygon)
{
    AddEntity(polygon.get(), RenderLayer::POLYGONS);
    m_proxies.push_back(std::make_unique<PolygonProxy>("unnamed", std::move(polygon)));
}

void Editor::AddPath(std::unique_ptr<editor::PathEntity> path)
{
    AddEntity(path.get(), RenderLayer::OBJECTS);
    m_proxies.push_back(std::make_unique<PathProxy>(std::move(path), this));
}

void Editor::SelectProxyObject(IObjectProxy* proxy_object)
{
    m_selected_id = NO_SELECTION;
    m_context.selected_proxy_object = proxy_object;
    m_component_detail_visualizer->SetObjectProxy(proxy_object);

    for(auto& proxy : m_proxies)
        proxy->SetSelected(false);

    if(proxy_object)
    {
        m_selected_id = proxy_object->Id();
        proxy_object->SetSelected(true);
    }

    UpdateSnappers();
    UpdateGrabbers();
}

void Editor::PreselectProxyObject(IObjectProxy* proxy_object)
{
    if(proxy_object)
    {
        proxy_object->SetSelected(true);
        m_preselected_id = proxy_object->Id();
    }
    else
    {
        m_preselected_id = NO_SELECTION;
    }
    
    m_context.preselected_proxy_object = proxy_object;
}

void Editor::TeleportToProxyObject(IObjectProxy* proxy_object)
{
    const math::Vector& proxy_position = proxy_object->GetPosition();
    m_camera->SetPosition(proxy_position);

    const math::Quad& proxy_bounding_box = proxy_object->GetBoundingBox();
    const float length_squared =
        math::Length(math::TopLeft(proxy_bounding_box) - math::BottomRight(proxy_bounding_box)) * 8;

    const math::Quad viewport = m_camera->GetViewport();
    const float ratio = math::Width(viewport) * math::Height(viewport);
    const float height = length_squared * ratio;
    m_camera->SetTargetViewport(math::Quad(0, 0, length_squared, height));
}

void Editor::TeleportToSelectedProxyObject()
{
    IObjectProxy* proxy = FindProxyObject(m_selected_id);
    if(proxy)
        TeleportToProxyObject(proxy);
}

IObjectProxy* Editor::FindProxyObject(const math::Vector& position)
{
    std::vector<IObjectProxy*> found_proxies;
    for(const auto& proxy : m_proxies)
    {
        const bool intersects = proxy->Intersects(position);
        if(intersects)
            found_proxies.push_back(proxy.get());
    }

    if(found_proxies.empty())
        return nullptr;

    const auto sort_on_y = [](const IObjectProxy* first, const IObjectProxy* second) {
        const math::Vector& first_position = first->GetPosition();
        const math::Vector& second_position = second->GetPosition();

        return first_position.y > second_position.y;
    };

    std::sort(found_proxies.begin(), found_proxies.end(), sort_on_y);
    return found_proxies.back();
}

IObjectProxy* Editor::FindProxyObject(uint32_t proxy_id) const
{
    if(proxy_id == NO_SELECTION)
        return nullptr;

    const auto find_func = [proxy_id](const IObjectProxyPtr& proxy) {
        return proxy_id == proxy->Id();
    };

    auto it = std::find_if(m_proxies.begin(), m_proxies.end(), find_func);
    if(it != m_proxies.end())
        return it->get();

    return nullptr;
}

uint32_t Editor::GetSelectedObjectId() const
{
    return m_selected_id;
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

    const IObjectProxy* proxy_object = FindProxyObject(m_selected_id);
    if(proxy_object)
        m_grabbers = proxy_object->GetGrabbers();
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

    const IObjectProxy *proxy_object = FindProxyObject(m_selected_id);
    if(proxy_object)
        selected_snappers = proxy_object->GetSnappers();

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
    if(m_selected_id == NO_SELECTION)
        return;

    IObjectProxy* proxy = FindProxyObject(m_selected_id);
    mono::IEntity* entity = proxy->Entity();
    if(entity)
        RemoveEntity(entity);

    const uint32_t id = m_selected_id;
    const auto find_func = [id](const IObjectProxyPtr& proxy) {
        return id == proxy->Id();
    };

    mono::remove_if(m_proxies, find_func);
    SelectProxyObject(nullptr);
    m_grabbers.clear();
}

void Editor::AddComponent(uint32_t component_hash)
{
    IObjectProxy* proxy_object = FindProxyObject(m_selected_id);
    if(proxy_object)
    {
        m_entity_manager.AddComponent(m_selected_id, component_hash);

        Component new_component = DefaultComponentFromHash(component_hash);
        std::vector<Component>& components = proxy_object->GetComponents();

        if(new_component.depends_on != NULL_COMPONENT)
        {
            const Component* found_dependency = FindComponentFromHash(new_component.depends_on, components);
            if(!found_dependency)
                components.push_back(DefaultComponentFromHash(new_component.depends_on));
        }

        components.push_back(new_component);
    }
}

void Editor::DeleteComponent(uint32_t index)
{
    IObjectProxy* proxy_object = FindProxyObject(m_selected_id);
    if(proxy_object)
    {
        std::vector<Component>& components = proxy_object->GetComponents();
        m_entity_manager.RemoveComponent(m_selected_id, components[index].hash);
        components.erase(components.begin() + index);
    }
}

void Editor::EntityComponentUpdated(uint32_t entity_id, uint32_t component_hash)
{
    IObjectProxy* proxy_object = FindProxyObject(m_selected_id);
    if(proxy_object)
    {
        for(Component& component : proxy_object->GetComponents())
        {
            if(component.hash == component_hash)
            {
                auto data = m_entity_manager.GetComponentData(entity_id, component_hash);
                component.properties = data;
            }
        }
    }
}

void Editor::OnContextMenu(int index)
{
    m_user_input_controller->HandleContextMenu(index);
}

void Editor::SelectItemCallback(int index)
{
    mono::TransformSystem* transform_system = m_system_context.GetSystem<mono::TransformSystem>();

    const std::string& selected_item = m_context.modal_items[index];
    std::vector<IObjectProxyPtr> loaded_objects = LoadComponentObjects(selected_item.c_str(), &m_entity_manager, transform_system);
    if(loaded_objects.empty())
        return;

    for(auto& object : loaded_objects)
        m_proxies.push_back(std::move(object));

    SelectProxyObject(m_proxies.back().get());
    TeleportToProxyObject(m_proxies.back().get());
    m_context.notifications.emplace_back(m_context.default_icon, "Imported Entity...", 2000);
}

void Editor::EditorMenuCallback(EditorMenuOptions option)
{
    if(option == EditorMenuOptions::NEW)
        NewEntity();
    else if(option == EditorMenuOptions::SAVE)
        Save();
    else if(option == EditorMenuOptions::IMPORT_ENTITY)
        ImportEntity();
    else if(option == EditorMenuOptions::EXPORT_ENTITY)
        ExportEntity();
    else if(option == EditorMenuOptions::DUPLICATE)
        DuplicateSelected();
}

void Editor::ToolsMenuCallback(ToolsMenuOptions option)
{
    m_user_input_controller->SelectTool(option);
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
    m_context.draw_snappers = enable;
}

bool Editor::DrawOutline() const
{
    return m_context.draw_outline;
}

void Editor::EnableDrawOutline(bool enable)
{
    m_context.draw_outline = enable;
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

void Editor::DuplicateSelected()
{
    IObjectProxy* proxy_object = FindProxyObject(m_selected_id);
    if(!proxy_object)
        return;

    IObjectProxyPtr cloned_proxy = proxy_object->Clone();
    if(cloned_proxy)
    {
        const uint32_t cloned_entity_id = cloned_proxy->Id();
        
        mono::TransformSystem* transform_system = m_system_context.GetSystem<mono::TransformSystem>();
        math::Matrix& transform = transform_system->GetTransform(cloned_entity_id);
        math::Translate(transform, math::Vector(1.0f, 1.0f));

        IObjectProxy* proxy_pointer = cloned_proxy.get();
        m_proxies.push_back(std::move(cloned_proxy));
        SelectProxyObject(proxy_pointer);
    }
}
