
#pragma once

#include <cstdint>
#include <vector>

namespace editor
{
    using Selection = std::vector<uint32_t>;

    class ISelectionHandler
    {
    public:

        virtual ~ISelectionHandler() = default;

        virtual void SetSelection(const Selection& selected_ids) = 0;
        virtual const Selection& GetSelection() const = 0;
        virtual void AddToSelection(const Selection& selected_ids) = 0;
        virtual void RemoveFromSelection(const Selection& selected_ids) = 0;
        virtual void ClearSelection() = 0;
    };
}
