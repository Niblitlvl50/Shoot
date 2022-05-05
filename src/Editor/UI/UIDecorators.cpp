
#include "UIDecorators.h"
#include "UIContext.h"

#include "Particle/ParticleSystem.h"
#include "SystemContext.h"

#include "imgui/imgui.h"

bool editor::AreaEmitterFooter(const editor::UIContext& ui_context, uint32_t component_index, Component& component)
{
    ImGui::Spacing();
    const bool restart_emitter = ImGui::Button("Restart");
    if(restart_emitter)
    {
        mono::ParticleSystem* particle_system = ui_context.system_context->GetSystem<mono::ParticleSystem>();
        //particle_system->Ge
    }

    return restart_emitter;
}
