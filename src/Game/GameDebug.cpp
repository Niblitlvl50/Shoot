
#include "GameDebug.h"
#include "Player/PlayerInfo.h"
#include "TriggerSystem/TriggerSystem.h"

#include "Events/KeyEvent.h"
#include "Events/EventFuncFwd.h"
#include "Events/PlayerConnectedEvent.h"
#include "EventHandler/EventHandler.h"
#include "Physics/PhysicsDebugDrawer.h"
#include "ImGuiImpl/ImGuiWidgets.h"
#include "Util/Hash.h"

#include "imgui/imgui.h"

bool game::g_draw_client_viewport = false;
bool game::g_draw_navmesh = false;
bool game::g_draw_transformsystem = false;
bool game::g_draw_physics = false;
bool game::g_draw_triggers = false;
uint32_t game::g_draw_physics_subcomponents = mono::PhysicsDebugComponents::DRAW_SHAPES;
bool game::g_draw_physics_stats = false;
bool game::g_draw_particle_stats = false;
bool game::g_draw_network_stats = false;
bool game::g_draw_position_prediction = false;
bool game::g_draw_debug_players = false;
bool game::g_draw_spawn_points = false;

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
        const uint32_t hash_value = mono::Hash(out_buffer);
        trigger_system->EmitTrigger(hash_value);
    }
    
    ImGui::End();
}

void DrawDebugPlayers(bool& show_window, mono::EventHandler* event_handler)
{
    constexpr int flags =
        ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoResize;

    ImGui::SetNextWindowSize(ImVec2(600, -1));

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
        const bool spawn_player_index = ImGui::SmallButton("Spawn");
        if(spawn_player_index)
            event_handler->DispatchEvent(game::SpawnPlayerEvent(index));
        ImGui::PopID();

        ImGui::NextColumn();
    }

    ImGui::End();
}


game::DebugUpdater::DebugUpdater(TriggerSystem* trigger_system, mono::EventHandler* event_handler)
    : m_trigger_system(trigger_system)
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
}

game::DebugUpdater::~DebugUpdater()
{
    m_event_handler->RemoveListener(m_keyup_token);
}

void game::DebugUpdater::Draw(mono::IRenderer& renderer) const
{
    m_counter++;

    if(m_draw_debug_menu)
        DrawDebugMenu(m_counter.Fps());

    if(m_draw_trigger_input)
        DrawTriggerInput(m_draw_trigger_input, m_trigger_system);

    if(game::g_draw_debug_players)
        DrawDebugPlayers(game::g_draw_debug_players, m_event_handler);
}

math::Quad game::DebugUpdater::BoundingBox() const
{
    return math::InfQuad;
}
