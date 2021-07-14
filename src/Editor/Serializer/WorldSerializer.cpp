
#include "WorldSerializer.h"
#include "ObjectProxies/ComponentProxy.h"
#include "Serializer/JsonSerializer.h"

#include "EntitySystem/Entity.h"
#include "WorldFile.h"


editor::World editor::LoadWorld(
    const char* file_name, mono::IEntityManager* entity_manager, mono::TransformSystem* transform_system, Editor* editor)
{
    editor::World world;

    const auto entity_callback =
        [&world, entity_manager, transform_system, editor]
        (const mono::Entity& entity, const std::string& folder, const std::vector<Component>& components)
    {
        auto component_proxy = std::make_unique<ComponentProxy>(entity.id, components, entity_manager, transform_system, editor);
        component_proxy->SetEntityProperties(entity.properties);

        world.loaded_proxies.push_back(std::move(component_proxy));
    };

    world.leveldata = shared::ReadWorldComponentObjects(file_name, entity_manager, entity_callback);
    return world;
}

void editor::SaveWorld(const char* file_name, const std::vector<IObjectProxyPtr>& proxies, const shared::LevelMetadata& level_data)
{
    JsonSerializer serializer;

    for(auto& proxy : proxies)
        proxy->Visit(serializer);

    serializer.WriteComponentEntities(file_name, level_data);
}
