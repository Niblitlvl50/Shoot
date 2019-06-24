
#pragma once

namespace game
{
    enum class ClientStatus
    {
        DISCONNECTED,
        SEARCHING,
        FOUND_SERVER,
        CONNECTED,
        FAILED
    };

    inline const char* ClientStatusToString(ClientStatus status)
    {
        switch(status)
        {
        case ClientStatus::DISCONNECTED:
            return "Disconnected";
        case ClientStatus::SEARCHING:
            return "Searching";
        case ClientStatus::FOUND_SERVER:
            return "Found Server";
        case ClientStatus::CONNECTED:
            return "Connected";
        case ClientStatus::FAILED:
            return "Failed";
        }
    };
}
