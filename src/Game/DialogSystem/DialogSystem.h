
#pragma once

#include "IGameSystem.h"
#include "Util/ActiveVector.h"
#include <string>

namespace game
{
    struct DialogComponent
    {
        std::string message;
        float duration;
    };

    class DialogSystem : public mono::IGameSystem
    {
    public:

        DialogSystem(uint32_t n);

        DialogComponent* AllocateComponent(uint32_t entity_id);
        void ReleaseComponent(uint32_t entity_id);
        void AddComponent(uint32_t entity_id, const std::string& message, float duration);

        uint32_t Id() const override;
        const char* Name() const override;
        void Update(const mono::UpdateContext& update_context) override;

        template <typename T>
        inline void ForEach(T&& callable)
        {
            m_components.ForEach(callable);
        }

    private:
    
        mono::ActiveVector<DialogComponent> m_components;
    };
}
