
#include "NetworkStatusDrawer.h"
#include "Network/INetworkPipe.h"
#include "Network/ConnectionStats.h"

#include "Math/Quad.h"
#include "Debug/GameDebug.h"

#include "imgui/imgui.h"

using namespace game;

NetworkStatusDrawer::NetworkStatusDrawer(const INetworkPipe* network_pipe)
    : m_network_pipe(network_pipe)
{ }

void NetworkStatusDrawer::Draw(mono::IRenderer& renderer) const
{
    if(!game::g_draw_network_stats)
        return;

    const ConnectionInfo& info = m_network_pipe->GetConnectionInfo();
    const ConnectionStats& stats = info.stats;

    const float mb_sent = float(stats.total_compressed_byte_sent) / 100'000;
    const float mb_received = float(stats.total_compressed_byte_received) / 100'000;
    const float kb_sent_per_frame = float(stats.total_compressed_byte_sent) / 100.0f / float(m_total_frame_count);
    const float kb_received_per_frame = float(stats.total_compressed_byte_received) / 100.0f / float(m_total_frame_count);

    const float compression_rate = (1.0f - float(stats.total_compressed_byte_sent) / float(stats.total_byte_sent)) * 100.0f;

    constexpr int flags =
        ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoResize;

    const bool window_open = ImGui::Begin("Network Status", nullptr, flags);
    if(window_open)
    {
        ImGui::Text("packages: %u/%u", stats.total_packages_sent, stats.total_packages_received);
        ImGui::Text("total: %.1fmb / %.1fmb", mb_sent, mb_received);
        ImGui::Text("frame: %.1fkb / %.1fkb", kb_sent_per_frame, kb_received_per_frame);
        ImGui::Text("compression rate: %.1f%%", compression_rate);

        for(const std::string& additional_text : info.additional_info)
            ImGui::Text("%s", additional_text.c_str());
    }

    ImGui::End();
}

math::Quad NetworkStatusDrawer::BoundingBox() const
{
    return math::InfQuad;
}
