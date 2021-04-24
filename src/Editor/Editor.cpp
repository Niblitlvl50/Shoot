
#include "Editor.h"
#include "EditorConfig.h"
#include "Camera/ICamera.h"

#include "Rendering/RenderSystem.h"
#include "Rendering/IRenderer.h"
#include "Rendering/Sprite/ISprite.h"
#include "Rendering/Sprite/ISpriteFactory.h"
#include "Rendering/Sprite/SpriteBatchDrawer.h"
#include "Rendering/Sprite/SpriteSystem.h"
#include "Rendering/Text/TextSystem.h"
#include "Rendering/Text/TextBatchDrawer.h"
#include "Rendering/Objects/StaticBackground.h"

#include "RoadSystem/RoadSystem.h"
#include "RoadSystem/RoadBatchDrawer.h"

#include "Events/EventFuncFwd.h"
#include "Events/SurfaceChangedEvent.h"
#include "EventHandler/EventHandler.h"

#include "SystemContext.h"
#include "TransformSystem/TransformSystem.h"
#include "Paths/PathSystem.h"
#include "Paths/PathBatchDrawer.h"

#include "UserInputController.h"
#include "UI/ImGuiInterfaceDrawer.h"
#include "ImGuiImpl/ImGuiInputHandler.h"
#include "Resources.h"
#include "RenderLayers.h"
#include "Component.h"

#include "ObjectProxies/PathProxy.h"

#include "ObjectProxies/ComponentProxy.h"
#include "EntitySystem/IEntityManager.h"
#include "EntitySystem/EntitySystem.h"

#include "Visualizers/GridVisualizer.h"
#include "Visualizers/GameCameraVisualizer.h"
#include "Visualizers/GrabberVisualizer.h"
#include "Visualizers/SnapperVisualizer.h"
#include "Visualizers/ObjectNameVisualizer.h"
#include "Visualizers/SelectionVisualizer.h"

#include "Visualizers/ComponentDrawFuncs.h"
#include "Visualizers/ComponentDetailVisualizer.h"

#include "Serializer/JsonSerializer.h"
#include "Serializer/WorldSerializer.h"

#include "Util/Algorithm.h"
#include "Util/FpsCounter.h"
#include "Math/MathFunctions.h"

#include <algorithm>
#include <limits>

namespace
{
    void SetupIcons(editor::UIContext& context)
    {
        context.ui_icons[editor::placeholder_texture] = {
            mono::GetTextureFactory()->CreateTexture(editor::placeholder_texture),
            math::Vector(0.0f, 1.0f),
            math::Vector(1.0f, 0.0f),
            math::Vector(1.0f, 1.0f),
            "ui"
        };

        context.ui_icons[editor::export_texture] = {
            mono::GetTextureFactory()->CreateTexture(editor::export_texture),
            math::Vector(0.0f, 1.0f),
            math::Vector(1.0f, 0.0f),
            math::Vector(1.0f, 1.0f),
            "ui"
        };

        context.ui_icons[editor::import_texture] = {
            mono::GetTextureFactory()->CreateTexture(editor::import_texture),
            math::Vector(0.0f, 1.0f),
            math::Vector(1.0f, 0.0f),
            math::Vector(1.0f, 1.0f),
            "ui"
        };

        context.ui_icons[editor::information_texture] = {
            mono::GetTextureFactory()->CreateTexture(editor::information_texture),
            math::Vector(0.0f, 1.0f),
            math::Vector(1.0f, 0.0f),
            math::Vector(1.0f, 1.0f),
            "ui"
        };

        context.ui_icons[editor::save_texture] = {
            mono::GetTextureFactory()->CreateTexture(editor::save_texture),
            math::Vector(0.0f, 1.0f),
            math::Vector(1.0f, 0.0f),
            math::Vector(1.0f, 1.0f),
            "ui"
        };

        context.ui_icons[editor::wrench_texture] = {
            mono::GetTextureFactory()->CreateTexture(editor::wrench_texture),
            math::Vector(0.0f, 1.0f),
            math::Vector(1.0f, 0.0f),
            math::Vector(1.0f, 1.0f),
            "ui"
        };
    }

