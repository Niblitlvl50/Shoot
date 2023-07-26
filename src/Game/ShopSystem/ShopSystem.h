
#pragma once

#include "IGameSystem.h"
#include <vector>
#include <string>

namespace game
{
    struct ItemComponent
    {
        std::string name;
        int cost;
    };

    class ShopSystem : public mono::IGameSystem
    {
    public:

        ShopSystem();
        const char* Name() const override;
        void Update(const mono::UpdateContext& update_context) override;

        const std::vector<ItemComponent>& GetItems() const;

    private:

        std::vector<ItemComponent> m_items;
    };
}
