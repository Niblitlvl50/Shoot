
#pragma once

#include "MonoFwd.h"
#include "IGameSystem.h"

#include <cstdint>
#include <string>

namespace game
{
    enum class AnnotationCorner
    {
        TopRight,
        TopLeft,
        BottomRight,
        BottomLeft,
    };

    class EntityAnnotationSystem : public mono::IGameSystem
    {
    public:

        EntityAnnotationSystem(mono::TransformSystem* transform_system, mono::IEntityManager* entity_manager);

        uint32_t AddAnnotation(uint32_t entity_id, const std::string& entity_file, AnnotationCorner corner);
        void RemoveAnnotation(uint32_t annotation_entity_id);

        const char* Name() const override;
        void Update(const mono::UpdateContext& update_context) override;

    private:

        mono::TransformSystem* m_transform_system;
        mono::IEntityManager* m_entity_manager;
    };
}
