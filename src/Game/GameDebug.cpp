
#include "GameDebug.h"
#include "AIKnowledge.h"

#include "Events/KeyEvent.h"
#include "Events/EventFuncFwd.h"
#include "EventHandler/EventHandler.h"
#include "Physics/PhysicsDebugDrawer.h"
#include "ImGuiImpl/ImGuiWidgets.h"

#include "imgui/imgui.h"

bool game::g_draw_client_viewport = false;
bool game::g_draw_navmesh = false;
bool game::g_draw_transformsystem = false;
bool game::g_draw_fps = false;
bool game::g_draw_physics = false;
bool game::g_draw_audio = false;
bool game::g_draw_triggers = false;
uint32_t game::g_draw_physics_subcomponents = mono::PhysicsDebugComponents::DRAW_SHAPES;
bool game::g_draw_physics_stats = false;
bool game::g_draw_particle_stats = false;
bool game::g_draw_network_stats = false;
bool game::g_draw_position_prediction = false;

void DrawDebugMenu(uint32_t fps)
{
    ImGui::BeginMainMenuBar();
    if(ImGui::BeginMenu("Options"))
    {
        ImGui::Checkbox("Draw Client Viewport", &game::g_draw_client_viewport);
        ImGui::Checkbox("Draw Navmesh",         &game::g_draw_navmesh);
        ImGui::Checkbox("Draw TransformSystem", &game::g_draw_transformsystem);
        ImGui::Checkbox("Draw Physics",         &game::g_draw_physics);
        ImGui::Indent();
        mono::DrawBitFieldType(
            game::g_draw_physics_subcomponents, mono::all_physics_debug_component, std::size(mono::all_physics_debug_component), mono::PhsicsDebugComponentToString);
        ImGui::Unindent();
        ImGui::Checkbox("Draw Audio",           &game::g_draw_audio);
        ImGui::Checkbox("Draw Triggers",        &game::g_draw_triggers);
        ImGui::Checkbox("Draw FPS",             &game::g_draw_fps);
        ImGui::Checkbox("Draw Physics Stats",   &game::g_draw_physics_stats);
        ImGui::Checkbox("Draw Particle Stats",  &game::g_draw_particle_stats);
        ImGui::Checkbox("Draw Network Stats",   &game::g_draw_network_stats);
        ImGui::Checkbox("Draw Prediction Sys",  &game::g_draw_position_prediction);

        ImGui::EndMenu();
    }

    ImGui::SameLine(ImGui::GetWindowWidth() -100);
    ImGui::TextDisabled("fps: %u", fps);

    ImGui::EndMainMenuBar();
}

void DrawDebugPlayers(bool& show_window)
{
    constexpr int flags =
        ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoResize;

    ImGui::SetNextWindowSize(ImVec2(600, -1));

    ImGui::Begin("DebugPlayers", &show_window, flags);

    ImGui::Columns(5, "mycolumns");
    ImGui::Separator();
    ImGui::Text("Index"); ImGui::NextColumn();
    ImGui::Text("Entity"); ImGui::NextColumn();
    ImGui::Text("State"); ImGui::NextColumn();
    ImGui::Text("Position"); ImGui::NextColumn();
    ImGui::Text("Viewport"); ImGui::NextColumn();
    ImGui::Separator();

    ImGui::SetColumnWidth(0, 60);
    ImGui::SetColumnWidth(1, 60);

    for(int index = 0; index < game::n_players; ++index)
    {
        game::PlayerInfo& player_info = game::g_players[index];

        ImGui::Text("%d", index);
        ImGui::NextColumn();
        
        ImGui::Text("%u", player_info.entity_id);
        ImGui::NextColumn();

        ImGui::Text("%s", game::PlayerStateToString(player_info.player_state));
        ImGui::NextColumn();

        ImGui::Text("%.1f %.1f", player_info.position.x, player_info.position.y);
        ImGui::NextColumn();

        const math::Quad& viewport = player_info.viewport;
        ImGui::Text("%.1f %.1f %.1f %.1f", viewport.mA.x, viewport.mA.y, viewport.mB.x, viewport.mB.y);
        ImGui::NextColumn();
    }

    ImGui::End();
}


game::DebugUpdater::DebugUpdater(mono::EventHandler* event_handler)
    : m_event_handler(event_handler)
    , m_draw_debug_menu(false)
    , m_draw_debug_players(false)
{
    const event::KeyUpEventFunc key_up_func = [this](const event::KeyUpEvent& event) {
        if(event.key == Keycode::R)
            m_draw_debug_menu = !m_draw_debug_menu;
        else if(event.key == Keycode::P)
            m_draw_debug_players = !m_draw_debug_players;

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

    if(m_draw_debug_players)
        DrawDebugPlayers(m_draw_debug_players);
}

math::Quad game::DebugUpdater::BoundingBox() const
{
    return math::InfQuad;
}
