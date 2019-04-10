
#pragma once

//
// Game1 - Looks for beacon, finds nothing
// Game1 - Starts game, enables beacon
// Game2 - Looks for beacon find Game1
// Game2 - Connect to Game1
// Game1 - Disable beacon
//

#include <thread>

namespace game
{
    class NetworkBeacon
    {
    public:
    
        NetworkBeacon(int beacon_port);
        ~NetworkBeacon();

        void Start();
        void Stop();

    private:
    
        int m_beacon_port;
        bool m_broadcast;
        std::thread m_broadcast_thread;
    };
}
