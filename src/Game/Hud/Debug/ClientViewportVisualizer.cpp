
#include "ClientViewportVisualizer.h"
#include "Network/ServerManager.h"

#include "Math/Quad.h"
#include "Rendering/Color.h"
#include "Rendering/IRenderer.h"
#include "System/Network.h"
#include "Debug/GameDebug.h"

using namespace game;

ClientViewportVisualizer::ClientViewportVisualizer(const std::unordered_map<network::Address, ClientData>& client_data)
    : m_client_data(client_data)
{ }

void ClientViewportVisualizer::Draw(mono::IRenderer& renderer) const
{
    if(!game::g_draw_client_viewport)
        return;

    for(const auto& client_data : m_client_data)
    {
        //const std::string& address = network::AddressToString(client_data.first);
        renderer.DrawQuad(client_data.second.viewport, mono::Color::RED, 2.0f);
    }
}

math::Quad ClientViewportVisualizer::BoundingBox() const
{
    return math::InfQuad;
}