    void SetupSpriteIcons(const std::vector<std::string>& sprite_files, editor::UIContext& context)
    {
        for(const std::string& sprite_file : sprite_files)
        {
            const std::string full_sprite_path = "res/sprites/" + sprite_file;
            const mono::SpriteData* sprite_data =
                mono::GetSpriteFactory()->GetSpriteDataForFile(full_sprite_path.c_str());

            context.ui_icons[sprite_file] = {
                mono::GetTextureFactory()->CreateTexture(sprite_data->texture_file.c_str()),
                sprite_data->frames.front().uv_upper_left,
                sprite_data->frames.front().uv_lower_right,
                sprite_data->frames.front().size,
                sprite_data->category
            };
        }
    }

    void SetupComponents(editor::UIContext& context)
    {
        for(const Component& component : default_components)
            context.component_items.push_back({component.hash, component.allow_multiple, ComponentNameFromHash(component.hash), component.category});
    }

    class SyncPoint : public mono::IUpdatable
    {
    public:
        SyncPoint(mono::IEntityManager& entity_manager, editor::UIContext& ui_context)
            : m_entity_manager(entity_manager)
            , m_ui_context(ui_context)
        { }

        void Update(const mono::UpdateContext& update_context) override
        {
            m_entity_manager.Sync();
            m_fps++;

            m_ui_context.fps = m_fps.Fps();
        }

        mono::IEntityManager& m_entity_manager;
        editor::UIContext& m_ui_context;
        mono::FpsCounter m_fps;
    };
}

using namespace editor;

Editor::Editor(
    System::IWindow* window,
    mono::IEntityManager& entity_manager,
    mono::EventHandler& event_handler,
    mono::SystemContext& system_context,
    Config& editor_config)
    : m_window(window)
    , m_entity_manager(entity_manager)
    , m_event_handler(event_handler)
    , m_system_context(system_context)
    , m_editor_config(editor_config)
    , m_selected_id(NO_SELECTION)
    , m_preselected_id(NO_SELECTION)
    , m_pick_target(nullptr)
{
    m_mode_stack.push(EditorMode::DEFAULT);

    using namespace std::placeholders;

    m_context.all_proxy_objects = &m_proxies;

    m_context.context_menu_callback = std::bind(&Editor::OnContextMenu, this, _1);
    m_context.modal_selection_callback = std::bind(&Editor::SelectItemCallback, this, _1);
    m_context.pick_callback = [this](uint32_t* target_data){
        SetPickingTarget(target_data);
        EnterMode(EditorMode::REFERENCE_PICKING);
    };

    m_context.delete_callback = std::bind(&Editor::OnDeleteObject, this);
    m_context.switch_world = std::bind(&Editor::SwitchWorld, this, _1);
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
    m_context.background_texture_callback = std::bind(&Editor::SetBackgroundTexture, this, _1);

    m_context.entity_name_callback = [&entity_manager](uint32_t entity_id){
        return entity_manager.GetEntityName(entity_id);
    };

    editor::LoadAllSprites("res/sprites/all_sprite_files.json");
    editor::LoadAllEntities("res/entities/all_entities.json");
    editor::LoadAllPaths("res/paths/all_paths.json");
    editor::LoadAllTextures("res/textures/all_textures.json");
    editor::LoadAllWorlds("res/worlds/all_worlds.json");

    SetupIcons(m_context);
    SetupSpriteIcons(editor::GetAllSprites(), m_context);
    SetupComponents(m_context);

    m_context.all_worlds = editor::GetAllWorlds();
}

Editor::~Editor()
{ }

