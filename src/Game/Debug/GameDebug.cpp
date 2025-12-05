
#include "GameDebug.h"
#include "Player/PlayerInfo.h"
#include "TriggerSystem/TriggerSystem.h"
#include "DamageSystem/DamageSystem.h"
#include "Entity/EntityLogicSystem.h"
#include "Navigation/NavmeshData.h"
#include "Weapons/WeaponTypes.h"
#include "Zones/ZoneManager.h"

#include "SystemContext.h"
#include "Events/KeyEvent.h"
#include "Events/EventFuncFwd.h"
#include "Events/MouseEvent.h"
#include "Events/PauseEvent.h"
#include "Events/PlayerEvents.h"
#include "Events/QuitEvent.h"
#include "EntitySystem/IEntityManager.h"
#include "EventHandler/EventHandler.h"
#include "Physics/PhysicsDebugDrawer.h"
#include "ImGuiImpl/ImGuiWidgets.h"
#include "System/Hash.h"
#include "System/System.h"

#include "TransformSystem/TransformSystem.h"
#include "Math/MathFunctions.h"
#include "Math/Matrix.h"
#include "Rendering/IRenderer.h"
#include "Rendering/RenderSystem.h"

#include "Entity/Component.h"

#include "imgui/imgui.h"

bool game::g_draw_client_viewport = false;
bool game::g_draw_navmesh = false;
uint32_t game::g_draw_navmesh_subcomponents = game::NavigationDebugComponents::DRAW_RECENT_PATHS;
bool game::g_draw_transformsystem = false;
bool game::g_draw_rendersystem = true;
bool game::g_draw_physics = false;
bool game::g_interact_physics = false;
bool game::g_body_introspection = false;
bool game::g_draw_triggers = false;
bool game::g_draw_targets = false;
uint32_t game::g_draw_physics_subcomponents = mono::PhysicsDebugComponents::DRAW_SHAPES;
bool game::g_draw_physics_stats = false;
bool game::g_draw_particle_stats = false;
bool game::g_draw_network_stats = false;
bool game::g_draw_position_prediction = false;
bool game::g_draw_debug_players = false;
bool game::g_draw_debug_frametimes = false;
bool game::g_draw_spawn_points = false;
bool game::g_draw_camera_debug = false;

bool game::g_draw_debug_uisystem = false;
bool game::g_draw_ui_element_bounds = false;

bool game::g_debug_camera = false;
bool game::g_debug_draw_damage_words = false;
bool game::g_debug_draw_shockwaves = false;


bool game::g_draw_debug_soundsystem = false;
bool game::g_mute_soundsystem = false;


bool game::g_draw_entity_introspection = false;

constexpr uint32_t NO_ID = std::numeric_limits<uint32_t>::max();

