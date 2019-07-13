
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
    const float mb_sent = float(stats.total_byte_sent) / 100000.0f;
    const float mb_received = float(stats.total_byte_received) / 100000.0f;

    char text_buffer[256] = { '\0' };
    std::snprintf(text_buffer, std::size(text_buffer), "%u/%u", stats.total_sent, stats.total_received);
    renderer.DrawText(FontId::PIXELETTE_MEGA, text_buffer, math::Vector(230.0f, 0.0f), false, mono::Color::BLACK);

    std::memset(text_buffer, 0, std::size(text_buffer));
    std::snprintf(text_buffer, std::size(text_buffer), "%.2fmb / %.2fmb", mb_sent, mb_received);
    renderer.DrawText(FontId::PIXELETTE_MEGA, text_buffer, math::Vector(230.0f, -5.0f), false, mono::Color::BLACK);

    float y = -10.0f;

    for(const ClientData& client : m_server_manager->GetConnectedClients())
    {
        const std::string& address = network::AddressToString(client.address);
        renderer.DrawText(FontId::PIXELETTE_MEGA, address.c_str(), math::Vector(230.0f, y), false, mono::Color::BLACK);
        y -= 5.0f;
    }
}

void ServerStatusDrawer::Update(const mono::UpdateContext& context)
{ }


ClientStatusDrawer::ClientStatusDrawer(const math::Vector& position, const ClientManager* client_manager)
    : m_client_manager(client_manager)
{
    m_position = position;
}

void ClientStatusDrawer::Draw(mono::IRenderer& renderer) const
{
    const char* client_status = ClientStatusToString(m_client_manager->GetConnectionStatus());
    renderer.DrawText(FontId::PIXELETTE_MEGA, client_status, math::Vector(250.0f, 0.0f), false, mono::Color::MAGENTA);

    const ConnectionStats& stats = m_client_manager->GetConnectionStats();

    char text_buffer[256] = { '\0' };
    std::snprintf(
        text_buffer, mono::arraysize(text_buffer), "%u/%u", stats.total_sent, stats.total_received);
    renderer.DrawText(FontId::PIXELETTE_MEGA, text_buffer, math::Vector(250.0f, 5.0f), false, mono::Color::MAGENTA);
}

void ClientStatusDrawer::Update(const mono::UpdateContext& context)
{ }