void Editor::OnLoad(mono::ICamera* camera, mono::IRenderer* renderer)
{
    m_renderer = renderer;

    EnableDrawObjectNames(m_editor_config.draw_object_names);
    EnableDrawSnappers(m_editor_config.draw_snappers);
    EnableDrawOutline(m_editor_config.draw_outline);
    EnableDrawLevelMetadata(m_editor_config.draw_metadata);
    SetBackgroundColor(m_editor_config.background_color);
    EnableSnapToGrid(m_editor_config.snap_to_grid);
    SwitchWorld(m_editor_config.selected_world);
    
    camera->SetPosition(m_editor_config.camera_position);
    camera->SetViewportSize(m_editor_config.camera_viewport);

    m_context.grid_size = m_editor_config.grid_size;

    m_camera = camera;

    m_input_handler = std::make_unique<ImGuiInputHandler>(m_event_handler);

    mono::TransformSystem* transform_system = m_system_context.GetSystem<mono::TransformSystem>();
    mono::TextSystem* text_system = m_system_context.GetSystem<mono::TextSystem>();
    mono::SpriteSystem* sprite_system = m_system_context.GetSystem<mono::SpriteSystem>();
    m_user_input_controller =
        std::make_unique<editor::UserInputController>(camera, m_window, this, &m_context, m_event_handler);
    mono::PathSystem* path_system = m_system_context.GetSystem<mono::PathSystem>();
    mono::RoadSystem* road_system = m_system_context.GetSystem<mono::RoadSystem>();

    ComponentDrawMap draw_funcs;
    draw_funcs[CIRCLE_SHAPE_COMPONENT] = editor::DrawCircleShapeDetails;
    draw_funcs[BOX_SHAPE_COMPONENT] = editor::DrawBoxShapeDetails;
    draw_funcs[SEGMENT_SHAPE_COMPONENT] = editor::DrawSegmentShapeDetails;
    draw_funcs[POLYGON_SHAPE_COMPONENT] = editor::DrawPolygonShapeDetails;
    draw_funcs[SPAWN_POINT_COMPONENT] = editor::DrawSpawnPointDetails;
    draw_funcs[SHAPE_TRIGGER_COMPONENT] = editor::DrawShapeTriggerComponentDetails;
    draw_funcs[AREA_TRIGGER_COMPONENT] = editor::DrawAreaTriggerComponentDetails;
    draw_funcs[DEATH_TRIGGER_COMPONENT] = editor::DrawDeathTriggerComponentDetails;
    draw_funcs[TIME_TRIGGER_COMPONENT] = editor::DrawTimeTriggerComponentDetails;
    draw_funcs[COUNTER_TRIGGER_COMPONENT] = editor::DrawCounterTriggerComponentDetails;
    draw_funcs[TRANSLATION_COMPONENT] = editor::DrawSetTranslationDetails;
    draw_funcs[ROTATION_COMPONENT] = editor::DrawSetRotationDetails;
    draw_funcs[PATH_COMPONENT] = editor::DrawPath;

    m_component_detail_visualizer = std::make_unique<ComponentDetailVisualizer>(draw_funcs, transform_system);
    m_static_background = std::make_unique<mono::StaticBackground>();

    AddUpdatable(new SyncPoint(m_entity_manager, m_context));

    AddDrawable(m_static_background.get(), RenderLayer::BACKGROUND);
    AddDrawable(new GridVisualizer(m_context.draw_grid), RenderLayer::BACKGROUND);
    AddDrawable(new GrabberVisualizer(m_grabbers), RenderLayer::GRABBERS);
    AddDrawable(new SnapperVisualizer(m_context.draw_snappers, m_snap_points), RenderLayer::GRABBERS);
    AddDrawable(new SelectionVisualizer(m_selected_id, m_preselected_id, transform_system), RenderLayer::UI);
    AddDrawable(new ObjectNameVisualizer(m_context.draw_object_names, m_proxies), RenderLayer::UI);
    AddDrawable(m_component_detail_visualizer.get(), RenderLayer::UI);
    AddDrawable(
        new GameCameraVisualizer(
            m_context.draw_level_metadata,
            m_context.player_spawn_point,
            m_context.camera_position,
            m_context.camera_size),
        RenderLayer::UI);
    AddDrawable(new mono::SpriteBatchDrawer(transform_system, sprite_system), RenderLayer::OBJECTS);
    AddDrawable(new mono::TextBatchDrawer(text_system, transform_system), RenderLayer::OBJECTS);
    AddDrawable(new mono::PathBatchDrawer(path_system, transform_system), RenderLayer::OBJECTS);
    AddDrawable(new mono::RoadBatchDrawer(road_system, path_system, transform_system), RenderLayer::OBJECTS);
    AddDrawable(new editor::ImGuiInterfaceDrawer(m_context), RenderLayer::UI);
}