void DrawDebugMenu(game::EntityLogicSystem* logic_system, mono::EventHandler* event_handler, uint32_t fps, float delta)
{
    ImGui::BeginMainMenuBar();
    if(ImGui::BeginMenu("Options"))
    {
        ImGui::Checkbox("Transform System",     &game::g_draw_transformsystem);
        ImGui::Checkbox("Render System",        &game::g_draw_rendersystem);
        ImGui::Checkbox("Navmesh",              &game::g_draw_navmesh);
        ImGui::PushID("NavmeshBitField");
        mono::DrawBitFieldType(
            game::g_draw_navmesh_subcomponents, game::all_navigation_debug_component, std::size(game::all_navigation_debug_component), game::NavigationDebugComponentToString);
        ImGui::PopID();
        ImGui::Checkbox("Triggers",             &game::g_draw_triggers);
        ImGui::Checkbox("Targets",              &game::g_draw_targets);
        ImGui::Checkbox("Physics",              &game::g_draw_physics);
        ImGui::SameLine();
        ImGui::PushID("PhysicsBitField");
        mono::DrawBitFieldType(
            game::g_draw_physics_subcomponents,
            mono::all_physics_debug_component,
            std::size(mono::all_physics_debug_component),
            mono::PhsicsDebugComponentToString);
        ImGui::PopID();
        ImGui::Checkbox("Physics Stats",        &game::g_draw_physics_stats);
        ImGui::Checkbox("Particle Stats",       &game::g_draw_particle_stats);
        ImGui::Checkbox("Network Stats",        &game::g_draw_network_stats);
        ImGui::Checkbox("Client Viewport",      &game::g_draw_client_viewport);
        ImGui::Checkbox("Prediction System",    &game::g_draw_position_prediction);
        ImGui::Checkbox("Frame Times",          &game::g_draw_debug_frametimes);
        ImGui::Checkbox("Spawn Points",         &game::g_draw_spawn_points);
        ImGui::Checkbox("Camera Debug",         &game::g_draw_camera_debug);

        ImGui::Separator();
        ImGui::Checkbox("UI Debug",             &game::g_draw_debug_uisystem);
        ImGui::Checkbox("UI Element Bounds",    &game::g_draw_ui_element_bounds);

        ImGui::Separator();
        ImGui::Checkbox("Debug Camera",         &game::g_debug_camera);
        ImGui::Checkbox("Damage Words",         &game::g_debug_draw_damage_words);
        ImGui::Checkbox("Shockwaves",           &game::g_debug_draw_shockwaves);
        ImGui::Separator();
        ImGui::Checkbox("Sound System",         &game::g_draw_debug_soundsystem);
        ImGui::Checkbox("Mute Sounds",          &game::g_mute_soundsystem);

        ImGui::EndMenu();
    }

    if(ImGui::BeginMenu("Debug Logic"))
    {
        const std::vector<game::EntityDebugCategory>& debug_categories = logic_system->GetDebugCategories();

        for(const game::EntityDebugCategory& debug_category : debug_categories)
        {
            bool enabled = debug_category.active;
            const bool changed = ImGui::Checkbox(debug_category.category, &enabled);
            if(changed)
                logic_system->SetDebugCategory(debug_category.category, enabled);
        }

        ImGui::EndMenu();
    }

    if(ImGui::BeginMenu("Tools"))
    {
        ImGui::Checkbox("Physics Interact",     &game::g_interact_physics);
        ImGui::Checkbox("Body Introspection",   &game::g_body_introspection);
        ImGui::Checkbox("Players",              &game::g_draw_debug_players);
        ImGui::Checkbox("Entity Introspection", &game::g_draw_entity_introspection);
        ImGui::EndMenu();
    }

    if(ImGui::BeginMenu("Levels"))
    {
        uint32_t selected_index = -1;

        const std::vector<game::Level>& levels = game::ZoneManager::GetLevels();
        for(uint32_t index = 0; index < levels.size(); ++index)
        {
            const game::Level& level = levels[index];

            if(ImGui::MenuItem(level.name.c_str(), "", false))
                selected_index = index;
        }

        ImGui::EndMenu();

        if(selected_index != -1)
        {
            game::ZoneManager::SwitchToLevel(levels[selected_index]);
            event_handler->DispatchEvent(event::QuitEvent());
        }
    }

    ImGui::SameLine(ImGui::GetWindowWidth() -150);
    ImGui::TextDisabled("fps: %u, dt: %.3f", fps, delta);

    ImGui::EndMainMenuBar();
}

void DrawTriggerInput(bool& draw_trigger_input, mono::TriggerSystem* trigger_system)
{
    if(!trigger_system)
        return;

    constexpr int flags =
        ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoResize;

    if(ImGui::Begin("TriggerInput", &draw_trigger_input, flags))
    {
        // Set focus to next widget, but this cause the window to not being able to be closed.
        //ImGui::SetKeyboardFocusHere();
        
        char out_buffer[128] = {};
        const bool input_entered = ImGui::InputText(
            "Trigger", out_buffer, std::size(out_buffer), ImGuiInputTextFlags_EnterReturnsTrue);
        if(input_entered)
        {
            const uint32_t hash_value = hash::Hash(out_buffer);
            trigger_system->EmitTrigger(hash_value);
        }

        const bool next_level = ImGui::Button("Level Completed");
        if(next_level)
            trigger_system->EmitTrigger(hash::Hash("level_completed"));

        const bool aborted_level = ImGui::Button("Level Aborted");
        if(aborted_level)
            trigger_system->EmitTrigger(hash::Hash("level_aborted"));

        const bool game_over = ImGui::Button("Game Over");
        if(game_over)
            trigger_system->EmitTrigger(hash::Hash("level_gameover"));

        const bool show_shop_screen = ImGui::Button("Shop Screen");
        if(show_shop_screen)
            trigger_system->EmitTrigger(hash::Hash("show_shop_screen"));

        ImGui::End();
    }
}

