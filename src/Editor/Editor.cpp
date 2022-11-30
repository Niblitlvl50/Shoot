
#include "Editor.h"
#include "EditorConfig.h"
#include "Camera/ICamera.h"

#include "Rendering/RenderSystem.h"
#include "Rendering/IRenderer.h"
#include "Rendering/Lights/LightSystem.h"
#include "Rendering/Lights/LightSystemDrawer.h"
#include "Rendering/Sprite/ISprite.h"
#include "Rendering/Sprite/ISpriteFactory.h"
#include "Rendering/Sprite/SpriteBatchDrawer.h"
#include "Rendering/Sprite/SpriteSystem.h"
#include "Rendering/Text/TextSystem.h"
#include "Rendering/Text/TextBatchDrawer.h"
#include "Rendering/Objects/StaticBackground.h"

#include "RoadSystem/RoadSystem.h"
#include "RoadSystem/RoadBatchDrawer.h"
#include "Particle/ParticleSystem.h"
#include "Particle/ParticleSystemDrawer.h"

#include "Events/EventFuncFwd.h"
#include "Events/SurfaceChangedEvent.h"
#include "EventHandler/EventHandler.h"

#include "SystemContext.h"
#include "TransformSystem/TransformSystem.h"
#include "Paths/PathSystem.h"
#include "Paths/PathBatchDrawer.h"
#include "World/WorldBoundsDrawer.h"
#include "World/WorldBoundsSystem.h"
#include "UI/UISystem.h"
#include "UI/UISystemDrawer.h"

#include "UserInputController.h"
#include "UI/ImGuiInterfaceDrawer.h"
#include "UI/UIDecorators.h"
#include "ImGuiImpl/ImGuiInputHandler.h"
#include "Resources.h"
#include "RenderLayers.h"
#include "Entity/Component.h"

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

