
#include "NetworkStatusDrawer.h"
#include "Network/ServerManager.h"
#include "Network/ClientManager.h"
#include "Network/ConnectionStats.h"
#include "FontIds.h"

#include "Rendering/IRenderer.h"
#include "Rendering/Color.h"
#include "System/Network.h"
#include "Util/Algorithm.h"

using namespace game;

ServerStatusDrawer::ServerStatusDrawer(const math::Vector& position, const class ServerManager* server_manager)
    : m_server_manager(server_manager)
{
    m_position = position;
}

void ServerStatusDrawer::Draw(mono::IRenderer& renderer) const
{
    const ConnectionStats& stats = m_server_manager->GetConnectionStats();

    const float mb_sent = float(stats.total_compressed_byte_sent) / 100'000;
    const float mb_received = float(stats.total_compressed_byte_received) / 100'000;
    const float kb_sent_per_frame = float(stats.total_compressed_byte_sent) / 100 / float(m_total_frame_count);
    const float kb_received_per_frame = float(stats.total_compressed_byte_received) / 100 / float(m_total_frame_count);

    const float compression_rate = (1.0f - float(stats.total_compressed_byte_sent) / float(stats.total_byte_sent)) * 100.0f;

    char text_buffer[256] = { '\0' };
    std::snprintf(text_buffer, std::size(text_buffer), "packages: %u/%u", stats.total_packages_sent, stats.total_packages_received);
    renderer.DrawText(FontId::PIXELETTE_MEGA, text_buffer, math::Vector(210.0f, 0.0f), false, mono::Color::BLACK);

    std::memset(text_buffer, 0, std::size(text_buffer));
    std::snprintf(text_buffer, std::size(text_buffer), "total: %.1fmb / %.1fmb", mb_sent, mb_received);
    renderer.DrawText(FontId::PIXELETTE_MEGA, text_buffer, math::Vector(210.0f, -5.0f), false, mono::Color::BLACK);

    std::memset(text_buffer, 0, std::size(text_buffer));
    std::snprintf(text_buffer, std::size(text_buffer), "frame: %.1fkb / %.1fkb", kb_sent_per_frame, kb_received_per_frame);
    renderer.DrawText(FontId::PIXELETTE_MEGA, text_buffer, math::Vector(210.0f, -10.0f), false, mono::Color::BLACK);

    std::memset(text_buffer, 0, std::size(text_buffer));
    std::snprintf(text_buffer, std::size(text_buffer), "compression rate: %.1f%%", compression_rate);
    renderer.DrawText(FontId::PIXELETTE_MEGA, text_buffer, math::Vector(210.0f, -15.0f), false, mono::Color::BLACK);

    float y = -25.0f;

    for(const ClientData& client : m_server_manager->GetConnectedClients())
    {
        const std::string& address = network::AddressToString(client.address);
        renderer.DrawText(FontId::PIXELETTE_MEGA, address.c_str(), math::Vector(210.0f, y), false, mono::Color::BLACK);
        y -= 5.0f;
    }
}

void ServerStatusDrawer::Update(const mono::UpdateContext& context)
{
    m_total_frame_count = context.frame_count;
}


ClientStatusDrawer::ClientStatusDrawer(const math::Vector& position, const ClientManager* client_manager)
    : m_client_manager(client_manager)
{
    m_position = position;
}

void ClientStatusDrawer::Draw(mono::IRenderer& renderer) const
{
    const char* client_status = ClientStatusToString(m_client_manager->GetConnectionStatus());
    renderer.DrawText(FontId::PIXELETTE_MEGA, client_status, math::Vector(230.0f, 0.0f), false, mono::Color::BLACK);

    const ConnectionStats& stats = m_client_manager->GetConnectionStats();
    const float mb_sent = float(stats.total_byte_sent) / 100000.0f;
    const float mb_received = float(stats.total_byte_received) / 100000.0f;

    char text_buffer[256] = { '\0' };
    std::snprintf(text_buffer, std::size(text_buffer), "%u/%u", stats.total_packages_sent, stats.total_packages_received);
    renderer.DrawText(FontId::PIXELETTE_MEGA, text_buffer, math::Vector(230.0f, 5.0f), false, mono::Color::BLACK);

    std::memset(text_buffer, 0, std::size(text_buffer));
    std::snprintf(text_buffer, std::size(text_buffer), "%.2fmb / %.2fmb", mb_sent, mb_received);
    renderer.DrawText(FontId::PIXELETTE_MEGA, text_buffer, math::Vector(230.0f, 10.0f), false, mono::Color::BLACK);
}

void ClientStatusDrawer::Update(const mono::UpdateContext& context)
{
    m_total_frame_count = context.frame_count;
}