void DrawFrameTimes(bool& show_window, const mono::CircularVector<float, 1000>& frame_times)
{
    constexpr int flags = 0;
//        ImGuiWindowFlags_AlwaysAutoResize;

    ImGui::SetNextWindowSize(ImVec2(1000, 250));
    ImGui::Begin("DebugFrameTimes", &show_window, flags);

    ImGui::PlotLines(
        "Frame Times",
        frame_times.Data(),
        frame_times.MaxSize(),
        frame_times.Offset(),
        nullptr,
        0.01f, 0.064f,
        ImVec2(-1, 200));

/*
    ImGui::PlotHistogram(
        "Frame Times Histo",
        frame_times.Data(),
        frame_times.MaxSize(),
        frame_times.Offset(),
        nullptr,
        0.01f, 0.064f,
        ImVec2(-1, 200));
*/

    ImGui::End();
}

void DrawDebugPlayers(bool& show_window, game::DamageSystem* damage_system, mono::EventHandler* event_handler)
{
    constexpr int flags =
        ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoResize;

    ImGui::SetNextWindowSize(ImVec2(1100, -1));
    ImGui::Begin("DebugPlayers", &show_window, flags);

    const bool table_result = ImGui::BeginTable("player_table", 10, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_SizingFixedFit);
    if(table_result)
    {
        ImGui::TableSetupColumn("Index", 0, 60);
        ImGui::TableSetupColumn("Entity", 0, 60);
        ImGui::TableSetupColumn("Controller", 0, 80);
        ImGui::TableSetupColumn("State", 0, 100);
        ImGui::TableSetupColumn("Position", 0, 100);
        //ImGui::TableSetupColumn("Viewport", 0, 150);
        ImGui::TableSetupColumn("GodMode", 0, 60);
        ImGui::TableSetupColumn("DamageScalar", 0, 80);
        ImGui::TableSetupColumn("AutoAim", 0, 60);
        ImGui::TableSetupColumn("AutoReload", 0, 80);
        ImGui::TableSetupColumn("Actions", 0, 150);
        ImGui::TableHeadersRow();

        for(int index = 0; index < game::n_players; ++index)
        {
            game::PlayerInfo& player_info = game::g_players[index];

            ImGui::PushID(index);
            ImGui::TableNextRow();

            ImGui::TableNextColumn(); ImGui::Text("%d", index);
            ImGui::TableNextColumn(); ImGui::Text("%u", player_info.entity_id);
            ImGui::TableNextColumn(); ImGui::Text("%d", player_info.controller_id);
            ImGui::TableNextColumn(); ImGui::Text("%s", game::PlayerStateToString(player_info.player_state));
            ImGui::TableNextColumn(); ImGui::Text("%.1f %.1f", player_info.position.x, player_info.position.y);

            /*
            ImGui::TableNextColumn();
            const math::Quad& viewport = player_info.viewport;
            ImGui::Text(
                "%.1f %.1f %.1f %.1f",
                viewport.bottom_left.x,
                viewport.bottom_left.y,
                viewport.top_right.x,
                viewport.top_right.y);
            */

            {
                ImGui::TableNextColumn();
                bool god_mode = damage_system->IsInvincible(player_info.entity_id);
                const bool changed = ImGui::Checkbox("##godmode_id", &god_mode);
                if(changed)
                {
                    damage_system->SetInvincible(player_info.entity_id, god_mode);
                    player_info.persistent_data.god_mode = god_mode;
                }

                ImGui::TableNextColumn();
                float damage_multiplier = damage_system->GetDamageMultiplier(player_info.entity_id);
                const bool multiplier_changed = ImGui::InputFloat("##damage_multiplier_id", &damage_multiplier);
                if(multiplier_changed)
                {
                    damage_system->SetDamageMultiplier(player_info.entity_id, damage_multiplier);
                    player_info.persistent_data.damage_multiplier = damage_multiplier;
                }
            }

            ImGui::TableNextColumn();
            ImGui::Checkbox("##autoaim_id", &player_info.persistent_data.auto_aim);

            ImGui::TableNextColumn();
            ImGui::Checkbox("##autoreload_id", &player_info.persistent_data.auto_reload);

            {
                // Buttons
                ImGui::TableNextColumn();

                const bool not_spawned = (player_info.player_state == game::PlayerState::NOT_SPAWNED);
                const char* button_text = not_spawned ? "Spawn" : "Despawn";

                const bool spawn_player_index = ImGui::SmallButton(button_text);
                if(spawn_player_index)
                {
                    if(not_spawned)
                        event_handler->DispatchEvent(game::SpawnPlayerEvent(index));
                    else
                        event_handler->DispatchEvent(game::DespawnPlayerEvent(index));
                }

                ImGui::SameLine();

                if(player_info.player_state == game::PlayerState::ALIVE)
                {
                    const bool kill_player = ImGui::SmallButton("Kill");
                    if(kill_player)
                        damage_system->ApplyDamage(player_info.entity_id, NO_ID, game::NO_WEAPON_IDENTIFIER, game::DamageDetails(1000000, true, false, false));
                }
                else if(player_info.player_state == game::PlayerState::DEAD)
                {
                    const bool respawn_player = ImGui::SmallButton("Respawn");
                    if(respawn_player)
                        event_handler->DispatchEvent(game::RespawnPlayerEvent(player_info.entity_id));
                }

            }

            ImGui::PopID();
        }

        ImGui::EndTable();
    }

    const bool package_table_result = ImGui::BeginTable("package_table", 3, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_SizingFixedFit);
    if(package_table_result)
    {
        ImGui::TableSetupColumn("Entity", 0, 60);
        ImGui::TableSetupColumn("State", 0, 100);
        ImGui::TableSetupColumn("Index", 0, 60);
        ImGui::TableHeadersRow();

        ImGui::TableNextRow();

        ImGui::TableNextColumn(); ImGui::Text("%u", game::g_package_info.entity_id);
        ImGui::TableNextColumn(); ImGui::Text("%s", game::PackageStateToString(game::g_package_info.state));
        ImGui::TableNextColumn(); ImGui::Text("%d", 0);

        ImGui::EndTable();
    }

    ImGui::End();
}

