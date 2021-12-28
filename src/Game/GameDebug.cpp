
#include "GameDebug.h"
#include "Player/PlayerInfo.h"
#include "TriggerSystem/TriggerSystem.h"
#include "DamageSystem.h"

#include "Events/KeyEvent.h"
#include "Events/EventFuncFwd.h"
#include "Events/MouseEvent.h"
#include "Events/PlayerConnectedEvent.h"
#include "EntitySystem/IEntityManager.h"
#include "EventHandler/EventHandler.h"
#include "Physics/PhysicsDebugDrawer.h"
#include "ImGuiImpl/ImGuiWidgets.h"
#include "System/Hash.h"
#include "System/System.h"

#include "TransformSystem/TransformSystem.h"
#include "Math/MathFunctions.h"
#include "Math/Matrix.h"

#include "Component.h"

#include "imgui/imgui.h"

bool game::g_draw_client_viewport = false;
bool game::g_draw_navmesh = false;
bool game::g_draw_transformsystem = false;
bool game::g_draw_physics = false;
bool game::g_interact_physics = false;
bool game::g_draw_triggers = false;
uint32_t game::g_draw_physics_subcomponents = mono::PhysicsDebugComponents::DRAW_SHAPES;
bool game::g_draw_physics_stats = false;
bool game::g_draw_particle_stats = false;
bool game::g_draw_network_stats = false;
bool game::g_draw_position_prediction = false;
bool game::g_draw_debug_players = false;
bool game::g_draw_spawn_points = false;

constexpr uint32_t NO_ID = std::numeric_limits<uint32_t>::max();

void DrawDebugMenu(uint32_t fps)
{
    ImGui::BeginMainMenuBar();
    if(ImGui::BeginMenu("Options"))
    {
        ImGui::Checkbox("Transform System",     &game::g_draw_transformsystem);
        ImGui::Checkbox("Navmesh",              &game::g_draw_navmesh);
        ImGui::Checkbox("Triggers",             &game::g_draw_triggers);
        ImGui::Checkbox("Physics",              &game::g_draw_physics);
        ImGui::SameLine();
        mono::DrawBitFieldType(
            game::g_draw_physics_subcomponents,
            mono::all_physics_debug_component,
            std::size(mono::all_physics_debug_component),
            mono::PhsicsDebugComponentToString);
        ImGui::Checkbox("Physics Interact",     &game::g_interact_physics);
        ImGui::Checkbox("Physics Stats",        &game::g_draw_physics_stats);
        ImGui::Checkbox("Particle Stats",       &game::g_draw_particle_stats);
        ImGui::Checkbox("Network Stats",        &game::g_draw_network_stats);
        ImGui::Checkbox("Client Viewport",      &game::g_draw_client_viewport);
        ImGui::Checkbox("Prediction System",    &game::g_draw_position_prediction);
        ImGui::Checkbox("Players",              &game::g_draw_debug_players);
        ImGui::Checkbox("Spawn Points",         &game::g_draw_spawn_points);

        ImGui::EndMenu();
    }

    ImGui::SameLine(ImGui::GetWindowWidth() -70);
    ImGui::TextDisabled("fps: %u", fps);

    ImGui::EndMainMenuBar();
}

void DrawTriggerInput(bool& draw_trigger_input, game::TriggerSystem* trigger_system)
{
    if(!trigger_system)
        return;

    constexpr int flags =
        ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoResize;

    ImGui::Begin("TriggerInput", &draw_trigger_input, flags);

    char out_buffer[128] = {};
    const bool input_entered = ImGui::InputText("Trigger", out_buffer, std::size(out_buffer), ImGuiInputTextFlags_EnterReturnsTrue);
    if(input_entered)
    {
        const uint32_t hash_value = hash::Hash(out_buffer);
        trigger_system->EmitTrigger(hash_value);
    }
    
    ImGui::End();
}

