
#include "FPSElement.h"
#include "GameDebug.h"
#include "Math/Quad.h"

#include "imgui/imgui.h"

using namespace game;

void FPSElement::Draw(mono::IRenderer& renderer) const
{
    m_counter++;

    if(!game::g_draw_fps)
        return;

    constexpr int flags =
        ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoResize;

    ImGui::Begin("fps_counter", &game::g_draw_fps, flags);
    ImGui::Text("fps: %u frames: %u", m_counter.Fps(), m_counter.Frames());
    ImGui::End();
}

math::Quad FPSElement::BoundingBox() const
{
    return math::InfQuad;
}