int Editor::OnUnload()
{
    RemoveDrawable(m_component_detail_visualizer.get());
    RemoveDrawable(m_static_background.get());

    m_editor_config.camera_position = m_camera->GetPosition();
    m_editor_config.camera_viewport = m_camera->GetViewportSize();
    m_editor_config.draw_object_names = DrawObjectNames();
    m_editor_config.draw_snappers = DrawSnappers();
    m_editor_config.draw_outline = DrawOutline();
    m_editor_config.draw_metadata = DrawLevelMetadata();
    m_editor_config.background_color = BackgroundColor();
    m_editor_config.snap_to_grid = SnapToGrid();
    m_editor_config.grid_size = m_context.grid_size;
    m_editor_config.selected_world = m_world_filename;

    Save();
    return 0;
}

void Editor::PostUpdate()
{
    if(m_world_filename != m_new_world_filename)
        LoadWorld(m_new_world_filename);
}

void Editor::SwitchWorld(const std::string& new_world_filename)
{
    m_new_world_filename = new_world_filename;
}

void Editor::LoadWorld(const std::string& world_filename)
{
    SelectProxyObject(nullptr);
    PreselectProxyObject(nullptr);

    if(!m_proxies.empty())
    {
        Save();
        m_proxies.clear();
    }

    m_entity_manager.Sync();

    mono::TransformSystem* transform_system = m_system_context.GetSystem<mono::TransformSystem>();
    editor::World world = ::LoadWorld(world_filename.c_str(), &m_entity_manager, transform_system, this);
    m_proxies = std::move(world.loaded_proxies);

    m_context.camera_position = world.leveldata.metadata.camera_position;
    m_context.camera_size = world.leveldata.metadata.camera_size;
    m_context.player_spawn_point = world.leveldata.metadata.player_spawn_point;
    m_context.background_texture = world.leveldata.metadata.background_texture;

    m_world_filename = world_filename;
    m_context.selected_world = world_filename;

    if(!world.leveldata.metadata.background_texture.empty())
        SetBackgroundTexture(world.leveldata.metadata.background_texture);
}

void Editor::Save()
{
    shared::LevelMetadata metadata;
    metadata.camera_position = m_context.camera_position;
    metadata.camera_size = m_context.camera_size;
    metadata.player_spawn_point = m_context.player_spawn_point;
    metadata.background_texture = m_context.background_texture;

    SaveWorld(m_world_filename.c_str(), m_proxies, metadata);
    m_context.notifications.emplace_back(save_texture, "Saved...", 2000);
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

    shared::LevelMetadata metadata;
    serializer.WriteComponentEntities(filename, metadata);

    editor::AddNewEntity(filename.c_str());
    m_context.notifications.emplace_back(export_texture, "Exported entity...", 2000);
}

void Editor::ImportEntity()
{
    m_context.modal_items = GetAllEntities();
    m_context.show_modal_item_selection = true;
}

void Editor::NewEntity()
{
    mono::TransformSystem* transform_system = m_system_context.GetSystem<mono::TransformSystem>();
    mono::Entity new_entity = m_entity_manager.CreateEntity("unnamed", { TRANSFORM_COMPONENT });

    const std::vector<Component> components = {
        DefaultComponentFromHash(TRANSFORM_COMPONENT)
    };

    auto proxy = std::make_unique<ComponentProxy>(
        new_entity.id, "unnamed", "", components, &m_entity_manager, transform_system, this);
    proxy->SetPosition(m_camera->GetPosition());

    m_proxies.push_back(std::move(proxy));
    SelectProxyObject(m_proxies.back().get());
}

