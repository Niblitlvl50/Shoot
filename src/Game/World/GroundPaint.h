
#pragma once

#include "WorldFile.h"
#include "Math/Vector.h"
#include "Math/Quad.h"
#include "Rendering/IDrawable.h"
#include "Rendering/RenderBuffer/IRenderBuffer.h"
#include "Rendering/Texture/ITextureFactory.h"

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace game
{
    // res/worlds/level.components gives res/worlds/level.splat.png
    std::string GroundPaintFilename(const std::string& world_filename);

    // A size to paint on.
    bool IsGroundPaintEnabled(const GroundPaintSettings& settings);

    // The splat map, RGBA weights of the four painted layers over the painted area. Rows go from
    // the bottom of the area to the top.
    class GroundPaintData
    {
    public:

        // Sized for the settings, with the paint from the file stretched over the area if there is one.
        void Load(const std::string& filename, const GroundPaintSettings& settings);
        bool Save(const std::string& filename) const;

        // Sized for new settings, keeping the paint where it is in the world.
        void Resize(const GroundPaintSettings& settings);
        bool MatchesSettings(const GroundPaintSettings& settings) const;

        // Paints layer 1 to 4, layer 0 erases back to the base layer. Strength is 0 to 1.
        void Paint(const math::Vector& world_position, float radius, float strength, int layer);

        // The layer with the most weight at the position, 0 is the base layer. Outside the area it's the base.
        int StrongestLayer(const math::Vector& world_position) const;

        int Width() const;
        int Height() const;
        const std::vector<uint8_t>& Pixels() const;

        // Set when the pixels change, the drawer clears it when it has uploaded them.
        bool m_dirty = true;

    private:

        math::Vector m_position;
        math::Vector m_size;
        float m_texels_per_meter = 0.0f;
        int m_width = 0;
        int m_height = 0;
        std::vector<uint8_t> m_pixels;
    };

    class GroundPaintDrawer : public mono::IDrawable
    {
    public:

        // Both are read every frame, so changes in the editor show up right away.
        GroundPaintDrawer(const GroundPaintSettings& settings, std::shared_ptr<GroundPaintData> data);

        void Draw(mono::IRenderer& renderer) const override;
        math::Quad BoundingBox() const override;

    private:

        const GroundPaintSettings& m_settings;
        std::shared_ptr<GroundPaintData> m_data;

        mutable std::string m_texture_names[GROUND_PAINT_LAYERS];
        mutable mono::ITexturePtr m_textures[GROUND_PAINT_LAYERS];
        mutable mono::ITexturePtr m_splat_texture;

        mutable math::Quad m_quad;
        mutable std::unique_ptr<mono::IRenderBuffer> m_vertices;
        mutable std::unique_ptr<mono::IElementBuffer> m_indices;
    };
}
