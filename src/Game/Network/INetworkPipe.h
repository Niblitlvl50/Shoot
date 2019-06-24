
#pragma once

namespace network
{
    struct Address;
}

namespace game
{
    struct NetworkMessage;

    class INetworkPipe
    {
    public:

        virtual ~INetworkPipe() = default;
        virtual void SendMessage(const NetworkMessage& message) = 0;
        virtual void SendMessageTo(const NetworkMessage& message, const network::Address& address) = 0;
    };
}
