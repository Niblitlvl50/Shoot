
#pragma once

#include "Rendering/IDrawable.h"

namespace game
{
    class DialogSystemDrawer : public mono::IDrawable
    {
    public:

        DialogSystemDrawer(class DialogSystem* message_system);

        void Draw(mono::IRenderer& renderer) const override;
        math::Quad BoundingBox() const override;

    private:

        DialogSystem* m_message_system;
    };
}
