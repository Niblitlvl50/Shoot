
#include "NetworkStatusDrawer.h"
#include "Network/ServerManager.h"
#include "Network/ClientManager.h"
#include "FontIds.h"

#include "Rendering/IRenderer.h"
#include "System/Network.h"

using namespace game;

ServerStatusDrawer::ServerStatusDrawer(const math::Vector& position, const class ServerManager* server_manager)
    : m_server_manager(server_manager)
{
    m_position = position;
}

void ServerStatusDrawer::Draw(mono::IRenderer& renderer) const
{
    float y = 0.0f;

    for(const ClientData& client : m_server_manager->GetConnectedClients())
    {
        const std::string& address = network::AddressToString(client.address);
        renderer.DrawText(FontId::PIXELETTE_MEGA, address.c_str(), math::Vector(250.0f, y), false, mono::Color::MAGENTA);
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
}

void ClientStatusDrawer::Update(const mono::UpdateContext& context)
{ }