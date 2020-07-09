
#include "StaticBackground.h"

#include "Math/MathFunctions.h"
#include "Math/Quad.h"
#include "Rendering/BufferFactory.h"
#include "Rendering/IRenderer.h"
#include "Rendering/Texture/ITextureFactory.h"

#include <vector>


StaticBackground::StaticBackground()
{
    const int vertex_count = 4;

    m_texture = mono::GetTextureFactory()->CreateTexture("res/textures/white_box_placeholder.png");
    m_vertex_buffer = mono::CreateRenderBuffer(mono::BufferTarget::ARRAY_BUFFER, mono::BufferType::STATIC, mono::BufferData::FLOAT, vertex_count * 2);
    m_texture_buffer = mono::CreateRenderBuffer(mono::BufferTarget::ARRAY_BUFFER, mono::BufferType::STATIC, mono::BufferData::FLOAT, vertex_count * 2); 

    constexpr math::Quad background_bb = math::Quad(-100.0f, -100.0f, 100.0f, 100.0f);

    const std::vector<math::Vector> vertices = {
        math::BottomLeft(background_bb),
        math::TopLeft(background_bb),
        math::TopRight(background_bb),
        math::BottomRight(background_bb)
    };

    std::vector<math::Vector> texture_coordinates;
    texture_coordinates.reserve(vertices.size());

    //const math::Vector& repeate = (background_bb.mB - background_bb.mA) / 8.0f;
    const math::Vector repeate(20.0f, 20.0f);
        
    for(const math::Vector& vertex : vertices)
        texture_coordinates.push_back(math::MapVectorInQuad(vertex, background_bb) * repeate);

    m_vertex_buffer->UpdateData(vertices.data(), 0, vertex_count * 2);
    m_texture_buffer->UpdateData(texture_coordinates.data(), 0, vertex_count * 2);
}

void StaticBackground::Draw(mono::IRenderer& renderer) const
{
    renderer.DrawGeometry(m_vertex_buffer.get(), m_texture_buffer.get(), 0, 4, m_texture.get());
}

math::Quad StaticBackground::BoundingBox() const
{
    return math::InfQuad;
}
