
#pragma once

#include <MonoPtrFwd.h>
#include <Math/MathFwd.h>
#include <vector>
#include <functional>

struct Attribute;

namespace System
{
    struct ControllerState;
}

namespace game
{
    using DestroyedFunction = std::function<void (unsigned int id)>;

    class IGameObjectFactory
    {
    public:

        virtual ~IGameObjectFactory()
        { }

        virtual mono::IPhysicsEntityPtr CreateShuttle(
            const math::Vector& position, const System::ControllerState& gamepad, DestroyedFunction callback) const = 0;
        virtual mono::IPhysicsEntityPtr CreateGameObject(const char* name, const std::vector<Attribute>& attributes) const = 0;
    };
}
