
#include "FogOverlay.h"
#include "Math/Quad.h"
#include "Rendering/IRenderer.h"
#include "Rendering/RenderSystem.h"
#include "Rendering/RenderBuffer/BufferFactory.h"

using namespace game;

FogOverlay::FogOverlay()
{
    m_texture = mono::GetTextureFactory()->CreateTexture("res/textures/noise/noise_1.png");

    const math::Vector vertices[] = {
        math::Vector(-0.5f, -0.5f),
        math::Vector(-0.5f,  0.5f),
        math::Vector( 0.5f,  0.5f),
        math::Vector( 0.5f, -0.5f),
    };

    constexpr uint16_t indices[] = { 0, 1, 2,   0, 2, 3 };

    m_vertex_buffer = mono::CreateRenderBuffer(mono::BufferType::STATIC, mono::BufferData::FLOAT, 2, std::size(vertices), vertices);
    m_index_buffer = mono::CreateElementBuffer(mono::BufferType::STATIC, std::size(indices), indices);
}

FogOverlay::~FogOverlay()
{ }

void FogOverlay::Draw(mono::IRenderer& renderer) const
{
    renderer.DrawFog(m_vertex_buffer.get(), m_index_buffer.get(), m_texture.get());
}

void FogOverlay::Update(const mono::UpdateContext& context)
{

}

math::Quad FogOverlay::BoundingBox() const
{
    return math::InfQuad;
}
