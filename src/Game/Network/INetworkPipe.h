
#pragma once

#include "ConnectionStats.h"
#include <vector>
#include <string>

namespace network
{
    struct Address;
}

namespace game
{
    struct NetworkMessage;

    struct ConnectionInfo
    {
        ConnectionStats stats;
        std::vector<std::string> additional_info;
    };

    class INetworkPipe
    {
    public:

        virtual ~INetworkPipe() = default;
        virtual void SendMessage(const NetworkMessage& message) = 0;
        virtual void SendMessageTo(const NetworkMessage& message, const network::Address& address) = 0;
        virtual ConnectionInfo GetConnectionInfo() const = 0;
    };
}