void Editor::AddPath(const std::vector<math::Vector>& path_points)
{
    // Make points local
    const math::Vector position = path_points.front();

    std::vector<math::Vector> local_points = path_points;
    for(math::Vector& point : local_points)
        point -= position;

    mono::TransformSystem* transform_system = m_system_context.GetSystem<mono::TransformSystem>();
    mono::Entity new_entity = m_entity_manager.CreateEntity("unnamed", { TRANSFORM_COMPONENT, PATH_COMPONENT });

    std::vector<Component> components = {
        DefaultComponentFromHash(TRANSFORM_COMPONENT),
        DefaultComponentFromHash(PATH_COMPONENT),
    };

    SetAttribute(POSITION_ATTRIBUTE, components.front().properties, position);
    SetAttribute(PATH_POINTS_ATTRIBUTE, components.back().properties, local_points);

    auto proxy = std::make_unique<PathProxy>(
        new_entity.id, "unnamed", "", components, &m_entity_manager, transform_system, this);
    proxy->SetPosition(position);

    m_proxies.push_back(std::move(proxy));
    SelectProxyObject(m_proxies.back().get());
}

void Editor::SelectProxyObject(IObjectProxy* proxy_object)
{
    const EditorMode mode = m_mode_stack.top();
    if(mode == EditorMode::REFERENCE_PICKING)
    {
        if(proxy_object)
        {
            const uint32_t selected_id = proxy_object->Id();
            *m_pick_target = selected_id;
            m_pick_target = nullptr;
            m_mode_stack.pop();
        }

        return;
    }

    m_selected_id = NO_SELECTION;
    m_context.selected_proxy_object = proxy_object;

    for(auto& proxy : m_proxies)
        proxy->SetSelected(false);

    std::vector<IObjectProxy*> proxies_to_draw;
    if(proxy_object)
    {
        m_selected_id = proxy_object->Id();
        proxy_object->SetSelected(true);
        proxies_to_draw.push_back(proxy_object);
    }

    m_component_detail_visualizer->SetObjectProxies(proxies_to_draw);

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
    m_camera->SetTargetPosition(proxy_position);

    const math::Quad& proxy_bounding_box = proxy_object->GetBoundingBox();
    const float length_squared =
        math::Length(math::TopLeft(proxy_bounding_box) - math::BottomRight(proxy_bounding_box)) * 8;

    const math::Quad viewport = m_camera->GetViewport();
    const float ratio = math::Width(viewport) * math::Height(viewport);
    const float height = length_squared * ratio;
    m_camera->SetTargetViewportSize(math::Vector(length_squared, height));
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

    mono::EntitySystem* entity_system = m_system_context.GetSystem<mono::EntitySystem>();
    const mono::SpriteSystem* sprite_system = m_system_context.GetSystem<mono::SpriteSystem>();

    const auto sort_on_y = [entity_system, sprite_system](const IObjectProxy* first, const IObjectProxy* second) {

        const mono::Entity* first_entity = entity_system->GetEntity(first->Id());
        const mono::Entity* second_entity = entity_system->GetEntity(second->Id());

        if(first_entity && second_entity)
        {
            const bool first_has_sprite = entity_system->HasComponent(first_entity, SPRITE_COMPONENT);
            const bool second_has_sprite = entity_system->HasComponent(second_entity, SPRITE_COMPONENT);
            if(first_has_sprite && second_has_sprite)
            {
                const int first_sprite_layer = sprite_system->GetSpriteLayer(first_entity->id);
                const int second_sprite_layer = sprite_system->GetSpriteLayer(second_entity->id);
                if(first_sprite_layer != second_sprite_layer)
                    return first_sprite_layer < second_sprite_layer;
            }
        }

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

    IObjectProxy* proxy_object = FindProxyObject(m_selected_id);
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

    const uint32_t id = m_selected_id;
    const auto find_func = [id](const IObjectProxyPtr& proxy) {
        return id == proxy->Id();
    };

    mono::remove_if(m_proxies, find_func);
    SelectProxyObject(nullptr);
    PreselectProxyObject(nullptr);
    m_grabbers.clear();
}

void Editor::AddComponent(uint32_t component_hash)
{
    IObjectProxy* proxy_object = FindProxyObject(m_selected_id);
    if(proxy_object)
    {
        std::vector<Component>& components = proxy_object->GetComponents();
        const uint32_t num_components = components.size();
        const uint32_t num_added = shared::AddComponent(component_hash, components);

        for(size_t index = num_components; index < num_components + num_added; ++index)
            m_entity_manager.AddComponent(m_selected_id, components[index].hash);

        shared::SortComponentsByPriority(components);
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

void Editor::AddComponentUI()
{
    m_context.open_add_component = true;
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
    std::vector<IObjectProxyPtr> loaded_objects = LoadComponentObjects(selected_item.c_str(), &m_entity_manager, transform_system, this);
    if(loaded_objects.empty())
        return;

    for(auto& object : loaded_objects)
        m_proxies.push_back(std::move(object));

    SelectProxyObject(m_proxies.back().get());
    TeleportToProxyObject(m_proxies.back().get());
    m_context.notifications.emplace_back(import_texture, "Imported Entity...", 2000);
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
    else if(option == EditorMenuOptions::REEXPORTENTITIES)
        ReExportEntities();
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

bool Editor::DrawLevelMetadata() const
{
    return m_context.draw_level_metadata;
}

void Editor::EnableDrawLevelMetadata(bool enable)
{
    m_context.draw_level_metadata = enable;
}

const mono::Color::RGBA& Editor::BackgroundColor() const
{
    return m_context.background_color;
}

void Editor::SetBackgroundColor(const mono::Color::RGBA& color)
{
    m_context.background_color = color;
    m_renderer->SetClearColor(color);
}

void Editor::SetBackgroundTexture(const std::string& background_texture)
{
    m_static_background->Load(background_texture.c_str(), mono::TextureModeFlags::REPEAT);
}

bool Editor::DrawGrid() const
{
    return m_context.draw_grid;
}

void Editor::EnableDrawGrid(bool enable)
{
    m_context.draw_grid = enable;
}

bool Editor::SnapToGrid() const
{
    return m_context.snap_to_grid;
}

void Editor::EnableSnapToGrid(bool enable)
{
    m_context.snap_to_grid = enable;

    char text_buffer[128];
    std::snprintf(text_buffer, std::size(text_buffer), "Snap to Grid: %s", enable ? "Enabled" : "Disabled");
    m_context.notifications.push_back({information_texture, text_buffer, 1000});
}

math::Vector Editor::GridSize() const
{
    return m_context.grid_size;
}

void Editor::EnableDrawAllObjects(bool enable)
{
    m_context.draw_all_objects = enable;

    std::vector<IObjectProxy*> proxies;
    
    if(m_context.draw_all_objects)
    {
        for(IObjectProxyPtr& proxy_ptr : m_proxies)
            proxies.push_back(proxy_ptr.get());
    }
    
    m_component_detail_visualizer->SetObjectProxies(proxies);
}

bool Editor::DrawAllObjects() const
{
    return m_context.draw_all_objects;
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

void Editor::ReExportEntities()
{
    mono::TransformSystem* transform_system = m_system_context.GetSystem<mono::TransformSystem>();

    const std::vector<std::string>& all_entities = GetAllEntities();

    for(auto entity_file : all_entities)
    {
        std::vector<IObjectProxyPtr> loaded_objects =
            LoadComponentObjects(entity_file.c_str(), &m_entity_manager, transform_system, this);

        for(IObjectProxyPtr& proxy : loaded_objects)
        {
            std::string filename = "res/entities/" + std::string(proxy->Name()) + ".entity";
            std::replace(filename.begin(), filename.end(), ' ', '_');

            editor::JsonSerializer serializer;
            proxy->Visit(serializer);

            shared::LevelMetadata metadata;
            serializer.WriteComponentEntities(filename, metadata);
        }
    }
}

void Editor::SetPickingTarget(uint32_t* target_data)
{
    m_pick_target = target_data;
}

void Editor::EnterMode(EditorMode new_mode)
{
    m_mode_stack.push(new_mode);

    const char* mode_text = nullptr;
    if(new_mode == EditorMode::DEFAULT)
        mode_text = "Default";
    else if(new_mode == EditorMode::REFERENCE_PICKING)
        mode_text = "Reference Picking";

    char text_buffer[128] = {};
    std::snprintf(text_buffer, std::size(text_buffer), "%s", mode_text);
    m_context.notifications.push_back({information_texture, text_buffer, 5000});
}

void Editor::PopMode()
{
    assert(m_mode_stack.size() > 1);
    m_mode_stack.pop();
}
