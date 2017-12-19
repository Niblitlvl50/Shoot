
#pragma once

#include "MonoPtrFwd.h"
#include "Engine.h"
#include <unordered_map>

namespace game
{
    class ZoneManager
    {
    public:

        ZoneManager(System::IWindow* window, const mono::ICameraPtr& camera, mono::EventHandler& eventHandler);
        void Run();

    private:

        mono::Engine m_engine;
        mono::EventHandler& m_event_handler;

        using LoadFunction = mono::IZonePtr(*)(mono::EventHandler& event_handler);
        std::unordered_map<int, LoadFunction> m_zones;
    };
}
