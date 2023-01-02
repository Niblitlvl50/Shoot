
#include "UIDecorators.h"
#include "UIContext.h"
#include "Entity/Component.h"

#include "Particle/ParticleSystem.h"
#include "SystemContext.h"

#include "Rendering/Texture/ITexture.h"
#include "Rendering/Texture/ITextureFactory.h"
#include "Rendering/RenderSystem.h"

#include "imgui/imgui.h"

bool editor::AreaEmitterFooter(const editor::UIContext& ui_context, uint32_t component_index, Component& component)
{
    ImGui::Spacing();

    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
    ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor(70, 70, 70));

    const bool restart_emitter = ImGui::Button("Restart");
    if(restart_emitter)
    {
        //mono::ParticleSystem* particle_system = ui_context.system_context->GetSystem<mono::ParticleSystem>();
        //particle_system->Ge
    }

    ImGui::PopStyleVar();
    ImGui::PopStyleColor();

    return restart_emitter;
}

bool editor::TexturedPolygonFooter(const UIContext& ui_context, uint32_t component_index, Component& component)
{
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
    ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor(70, 70, 70));

    const bool match_texture_size = ImGui::Button("Match Texture");

    ImGui::PopStyleVar();
    ImGui::PopStyleColor();

    if(match_texture_size)
    {
        std::string texture_name;
        FindAttribute(TEXTURE_ATTRIBUTE, component.properties, texture_name, FallbackMode::SET_DEFAULT);
        if(!texture_name.empty())
        {
            mono::ITexturePtr loaded_texture = mono::RenderSystem::GetTextureFactory()->CreateTexture(texture_name.c_str());
            const float half_width = (loaded_texture->Width() / 2.0f) / mono::RenderSystem::PixelsPerMeter();
            const float half_height = (loaded_texture->Height() / 2.0f) / mono::RenderSystem::PixelsPerMeter();

            const std::vector<math::Vector> vertices = {
                { -half_width, -half_height },
                { -half_width,  half_height },
                {  half_width,  half_height },
                {  half_width, -half_height },
            };

            SetAttribute(POLYGON_ATTRIBUTE, component.properties, vertices);
            return true;
        }
    }

    return false;
}
