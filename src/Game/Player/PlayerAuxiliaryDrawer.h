
#pragma once

#include "MonoFwd.h"
#include "Rendering/IDrawable.h"
#include "Rendering/RenderBuffer/IRenderBuffer.h"
#include "Rendering/Sprite/ISpriteFactory.h"
#include "Rendering/Sprite/SpriteBufferFactory.h"
#include "Rendering/Primitives/PrimitiveBufferFactory.h"

#include "Player/PlayerInfo.h"

#include <memory>
#include <vector>
#include <unordered_map>

namespace game
{
    class CameraSystem;
    class WeaponSystem;

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

    struct AimlineRenderData
    {
        std::unique_ptr<mono::IRenderBuffer> vertices;
        std::unique_ptr<mono::IRenderBuffer> colors;
        std::unique_ptr<mono::IElementBuffer> indices;
    };

    class PlayerAuxiliaryDrawer : public mono::IDrawable
    {
    public:

        PlayerAuxiliaryDrawer(const game::CameraSystem* camera_system, const game::WeaponSystem* weapon_system, const mono::TransformSystem* transform_system);

        void Draw(mono::IRenderer& renderer) const override;
        math::Quad BoundingBox() const override;

        void DrawLasers(const std::vector<const game::PlayerInfo*>& players, mono::IRenderer& renderer) const;
        void DrawStaminaBar(const std::vector<const game::PlayerInfo*>& players, mono::IRenderer& renderer) const;
        void DrawAbilities(const std::vector<const game::PlayerInfo*>& players, mono::IRenderer& renderer) const;
        void DrawPowerups(const std::vector<const game::PlayerInfo*>& players, mono::IRenderer& renderer) const;
        void DrawCrosshair(const std::vector<const game::PlayerInfo*>& players, mono::IRenderer& renderer) const;

        const game::CameraSystem* m_camera_system;
        const game::WeaponSystem* m_weapon_system;
        const mono::TransformSystem* m_transform_system;

        std::vector<AbilityRenderData> m_ability_render_datas;
        std::unordered_map<uint32_t, AbilityRenderData> m_powerup_render_datas;
        AbilityRenderData m_crosshair_render_data;
        mutable AimlineRenderData m_aimline_render_data;
        mutable mono::PrimitiveDrawBuffers m_aimline_targets_render_data;
        std::unique_ptr<mono::IElementBuffer> m_indices;

        mutable AbilityInstanceData m_ability_data[n_players];
    };

    class PackageAuxiliaryDrawer : public mono::IDrawable
    {
    public:

        PackageAuxiliaryDrawer(const mono::TransformSystem* transform_system);
        void Draw(mono::IRenderer& renderer) const override;
        math::Quad BoundingBox() const override;

        const mono::TransformSystem* m_transform_system;
    };  
}