void DrawDebugEntityIntrospection(bool& show_window, mono::IEntityManager* entity_manager)
{
    static int s_entity_id = 0;

    constexpr int flags =
        ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoResize;

    ImGui::SetNextWindowSize(ImVec2(500, -1));
    ImGui::Begin("DebugEntities", &show_window, flags);

    ImGui::InputInt("Entity Id", &s_entity_id);
    ImGui::Separator();
    ImGui::Spacing();

    const mono::Entity* entity = entity_manager->GetEntity(s_entity_id);
    if(entity)
    {
        const bool package_table_result = ImGui::BeginTable("package_table", 3, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_SizingFixedFit);
        if(package_table_result)
        {
            ImGui::TableSetupColumn("Name", 0, 200);
            ImGui::TableSetupColumn("State", 0, 100);
            ImGui::TableSetupColumn("Index", 0, 30);
            ImGui::TableHeadersRow();

            ImGui::TableNextRow();

            ImGui::TableNextColumn(); ImGui::Text("%s", entity_manager->GetEntityName(s_entity_id));
            ImGui::TableNextColumn(); ImGui::Text("%s", "Unknoiwn....");
            ImGui::TableNextColumn(); ImGui::Text("%d", s_entity_id);

            ImGui::EndTable();
        }
    }
    else
    {
        ImGui::TextDisabled("Invalid entity id...");
    }

    ImGui::End();
}

