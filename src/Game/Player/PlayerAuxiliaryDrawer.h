
#pragma once

#include "MonoFwd.h"
#include "Rendering/IDrawable.h"
#include "Rendering/RenderBuffer/IRenderBuffer.h"
#include "Rendering/Sprite/ISpriteFactory.h"
#include "Rendering/Sprite/SpriteBufferFactory.h"

#include "Player/PlayerInfo.h"

#include <memory>

namespace game
{
    class CameraSystem;

    struct AimlineRenderData
    {
        std::unique_ptr<mono::IRenderBuffer> vertices;
        std::unique_ptr<mono::IRenderBuffer> colors;
        std::unique_ptr<mono::IElementBuffer> indices;
    };

    struct AbilityInstanceData
    {
        float cooldown_position;
        float cooldown_velocity;

        float last_cooldown_fraction;
        uint32_t timestamp;
    };

    struct AbilityRenderData
    {
        mono::ISpritePtr sprite;
        mono::SpriteDrawBuffers sprite_buffers;
    };

    class PlayerAuxiliaryDrawer : public mono::IDrawable
    {
    public:

        PlayerAuxiliaryDrawer(const game::CameraSystem* camera_system, const mono::TransformSystem* transform_system);

        void Draw(mono::IRenderer& renderer) const override;
        math::Quad BoundingBox() const override;

        const game::CameraSystem* m_camera_system;
        const mono::TransformSystem* m_transform_system;

        std::vector<AbilityRenderData> m_ability_render_datas;
        AbilityRenderData m_crosshair_render_data;
        std::unique_ptr<mono::IElementBuffer> m_indices;

        mutable AimlineRenderData m_aimline_data[n_players];
        mutable AbilityInstanceData m_ability_data[n_players];
    };

    class PackageAuxiliaryDrawer : public mono::IDrawable
    {
    public:

        PackageAuxiliaryDrawer(const mono::TransformSystem* transform_system);
        void Draw(mono::IRenderer& renderer) const override;
        math::Quad BoundingBox() const override;

        const mono::TransformSystem* m_transform_system;
        mono::ISpritePtr m_package_sprite;
        mono::SpriteDrawBuffers m_sprite_buffers;
        std::unique_ptr<mono::IElementBuffer> m_indices;
    };
}