void DrawDebugPlayers(bool& show_window, game::DamageSystem* damage_system, mono::EventHandler* event_handler)
{
    constexpr int flags =
        ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoResize;

    ImGui::SetNextWindowSize(ImVec2(660, -1));

    ImGui::Begin("DebugPlayers", &show_window, flags);

    ImGui::Columns(7, "mycolumns");
    ImGui::Separator();
    ImGui::Text("Index"); ImGui::NextColumn();
    ImGui::Text("Entity"); ImGui::NextColumn();
    ImGui::Text("State"); ImGui::NextColumn();
    ImGui::Text("Lives"); ImGui::NextColumn();
    ImGui::Text("Position"); ImGui::NextColumn();
    ImGui::Text("Viewport"); ImGui::NextColumn();
    ImGui::NextColumn();
    ImGui::Separator();

    ImGui::SetColumnWidth(0, 60);
    ImGui::SetColumnWidth(1, 60);
    ImGui::SetColumnWidth(2, 100);
    ImGui::SetColumnWidth(3, 60);
    ImGui::SetColumnWidth(5, 170);
    ImGui::SetColumnWidth(6, 170);

    for(int index = 0; index < game::n_players; ++index)
    {
        game::PlayerInfo& player_info = game::g_players[index];

        ImGui::Text("%d", index);
        ImGui::NextColumn();
        
        ImGui::Text("%u", player_info.entity_id);
        ImGui::NextColumn();

        ImGui::Text("%s", game::PlayerStateToString(player_info.player_state));
        ImGui::NextColumn();

        ImGui::Text("%d", player_info.lives);
        ImGui::NextColumn();

        ImGui::Text("%.1f %.1f", player_info.position.x, player_info.position.y);
        ImGui::NextColumn();

        const math::Quad& viewport = player_info.viewport;
        ImGui::Text("%.1f %.1f %.1f %.1f", viewport.mA.x, viewport.mA.y, viewport.mB.x, viewport.mB.y);
        ImGui::NextColumn();

        ImGui::PushID(index);

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

        const bool player_alive = (player_info.player_state == game::PlayerState::ALIVE);
        const char* kill_button_text = player_alive ? "Kill" : "Respawn";

        const bool kill_player = ImGui::SmallButton(kill_button_text);
        if(kill_player)
        {
            if(player_alive)
                damage_system->ApplyDamage(player_info.entity_id, 1000000, NO_ID);
            else
                event_handler->DispatchEvent(game::RespawnPlayerEvent(player_info.entity_id));
        }

        ImGui::PopID();

        ImGui::NextColumn();
    }

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
                System::Log("\t%s", ComponentNameFromHash(component_hash));

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
    TriggerSystem* trigger_system,
    DamageSystem* damage_system,
    mono::TransformSystem* transform_system,
    mono::IEntityManager* entity_manager,
    mono::EventHandler* event_handler)
    : m_trigger_system(trigger_system)
    , m_damage_system(damage_system)
    , m_event_handler(event_handler)
    , m_draw_debug_menu(false)
    , m_draw_trigger_input(false)
{
    const event::KeyUpEventFunc key_up_func = [this](const event::KeyUpEvent& event) {
        if(event.key == Keycode::R)
            m_draw_debug_menu = !m_draw_debug_menu;
        else if(event.key == Keycode::T)
            m_draw_trigger_input = !m_draw_trigger_input;
        else if(event.key == Keycode::P)
            game::g_draw_debug_players = !game::g_draw_debug_players;

        return mono::EventResult::PASS_ON;
    };
    m_keyup_token = m_event_handler->AddListener(key_up_func);

    m_player_debug_handler = std::make_unique<PlayerDebugHandler>(game::g_draw_debug_players, transform_system, entity_manager, event_handler);
}

DebugUpdater::~DebugUpdater()
{
    m_event_handler->RemoveListener(m_keyup_token);
}

void DebugUpdater::Draw(mono::IRenderer& renderer) const
{
    m_counter++;

    if(m_draw_debug_menu)
        DrawDebugMenu(m_counter.Fps());

    if(m_draw_trigger_input)
        DrawTriggerInput(m_draw_trigger_input, m_trigger_system);

    if(game::g_draw_debug_players)
        DrawDebugPlayers(game::g_draw_debug_players, m_damage_system, m_event_handler);
}

math::Quad DebugUpdater::BoundingBox() const
{
    return math::InfQuad;
}
