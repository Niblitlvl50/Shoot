
#pragma once

#include "Rendering/ICamera.h"
#include "MonoFwd.h"
#include "Math/Quad.h"

namespace game
{
    class Camera : public mono::ICamera
    {
    public:

        Camera(int width, int height);
        
        virtual void doUpdate(unsigned int delta);
        
        virtual void Follow(const mono::IEntityPtr& entity, const math::Vector& offset);
        virtual void Unfollow();

        virtual math::Quad GetViewport() const;
        virtual math::Vector GetPosition() const;

        virtual void SetViewport(const math::Quad& viewport);
        virtual void SetTargetViewport(const math::Quad& target);
        virtual void SetPosition(const math::Vector& position);

        virtual math::Vector ScreenToWorld(const math::Vector& screen_pos, const math::Vector& window_size) const;

    private:
        
        mono::IEntityPtr m_entity;
        math::Vector m_offset;
        math::Quad m_viewport;
        math::Quad m_targetViewport;
    };
}