void DrawDebugRendersystem(bool& show_window)
{
    constexpr int flags =
        ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoResize;

    const mono::BufferStatus& buffer_status = mono::RenderSystem::GetBufferStatus();

    ImGui::Begin("Rendersystem", &show_window, flags);

    ImGui::TextDisabled("Buffers");
    ImGui::Text("maked: %d", buffer_status.make_buffers);
    ImGui::Text("destroyed: %d", buffer_status.destroyed_buffers);
    ImGui::Text("diff: %d", buffer_status.make_buffers - buffer_status.destroyed_buffers);

    ImGui::Separator();

    ImGui::TextDisabled("Images");
    ImGui::Text("maked: %d", buffer_status.make_images);
    ImGui::Text("destroyed: %d", buffer_status.destroy_images);
    ImGui::Text("diff: %d", buffer_status.make_images - buffer_status.destroy_images);

    ImGui::End();
}


using namespace game;

class DebugUpdater::PlayerDebugHandler
{
public:
    PlayerDebugHandler(
        const bool& enabled, mono::TransformSystem* transform_system, mono::IEntityManager* entity_manager, mono::EventHandler* event_handler)
        : m_enabled(enabled)
        , m_transform_system(transform_system)
        , m_entity_manager(entity_manager)
        , m_event_handler(event_handler)
        , m_player_id(NO_ID)
    {
        using namespace std::placeholders;
        const event::MouseDownEventFunc mouse_down_func = std::bind(&PlayerDebugHandler::OnMouseDown, this, _1);
        const event::MouseUpEventFunc mouse_up_func = std::bind(&PlayerDebugHandler::OnMouseUp, this, _1);
        const event::MouseMotionEventFunc mouse_motion_func = std::bind(&PlayerDebugHandler::OnMouseMotion, this, _1);

        m_mouse_down_token = m_event_handler->AddListener(mouse_down_func);
        m_mouse_up_token = m_event_handler->AddListener(mouse_up_func);
        m_mouse_motion_token = m_event_handler->AddListener(mouse_motion_func);
    }

    ~PlayerDebugHandler()
    {
        m_event_handler->RemoveListener(m_mouse_down_token);
        m_event_handler->RemoveListener(m_mouse_up_token);
        m_event_handler->RemoveListener(m_mouse_motion_token);
    }

    mono::EventResult OnMouseDown(const event::MouseDownEvent& event)
    {
        if(!m_enabled)
            return mono::EventResult::PASS_ON;

        const math::Vector world_click = {event.world_x, event.world_y};
   
        uint32_t found_index = NO_ID;

        const auto check_if_in_bb = [&, this](const mono::TransformSystem::Component& component, uint32_t index)
        {
            const math::Quad world_bb = m_transform_system->GetWorldBoundingBox(index);
            const bool found_entity = math::PointInsideQuad(world_click, world_bb);
            if(found_entity)
                found_index = index;

            return found_entity;
        };
        m_transform_system->ForEachComponentBreak(check_if_in_bb);

        if(found_index != NO_ID)
        {
            const mono::Entity* found_entity = m_entity_manager->GetEntity(found_index);
            System::Log("Found id %u '%s'", found_index, found_entity->name);

            for(uint32_t component_hash : found_entity->components)
                System::Log("\t%s", component::ComponentNameFromHash(component_hash));

            m_player_id = found_index;
            m_previous_position = world_click;
        }
        return found_index != NO_ID ? mono::EventResult::HANDLED : mono::EventResult::PASS_ON;
    }

    mono::EventResult OnMouseUp(const event::MouseUpEvent& event)
    {
        m_player_id = NO_ID;
        return mono::EventResult::PASS_ON;
    }