#include "System/File.h"
#include "System/Debug.h"

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

            std::string category = sprite_data->source_folder;

            const size_t pos = category.find("res/images/");
            if(pos != std::string::npos)
            {
                const size_t offset = std::size("res/images/");
                const size_t slash_pos = category.find_first_of('/', offset);
                category = category.substr(offset - 1, slash_pos - offset + 1);
            }

            context.ui_icons[sprite_file] = {
                mono::GetTextureFactory()->CreateTexture(sprite_data->texture_file.c_str()),
                sprite_data->frames.front().uv_upper_left,
                sprite_data->frames.front().uv_lower_right,
                sprite_data->frames.front().size,
                category
            };
        }
    }

    void SetupComponents(editor::UIContext& context)
    {
        for(const Component* component : component::GetAllDefaultComponents())
            context.component_items.push_back({component->hash, component->allow_multiple, ComponentNameFromHash(component->hash), component->category});

        context.component_decorators[AREA_EMITTER_COMPONENT] = { nullptr, editor::AreaEmitterFooter };
        context.component_decorators[TEXTURED_POLYGON_COMPONENT] = { nullptr, editor::TexturedPolygonFooter };
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
            //m_entity_manager.Sync();
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
    Config& editor_config,
    uint32_t max_entities)
    : m_window(window)
    , m_entity_manager(entity_manager)
    , m_event_handler(event_handler)
    , m_system_context(system_context)
    , m_editor_config(editor_config)
    , m_preselected_id(NO_SELECTION)
    , m_pick_target(nullptr)
{
    m_mode_stack.push(EditorMode::DEFAULT);

    using namespace std::placeholders;

    m_context.system_context = &system_context;
    m_context.max_entities = max_entities;
    m_context.all_proxy_objects = &m_proxies;

    m_context.context_menu_callback = std::bind(&Editor::OnContextMenu, this, _1);
    m_context.modal_selection_callback = std::bind(&Editor::SelectItemCallback, this, _1);
    m_context.pick_callback = [this](uint32_t* target_data){
        SetPickingTarget(target_data);
        EnterMode(EditorMode::REFERENCE_PICKING);
    };

    m_context.select_reference_callback = [this](uint32_t entity_reference) {
        const uint32_t entity_id = m_entity_manager.GetEntityIdFromUuid(entity_reference);
        SetSelection({ entity_id });
    };

    m_context.delete_callback = std::bind(&Editor::OnDeleteObject, this);
    m_context.switch_world = std::bind(&Editor::SwitchWorld, this, _1);
    m_context.create_new_world = std::bind(&Editor::CreateNewWorld, this, _1);
    m_context.select_object_callback = [this](uint32_t entity_id) {
        const System::ModifierState& state = System::GetModifierState();
        (state.ctrl) ? AddToSelection({ entity_id }) : SetSelection({ entity_id });
    };
    m_context.preselect_object_callback = std::bind(&Editor::PreselectProxyObject, this, _1);
    m_context.teleport_to_object_callback = [this](uint32_t entity_id) {
        IObjectProxy* proxy = FindProxyObject(entity_id);
        TeleportToProxyObject(proxy);
    };

    m_context.add_component = std::bind(&Editor::AddComponent, this, _1);
    m_context.delete_component = std::bind(&Editor::DeleteComponent, this, _1);

    m_context.editor_menu_callback = std::bind(&Editor::EditorMenuCallback, this, _1);
    m_context.tools_menu_callback = std::bind(&Editor::ToolsMenuCallback, this, _1);
    m_context.reset_zoom_callback = std::bind(&Editor::ResetCameraZoom, this);
    m_context.reset_position_callback = std::bind(&Editor::ResetCameraPosition, this);

    m_context.draw_object_names_callback = std::bind(&Editor::EnableDrawObjectNames, this, _1);
    m_context.draw_snappers_callback = std::bind(&Editor::EnableDrawSnappers, this, _1);
    m_context.draw_lights_callback = std::bind(&Editor::EnableLights, this, _1);
    m_context.background_color_callback = std::bind(&Editor::SetBackgroundColor, this, _1);
    m_context.ambient_shade_callback = std::bind(&Editor::SetAmbientShade, this, _1);
    m_context.background_callback = std::bind(&Editor::SetBackgroundTexture, this, _1, _2);

    m_context.entity_name_callback = [&entity_manager](uint32_t entity_uuid_hash){
        const uint32_t entity_id = entity_manager.GetEntityIdFromUuid(entity_uuid_hash);
        return entity_manager.GetEntityName(entity_id);
    };

    editor::LoadAllSprites("res/sprites/all_sprite_files.json");
    editor::LoadAllMusicTracks("res/sound_config.json");
    editor::LoadAllSounds("res/sound/all_sounds.json");
    editor::LoadAllWeapons("res/weapon_config.json");
    editor::LoadAllUILayers("res/ui_config.json");
    editor::LoadAllEntities("res/entities/all_entities.json");
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

    SetLastLightingLayer(RenderLayer::POST_GAMEOBJECTS);

    EnableDrawObjectNames(m_editor_config.draw_object_names);
    EnableDrawSnappers(m_editor_config.draw_snappers);
    EnableDrawOutline(m_editor_config.draw_outline);
    EnableDrawLevelMetadata(m_editor_config.draw_metadata);
    EnableSnapToGrid(m_editor_config.snap_to_grid);
    SwitchWorld(m_editor_config.selected_world);
    
    camera->SetPosition(m_editor_config.camera_position);
    camera->SetViewportSize(m_editor_config.camera_viewport);
    m_camera = camera;

    m_context.grid_size = m_editor_config.grid_size;

    m_input_handler = std::make_unique<ImGuiInputHandler>(m_event_handler);
    m_user_input_controller =
        std::make_unique<editor::UserInputController>(camera, this, &m_context, m_event_handler);

    mono::TransformSystem* transform_system = m_system_context.GetSystem<mono::TransformSystem>();
    mono::TextSystem* text_system = m_system_context.GetSystem<mono::TextSystem>();
    mono::SpriteSystem* sprite_system = m_system_context.GetSystem<mono::SpriteSystem>();
    mono::LightSystem* light_system = m_system_context.GetSystem<mono::LightSystem>();
    mono::PathSystem* path_system = m_system_context.GetSystem<mono::PathSystem>();
    mono::RoadSystem* road_system = m_system_context.GetSystem<mono::RoadSystem>();
    mono::ParticleSystem* particle_system = m_system_context.GetSystem<mono::ParticleSystem>();
    game::WorldBoundsSystem* world_bounds_system = m_system_context.GetSystem<game::WorldBoundsSystem>();
    game::UISystem* ui_system = m_system_context.GetSystem<game::UISystem>();

    ComponentDrawMap draw_funcs;
    draw_funcs[CIRCLE_SHAPE_COMPONENT] = editor::DrawCircleShapeDetails;
    draw_funcs[BOX_SHAPE_COMPONENT] = editor::DrawBoxShapeDetails;
    draw_funcs[SEGMENT_SHAPE_COMPONENT] = editor::DrawSegmentShapeDetails;
    draw_funcs[POLYGON_SHAPE_COMPONENT] = editor::DrawPolygonShapeDetails;
    draw_funcs[SPAWN_POINT_COMPONENT] = editor::DrawSpawnPointDetails;
    draw_funcs[ENTITY_SPAWN_POINT_COMPONENT] = editor::DrawEntitySpawnPointDetails;
    draw_funcs[SHAPE_TRIGGER_COMPONENT] = editor::DrawShapeTriggerComponentDetails;
    draw_funcs[AREA_TRIGGER_COMPONENT] = editor::DrawAreaTriggerComponentDetails;
    draw_funcs[DESTROYED_TRIGGER_COMPONENT] = editor::DrawDestroyedTriggerComponentDetails;
    draw_funcs[TIME_TRIGGER_COMPONENT] = editor::DrawTimeTriggerComponentDetails;
    draw_funcs[COUNTER_TRIGGER_COMPONENT] = editor::DrawCounterTriggerComponentDetails;
    draw_funcs[TRANSLATION_COMPONENT] = editor::DrawSetTranslationDetails;
    draw_funcs[ROTATION_COMPONENT] = editor::DrawSetRotationDetails;
    draw_funcs[SPRITE_COMPONENT] = editor::DrawSpriteDetails;
    draw_funcs[AREA_EMITTER_COMPONENT] = editor::DrawAreaEmitterDetails;
    draw_funcs[PATH_COMPONENT] = editor::DrawPath;

    m_static_background = std::make_unique<mono::StaticBackground>();
    m_selection_visualizer = new SelectionVisualizer(m_selected_ids, m_preselected_id, transform_system);
    m_component_detail_visualizer = std::make_unique<ComponentDetailVisualizer>(draw_funcs, transform_system);

    AddUpdatable(new SyncPoint(m_entity_manager, m_context));

    AddDrawable(m_static_background.get(), RenderLayer::BACKGROUND);
    AddDrawable(new GridVisualizer(m_context.draw_grid), RenderLayer::BACKGROUND);
    AddDrawable(new mono::RoadBatchDrawer(road_system, path_system, transform_system), RenderLayer::BACKGROUND);
    AddDrawable(new GrabberVisualizer(m_grabbers), RenderLayer::GRABBERS);
    AddDrawable(new SnapperVisualizer(m_context.draw_snappers, m_snap_points), RenderLayer::GRABBERS);
    AddDrawable(m_selection_visualizer, RenderLayer::UI);
    AddDrawable(new ObjectNameVisualizer(m_context.draw_object_names, m_proxies), RenderLayer::UI);
    AddDrawable(m_component_detail_visualizer.get(), RenderLayer::GAMEOBJECTS_UI);
    AddDrawable(new GameCameraVisualizer(m_context.draw_level_metadata, m_context.level_metadata), RenderLayer::UI);
    AddDrawable(new mono::SpriteBatchDrawer(transform_system, sprite_system), RenderLayer::GAMEOBJECTS);
    AddDrawable(new mono::TextBatchDrawer(text_system, transform_system), RenderLayer::GAMEOBJECTS);
    AddDrawable(new mono::PathBatchDrawer(path_system, transform_system), RenderLayer::GAMEOBJECTS);
    AddDrawable(new mono::ParticleSystemDrawer(particle_system, transform_system, mono::ParticleDrawLayer::PRE_GAMEOBJECTS), RenderLayer::PRE_GAMEOBJECTS);
    AddDrawable(new mono::ParticleSystemDrawer(particle_system, transform_system, mono::ParticleDrawLayer::POST_GAMEOBJECTS), RenderLayer::POST_GAMEOBJECTS);
    AddDrawable(new editor::ImGuiInterfaceDrawer(m_context), RenderLayer::UI);
    AddDrawable(new mono::LightSystemDrawer(light_system, transform_system), RenderLayer::GAMEOBJECTS);
    AddDrawable(new game::WorldBoundsDrawer(transform_system, world_bounds_system), RenderLayer::POST_GAMEOBJECTS);
    AddDrawable(new game::UISystemDrawer(ui_system, transform_system), RenderLayer::UI);
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

void Editor::CreateNewWorld(const std::string& new_world)
{
    const std::string world_filename = "res/worlds/" + new_world + ".components";
    const bool file_exists_already = file::Exists(world_filename.c_str());
    if(file_exists_already)
    {
        System::Log("'%s' already exists.", world_filename.c_str());
        return;
    }

    System::Log("%s", world_filename.c_str());

    std::vector<IObjectProxyPtr> proxies;
    game::LevelMetadata level_metadata;
    SaveWorld(world_filename.c_str(), proxies, level_metadata);
    AddNewWorld(world_filename.c_str());
}

void Editor::LoadWorld(const std::string& world_filename)
{
    const bool world_exists = file::Exists(world_filename.c_str());
    if(!world_exists)
    {
        System::Log("'%s' does not exist. Abort loading...", world_filename.c_str());
        return;
    }

    ClearSelection();
    PreselectProxyObject(nullptr);

    if(!m_proxies.empty())
    {
        Save();
        m_proxies.clear();
    }

    m_system_context.SyncSystems();

    mono::TransformSystem* transform_system = m_system_context.GetSystem<mono::TransformSystem>();
    editor::World world = ::LoadWorld(world_filename.c_str(), &m_entity_manager, transform_system, this);
    m_proxies = std::move(world.loaded_proxies);

    m_context.level_metadata = world.leveldata.metadata;

    m_world_filename = world_filename;
    m_context.selected_world = world_filename;

    SetBackgroundColor(world.leveldata.metadata.background_color);
    SetAmbientShade(world.leveldata.metadata.ambient_shade);
    SetBackgroundTexture(world.leveldata.metadata.background_size, world.leveldata.metadata.background_texture);

    //ResetCameraZoom();
    //ResetCameraPosition();
}

void Editor::Save()
{
    if(m_world_filename.empty())
    {
        System::Log("editor|Unable to save world, empty filename.");
        return;
    }

    std::sort(m_context.level_metadata.triggers.begin(), m_context.level_metadata.triggers.end());
    SaveWorld(m_world_filename.c_str(), m_proxies, m_context.level_metadata);
    m_context.notifications.emplace_back(save_texture, "Saved...", 2.0f);
}

void Editor::ExportEntity()
{
    for(uint32_t id : m_selected_ids)
    {
        IObjectProxy* proxy = FindProxyObject(id);
        if(!proxy)
            continue;

        std::string filename = "res/entities/" + std::string(proxy->Name()) + ".entity";
        std::replace(filename.begin(), filename.end(), ' ', '_');

        editor::JsonSerializer serializer;
        proxy->Visit(serializer);

        game::LevelMetadata metadata;
        serializer.WriteComponentEntities(filename, metadata);

        editor::AddNewEntity(filename.c_str());
        m_context.notifications.emplace_back(export_texture, "Exported entity...", 2.0f);
    }
}

void Editor::ImportEntity()
{
    m_context.modal_items = GetAllEntities();
    m_context.show_modal_item_selection = true;
}

void Editor::NewEntity()
{
    mono::TransformSystem* transform_system = m_system_context.GetSystem<mono::TransformSystem>();
    mono::Entity new_entity = m_entity_manager.CreateEntity("unnamed", { NAME_FOLDER_COMPONENT, TRANSFORM_COMPONENT });

    const std::vector<Component> components = {
        DefaultComponentFromHash(NAME_FOLDER_COMPONENT),
        DefaultComponentFromHash(TRANSFORM_COMPONENT)
    };

    auto proxy = std::make_unique<ComponentProxy>(new_entity.id, components, &m_entity_manager, transform_system, this);
    proxy->SetPosition(m_camera->GetPosition());

    m_proxies.push_back(std::move(proxy));

    const Selection new_selection = { new_entity.id };
    SetSelection(new_selection);
}

void Editor::SetSelection(const Selection& selected_ids)
{
    m_context.draw_all_objects = false;

    const EditorMode mode = m_mode_stack.top();
    if(mode == EditorMode::REFERENCE_PICKING)
    {
        if(!selected_ids.empty())
        {
            const uint32_t selected_entity_id = selected_ids.front();
            const uint32_t entity_uuid = m_entity_manager.GetEntityUuid(selected_entity_id);

            *m_pick_target = entity_uuid;
            m_pick_target = nullptr;
            m_mode_stack.pop();
        }

        return;
    }

    m_selected_ids = selected_ids;
    UpdateSelection();
}

const Selection& Editor::GetSelection() const
{
    return m_selected_ids;
}

void Editor::AddToSelection(const Selection& selected_ids)
{
    m_selected_ids.insert(m_selected_ids.end(), selected_ids.begin(), selected_ids.end());
    UpdateSelection();
}

void Editor::RemoveFromSelection(const Selection& selected_ids)
{
    const auto remove_check = [&selected_ids](uint32_t id) {
        return (std::find(selected_ids.begin(), selected_ids.end(), id) != selected_ids.end());
    };
    mono::remove_if(m_selected_ids, remove_check);
    UpdateSelection();
}

void Editor::ClearSelection()
{
    m_context.draw_all_objects = false;

    m_selected_ids.clear();
    UpdateSelection();
}

void Editor::UpdateSelection()
{
    std::vector<IObjectProxy*> selected_proxies;
    for(uint32_t id : m_selected_ids)
    {
        IObjectProxy* proxy = FindProxyObject(id);
        if(proxy)
            selected_proxies.push_back(proxy);
    }

    m_context.selected_proxies = selected_proxies;
    m_component_detail_visualizer->SetObjectProxies(selected_proxies);

    UpdateSnappers();
    UpdateGrabbers();
}

void Editor::MoveSelectionToHere()
{
    for(uint32_t id : GetSelection())
    {
        IObjectProxy* proxy = FindProxyObject(id);
        if(proxy)
            proxy->SetPosition(m_camera->GetPosition());
    }
}

void Editor::SetSelectionPoint(const math::Vector& selection_point)
{
    m_selection_visualizer->SetClickPoint(selection_point);
}

void Editor::SetSelectionBox(const math::Quad& selection_box)
{
    m_selection_visualizer->SetSelectionBox(selection_box);
}

void Editor::PreselectProxyObject(IObjectProxy* proxy_object)
{
    m_preselected_id = (proxy_object != nullptr) ? proxy_object->Id() : NO_SELECTION;
    m_context.preselected_proxy_object = proxy_object;
}

void Editor::TeleportToProxyObject(const IObjectProxy* proxy_object)
{
    const std::vector<const IObjectProxy*> proxies = { proxy_object };
    TeleportToProxyObject(proxies);
}

void Editor::TeleportToProxyObject(const std::vector<const IObjectProxy*>& proxies)
{
    if(proxies.empty())
        return;

    math::Quad bb = { math::INF, math::INF, -math::INF, -math::INF };

    for(const IObjectProxy* proxy : proxies)
    {
        const math::Quad& proxy_bounding_box = proxy->GetBoundingBox();
        bb |= proxy_bounding_box;
    }

    const math::Vector position = math::Center(bb);
    m_camera->SetTargetPosition(position);

    const math::Quad viewport = m_camera->GetViewport();
    const float ratio = math::Width(viewport) / math::Height(viewport);

    float width = 0.0f;
    float height = 0.0f;

    // Make the width/height min 1.0f, so that the zoom level is not too close.
    const float bb_width = std::max(math::Width(bb), 1.0f);
    const float bb_height = std::max(math::Height(bb), 1.0f);

    if(bb_width > bb_height)
    {
        width = bb_width;
        height = bb_width / ratio;
    }
    else
    {
        width = bb_height * ratio;
        height = bb_height;
    }

    m_camera->SetTargetViewportSize(math::Vector(width, height) * 3.0f);
}

void Editor::TeleportToSelectedProxyObject()
{
    std::vector<const IObjectProxy*> proxies;

    for(uint32_t id : m_selected_ids)
    {
        IObjectProxy* proxy = FindProxyObject(id);
        if(proxy)
            proxies.push_back(proxy);
    }
    
    TeleportToProxyObject(proxies);
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

std::vector<IObjectProxy*> Editor::FindProxiesFromBox(const math::Quad& world_bb) const
{
    std::vector<IObjectProxy*> found_proxies;

    for(const auto& proxy : m_proxies)
    {
        const bool intersects = proxy->Intersects(world_bb);
        if(intersects)
            found_proxies.push_back(proxy.get());
    }

    return found_proxies;
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
        const float distance = math::DistanceBetween(grabber.position, position);
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

    for(uint32_t id : m_selected_ids)
    {
        IObjectProxy* proxy_object = FindProxyObject(id);
        if(proxy_object)
        {
            const std::vector<editor::Grabber>& proxy_grabbers = proxy_object->GetGrabbers();
            m_grabbers.insert(m_grabbers.end(), proxy_grabbers.begin(), proxy_grabbers.end());
        }
    }
}

float Editor::GetPickingDistance() const
{
    const System::Size& size = m_window->Size();
    return math::Width(m_camera->GetViewport()) / size.width * 5.0f;
}

SnapPair Editor::FindSnapPosition(const math::Vector& position) const
{
    SnapPair snap_pair;
    return snap_pair;

    /*
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
    */
}

void Editor::OnDeleteObject()
{
    const auto find_func = [this](const IObjectProxyPtr& proxy) {
        return (std::find(m_selected_ids.begin(), m_selected_ids.end(), proxy->Id()) != m_selected_ids.end());
    };

    mono::remove_if(m_proxies, find_func);
    ClearSelection();
    PreselectProxyObject(nullptr);
    m_grabbers.clear();
}

void Editor::AddComponent(uint32_t component_hash)
{
    for(uint32_t id : m_selected_ids)
    {
        IObjectProxy* proxy_object = FindProxyObject(id);
        if(!proxy_object)
            continue;

        std::vector<Component>& components = proxy_object->GetComponents();

        const std::vector<Component*> added_components = component::AddComponent(component_hash, components);
        for(Component* component : added_components)
            m_entity_manager.AddComponent(id, component->hash);

        component::SortComponentsByPriority(components);
    }
}

void Editor::DeleteComponent(uint32_t index)
{
    for(uint32_t id : m_selected_ids)
    {
        IObjectProxy* proxy_object = FindProxyObject(id);
        if(!proxy_object)
            continue;

        std::vector<Component>& components = proxy_object->GetComponents();
        m_entity_manager.RemoveComponent(id, components[index].hash);
        components.erase(components.begin() + index);
    }
}

void Editor::AddComponentUI()
{
    m_context.open_add_component = true;
}

void Editor::OnContextMenu(int index)
{
    m_user_input_controller->HandleContextMenu(index);
}

void Editor::SelectItemCallback(int index)
{
    mono::TransformSystem* transform_system = m_system_context.GetSystem<mono::TransformSystem>();

    const std::string& selected_item = m_context.modal_items[index];
    editor::World loaded_world = ::LoadWorld(selected_item.c_str(), &m_entity_manager, transform_system, this);
    if(loaded_world.loaded_proxies.empty())
        return;

    Selection new_selection;
    for(auto& object : loaded_world.loaded_proxies)
    {
        new_selection.push_back(object->Id());
        m_proxies.push_back(std::move(object));
    }

    SetSelection(new_selection);
    TeleportToProxyObject(m_proxies.back().get());
    m_context.notifications.emplace_back(import_texture, "Imported Entity...", 2.0f);
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

bool Editor::DrawLights() const
{
    return m_context.draw_lights;
}

void Editor::EnableLights(bool enable)
{
    m_context.draw_lights = enable;
    m_renderer->ToggleLighting();
}

const mono::Color::RGBA& Editor::BackgroundColor() const
{
    return m_context.level_metadata.background_color;
}

void Editor::SetBackgroundColor(const mono::Color::RGBA& color)
{
    m_context.level_metadata.background_color = color;
    m_renderer->SetClearColor(color);
}

const mono::Color::RGBA& Editor::AmbientShade() const
{
    return m_context.level_metadata.background_color;
}

void Editor::SetAmbientShade(const mono::Color::RGBA& color)
{
    m_context.level_metadata.ambient_shade = color;
    m_renderer->SetAmbientShade(color);
}

void Editor::SetBackgroundTexture(const math::Vector& size, const std::string& background_texture)
{
    if(background_texture.empty())
        m_static_background->Clear();
    else
        m_static_background->Load(size, background_texture.c_str(), mono::TextureModeFlags::REPEAT);
}

void Editor::ResetCameraZoom()
{
    m_camera->SetTargetViewportSize(m_context.level_metadata.camera_size);
}

void Editor::ResetCameraPosition()
{
    m_camera->SetTargetPosition(m_context.level_metadata.camera_position);
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
    m_context.notifications.push_back({information_texture, text_buffer, 1.0f});
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
    Selection duplicated_selection;

    for(uint32_t id : m_selected_ids)
    {
        IObjectProxy* proxy_object = FindProxyObject(id);
        if(!proxy_object)
            continue;

        IObjectProxyPtr cloned_proxy = proxy_object->Clone();
        if(cloned_proxy)
        {
            const uint32_t cloned_entity_id = cloned_proxy->Id();
            
            mono::TransformSystem* transform_system = m_system_context.GetSystem<mono::TransformSystem>();
            math::Matrix& transform = transform_system->GetTransform(cloned_entity_id);
            math::Translate(transform, math::Vector(1.0f, 1.0f));

            m_proxies.push_back(std::move(cloned_proxy));
            duplicated_selection.push_back(cloned_entity_id);
        }
    }

    SetSelection(duplicated_selection);
}

void Editor::ReExportEntities()
{
    mono::TransformSystem* transform_system = m_system_context.GetSystem<mono::TransformSystem>();

    const std::vector<std::string>& all_entities = GetAllEntities();

    for(auto entity_file : all_entities)
    {
        editor::World loaded_world = ::LoadWorld(entity_file.c_str(), &m_entity_manager, transform_system, this);

        for(IObjectProxyPtr& proxy : loaded_world.loaded_proxies)
        {
            std::string filename = "res/entities/" + std::string(proxy->Name()) + ".entity";
            std::replace(filename.begin(), filename.end(), ' ', '_');

            editor::JsonSerializer serializer;
            proxy->Visit(serializer);

            game::LevelMetadata metadata;
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
    m_context.notifications.push_back({information_texture, text_buffer, 5.0f});
}

void Editor::PopMode()
{
    MONO_ASSERT(m_mode_stack.size() > 1);
    m_mode_stack.pop();
}
