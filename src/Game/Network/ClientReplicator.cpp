
#include "ClientReplicator.h"
#include "ClientManager.h"
#include "INetworkPipe.h"
#include "NetworkMessage.h"

#include "Math/MathFunctions.h"
#include "Camera/ICamera.h"

using namespace game;

ClientReplicator::ClientReplicator(mono::ICamera* camera, ClientManager* remote_connection)
    : m_camera(camera)
    , m_remote_connection(remote_connection)
    , m_replicate_timer_s(0.0f)
{ }

void ClientReplicator::Update(const mono::UpdateContext& update_context)
{
    const ClientStatus client_status = m_remote_connection->GetConnectionStatus();
    if(client_status != ClientStatus::CONNECTED)
        return;

    RemoteInputMessage remote_input;
    remote_input.controller_state = System::GetController(System::ControllerId::Primary);

    {
        NetworkMessage message;
        message.payload = SerializeMessage(remote_input);
        m_remote_connection->SendMessage(message);
    }

    m_replicate_timer_s += update_context.delta_s;
    if(m_replicate_timer_s > 0.016f)
    {
        RemoteCameraMessage camera_message;
        camera_message.position = m_camera->GetPosition();
        camera_message.viewport = m_camera->GetViewport();

        NetworkMessage message;
        message.payload = SerializeMessage(camera_message);
        m_remote_connection->SendMessage(message);

        ViewportMessage viewport_message;
        viewport_message.sender = m_remote_connection->GetClientAddress();
        viewport_message.viewport = m_camera->GetViewport();

        NetworkMessage message2;
        message2.payload = SerializeMessage(viewport_message);
        m_remote_connection->SendMessage(message2);

        m_replicate_timer_s = 0.0f;
    }
}