    mono::EventResult OnMouseMotion(const event::MouseMotionEvent& event)
    {
        if(m_player_id != NO_ID)
        {
            const math::Vector world_position = { event.world_x, event.world_y };
            const math::Vector delta = world_position - m_previous_position;
            math::Matrix& transform = m_transform_system->GetTransform(m_player_id);
            math::Translate(transform, delta);

            m_transform_system->SetTransformState(m_player_id, mono::TransformState::CLIENT);

            m_previous_position = world_position;
        }

        return mono::EventResult::PASS_ON;
    }

    const bool& m_enabled;
    mono::TransformSystem* m_transform_system;
    mono::IEntityManager* m_entity_manager;
    mono::EventHandler* m_event_handler;

    mono::EventToken<event::MouseDownEvent> m_mouse_down_token;
    mono::EventToken<event::MouseUpEvent> m_mouse_up_token;
    mono::EventToken<event::MouseMotionEvent> m_mouse_motion_token;

    uint32_t m_player_id;
    math::Vector m_previous_position;
};

DebugUpdater::DebugUpdater(
    mono::SystemContext* system_context,
    mono::EventHandler* event_handler,
    mono::IRenderer* renderer)
    : m_event_handler(event_handler)
    , m_draw_debug_menu(false)
    , m_draw_trigger_input(false)
    , m_pause(false)
{
    m_trigger_system = system_context->GetSystem<mono::TriggerSystem>();
    m_damage_system = system_context->GetSystem<DamageSystem>();
    m_logic_system = system_context->GetSystem<EntityLogicSystem>();

    const event::KeyUpEventFunc key_up_func = [this, renderer](const event::KeyUpEvent& event) {
        if(!event.ctrl)
            return mono::EventResult::PASS_ON;

        if(event.key == Keycode::R)
            m_draw_debug_menu = !m_draw_debug_menu;
        else if(event.key == Keycode::T)
            m_draw_trigger_input = !m_draw_trigger_input;
        else if(event.key == Keycode::P)
            game::g_draw_debug_players = !game::g_draw_debug_players;
        else if(event.key == Keycode::O)
        {
            m_pause = !m_pause;
            m_event_handler->DispatchEvent(event::PauseEvent(m_pause)); 
        }
        else if(event.key == Keycode::L && event.ctrl)
            renderer->ToggleLighting();

        return mono::EventResult::HANDLED;
    };
    m_keyup_token = m_event_handler->AddListener(key_up_func);

    mono::TransformSystem* transform_system = system_context->GetSystem<mono::TransformSystem>();
    m_entity_manager = system_context->GetSystem<mono::IEntityManager>();
    m_player_debug_handler = std::make_unique<PlayerDebugHandler>(game::g_draw_debug_players, transform_system, m_entity_manager, event_handler);
}

DebugUpdater::~DebugUpdater()
{
    m_event_handler->RemoveListener(m_keyup_token);
}

void DebugUpdater::Update(const mono::UpdateContext& update_context)
{
    m_counter++;
    m_frame_times.Push(update_context.delta_s_raw);
}

void DebugUpdater::Draw(mono::IRenderer& renderer) const
{
    if(m_draw_debug_menu)
        DrawDebugMenu(m_logic_system, m_event_handler, m_counter.Fps(), m_counter.Delta());

    if(m_draw_trigger_input)
        DrawTriggerInput(m_draw_trigger_input, m_trigger_system);

    if(game::g_draw_debug_frametimes)
        DrawFrameTimes(game::g_draw_debug_frametimes, m_frame_times);

    if(game::g_draw_debug_players)
        DrawDebugPlayers(game::g_draw_debug_players, m_damage_system, m_event_handler);

    if(game::g_draw_entity_introspection)
        DrawDebugEntityIntrospection(game::g_draw_entity_introspection, m_entity_manager);

    if(game::g_draw_rendersystem)
    {
        DrawDebugRendersystem(game::g_draw_rendersystem);
    }
}

math::Quad DebugUpdater::BoundingBox() const
{
    return math::InfQuad;
}
