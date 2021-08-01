
#pragma once

#include "IGameSystem.h"
#include <string>
#include <vector>

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
            for(uint32_t index = 0; index < m_active.size(); ++index)
            {
                const bool is_active = m_active[index];
                if(is_active)
                    callable(index, m_components[index]);
            }
        }

    private:
    
        std::vector<DialogComponent> m_components;
        std::vector<bool> m_active;
    };
}
