
#include "EditorServer.h"
#include "HttpListener.h"
#include "Editor.h"
#include "ObjectProxies/IObjectProxy.h"

#include "Entity/Component.h"
#include "EntitySystem/Serialize.h"
#include "Math/Quad.h"
#include "Math/Serialize.h"
#include "System/System.h"

#include "nlohmann/json.hpp"

#include <chrono>
#include <deque>
#include <future>
#include <mutex>
#include <string>

namespace
{
    constexpr std::chrono::seconds request_timeout = std::chrono::seconds(5);

    using ServerRequest = editor::HttpRequest;

    struct ServerResponse
    {
        int status = 200;
        nlohmann::json body;
    };

    struct PendingRequest
    {
        ServerRequest request;
        std::promise<ServerResponse> promise;
    };

    ServerResponse MakeError(int status, const std::string& message)
    {
        ServerResponse response;
        response.status = status;
        response.body["error"] = message;
        return response;
    }

    nlohmann::json SerializeComponents(const std::vector<Component>& components)
    {
        nlohmann::json json_components = nlohmann::json::array();

        for(const Component& component : components)
        {
            nlohmann::json json_properties = nlohmann::json::array();
            for(const Attribute& property : component.properties)
                json_properties.push_back(property);

            nlohmann::json json_component;
            json_component["hash"] = component.hash;
            json_component["name"] = component::ComponentNameFromHash(component.hash);
            json_component["properties"] = json_properties;
            json_components.push_back(json_component);
        }

        return json_components;
    }

    nlohmann::json SerializeBounds(const math::Quad& bounds)
    {
        nlohmann::json json_bounds;
        json_bounds["bottom_left"] = bounds.bottom_left;
        json_bounds["top_right"] = bounds.top_right;
        return json_bounds;
    }

    nlohmann::json SerializeEntitySummary(const editor::IObjectProxy* proxy)
    {
        nlohmann::json component_names = nlohmann::json::array();
        for(const Component& component : proxy->GetComponents())
            component_names.push_back(component::ComponentNameFromHash(component.hash));

        nlohmann::json json_entity;
        json_entity["id"] = proxy->Id();
        json_entity["uuid_hash"] = proxy->Uuid();
        json_entity["name"] = proxy->Name();
        json_entity["folder"] = proxy->GetFolder();
        json_entity["locked"] = proxy->IsLocked();
        json_entity["position"] = proxy->GetPosition();
        json_entity["rotation"] = proxy->GetRotation();
        json_entity["components"] = component_names;
        return json_entity;
    }

    nlohmann::json SerializeEntity(const editor::IObjectProxy* proxy)
    {
        nlohmann::json json_entity;
        json_entity["id"] = proxy->Id();
        json_entity["uuid_hash"] = proxy->Uuid();
        json_entity["name"] = proxy->Name();
        json_entity["folder"] = proxy->GetFolder();
        json_entity["locked"] = proxy->IsLocked();
        json_entity["entity_properties"] = proxy->GetEntityProperties();
        json_entity["position"] = proxy->GetPosition();
        json_entity["rotation"] = proxy->GetRotation();
        json_entity["bounds"] = SerializeBounds(proxy->GetBoundingBox());
        json_entity["components"] = SerializeComponents(proxy->GetComponents());
        return json_entity;
    }

    ServerResponse HandleStatus(editor::Editor& editor)
    {
        ServerResponse response;
        response.body["world"] = editor.GetWorldFilename();
        response.body["entities"] = editor.GetProxies().size();
        response.body["selected"] = editor.GetSelection().size();
        return response;
    }

    ServerResponse HandleGetEntities(editor::Editor& editor)
    {
        nlohmann::json json_entities = nlohmann::json::array();
        for(const IObjectProxyPtr& proxy : editor.GetProxies())
            json_entities.push_back(SerializeEntitySummary(proxy.get()));

        ServerResponse response;
        response.body["entities"] = json_entities;
        return response;
    }

    ServerResponse HandleGetEntity(editor::Editor& editor, uint32_t entity_id)
    {
        const editor::IObjectProxy* proxy = editor.FindProxyObject(entity_id);
        if(!proxy)
            return MakeError(404, "No entity with id " + std::to_string(entity_id));

        ServerResponse response;
        response.body = SerializeEntity(proxy);
        return response;
    }

    ServerResponse HandleGetSelection(editor::Editor& editor)
    {
        nlohmann::json json_entities = nlohmann::json::array();
        math::Quad selection_bounds = math::InverseInfQuad;

        for(uint32_t entity_id : editor.GetSelection())
        {
            const editor::IObjectProxy* proxy = editor.FindProxyObject(entity_id);
            if(!proxy)
                continue;

            json_entities.push_back(SerializeEntity(proxy));
            math::ExpandBy(selection_bounds, proxy->GetBoundingBox());
        }

        ServerResponse response;
        response.body["entities"] = json_entities;
        if(!json_entities.empty())
            response.body["bounds"] = SerializeBounds(selection_bounds);

        return response;
    }

    ServerResponse HandleSetSelection(editor::Editor& editor, const std::string& body)
    {
        const nlohmann::json json = nlohmann::json::parse(body, nullptr, false);
        if(json.is_discarded() || !json.contains("ids") || !json["ids"].is_array())
            return MakeError(400, "Expected a body like { \"ids\": [ 1, 2, 3 ] }");

        editor::Selection new_selection;
        nlohmann::json unknown_ids = nlohmann::json::array();

        for(const nlohmann::json& json_id : json["ids"])
        {
            const uint32_t entity_id = json_id.get<uint32_t>();
            if(editor.FindProxyObject(entity_id))
                new_selection.push_back(entity_id);
            else
                unknown_ids.push_back(entity_id);
        }

        editor.SetSelection(new_selection);

        ServerResponse response;
        response.body["selected"] = new_selection;
        if(!unknown_ids.empty())
            response.body["unknown_ids"] = unknown_ids;

        return response;
    }

    ServerResponse HandleGetComponents()
    {
        nlohmann::json json_components = nlohmann::json::array();

        for(const Component* component : component::GetAllDefaultComponents())
        {
            nlohmann::json json_properties = nlohmann::json::array();
            for(const Attribute& property : component->properties)
                json_properties.push_back(property);

            nlohmann::json json_component;
            json_component["hash"] = component->hash;
            json_component["name"] = component::ComponentNameFromHash(component->hash);
            json_component["category"] = component->category;
            json_component["allow_multiple"] = component->allow_multiple;
            json_component["depends_on"] =
                (component->depends_on != NULL_COMPONENT) ? component::ComponentNameFromHash(component->depends_on) : "";
            json_component["properties"] = json_properties;
            json_components.push_back(json_component);
        }

        ServerResponse response;
        response.body["components"] = json_components;
        return response;
    }

    constexpr const char* g_variant_type_names[] = {
        "bool", "int", "uint", "float", "point", "color", "string", "polygon", "interval", "value_spread", "gradient_4", "event"
    };

    const char* VariantTypeName(size_t variant_index)
    {
        return (variant_index < std::size(g_variant_type_names)) ? g_variant_type_names[variant_index] : "unknown";
    }

    nlohmann::json AttributeValueToJson(const Attribute& attribute)
    {
        const nlohmann::json json_attribute = attribute;
        return json_attribute["variant_value"];
    }

    struct AttributeChange
    {
        Component* component;
        Attribute* attribute;
        Attribute new_value;
    };

    // Parses json_properties against the attributes of component, using each attribute's own type so a
    // value of the wrong type is an error instead of a type change. Returns an empty string on success.
    std::string ParseProperties(
        const Component& component, const nlohmann::json& json_properties, std::vector<Attribute>& out_attributes)
    {
        const std::string component_name = component::ComponentNameFromHash(component.hash);

        if(!json_properties.is_object())
            return "Component '" + component_name + "' needs a \"properties\" object";

        for(const auto& [attribute_name, json_value] : json_properties.items())
        {
            const Attribute* template_attribute = nullptr;
            for(const Attribute& attribute : component.properties)
            {
                if(attribute_name == mono::EntitySystem::AttributeNameLookup(attribute.id))
                {
                    template_attribute = &attribute;
                    break;
                }
            }

            if(!template_attribute)
                return "Component '" + component_name + "' has no attribute '" + attribute_name + "'";

            const size_t variant_index = template_attribute->value.index();
            nlohmann::json json_attribute;
            json_attribute["id"] = template_attribute->id;
            json_attribute["variant_type"] = variant_index;
            json_attribute["variant_value"] = json_value;

            try
            {
                out_attributes.push_back(json_attribute.get<Attribute>());
            }
            catch(const std::exception&)
            {
                return "Attribute '" + attribute_name + "' on '" + component_name + "' expects a " + VariantTypeName(variant_index);
            }
        }

        return std::string();
    }

    // Body: { "components": [ { "name": "spawn_point", "index": 0, "properties": { "spawn_limit": 5 } } ] }
    // index picks between several components with the same name and is optional. Every change is
    // checked before anything is applied, so a bad request changes nothing.
    ServerResponse HandlePatchEntity(editor::Editor& editor, uint32_t entity_id, const std::string& body)
    {
        editor::IObjectProxy* proxy = editor.FindProxyObject(entity_id);
        if(!proxy)
            return MakeError(404, "No entity with id " + std::to_string(entity_id));

        if(proxy->IsLocked())
            return MakeError(409, "Entity " + std::to_string(entity_id) + " is locked in the editor");

        const nlohmann::json json = nlohmann::json::parse(body, nullptr, false);
        if(json.is_discarded() || !json.contains("components") || !json["components"].is_array())
            return MakeError(400, "Expected a body like { \"components\": [ { \"name\": ..., \"properties\": { ... } } ] }");

        std::vector<Component>& components = proxy->GetComponents();
        std::vector<AttributeChange> changes;

        for(const nlohmann::json& json_component : json["components"])
        {
            const std::string component_name = json_component.value("name", "");
            const int component_index = json_component.value("index", 0);

            int found_index = 0;
            Component* target_component = nullptr;
            for(Component& component : components)
            {
                if(component_name != component::ComponentNameFromHash(component.hash))
                    continue;

                if(found_index == component_index)
                {
                    target_component = &component;
                    break;
                }

                ++found_index;
            }

            if(!target_component)
                return MakeError(400, "Entity has no component '" + component_name + "' at index " + std::to_string(component_index));

            std::vector<Attribute> new_values;
            const std::string parse_error = ParseProperties(*target_component, json_component.value("properties", nlohmann::json()), new_values);
            if(!parse_error.empty())
                return MakeError(400, parse_error);

            for(const Attribute& new_value : new_values)
            {
                Attribute* target_attribute = nullptr;
                FindAttribute(new_value.id, target_component->properties, target_attribute);
                changes.push_back({ target_component, target_attribute, new_value });
            }
        }

        nlohmann::json json_changes = nlohmann::json::array();

        for(AttributeChange& change : changes)
        {
            nlohmann::json json_change;
            json_change["component"] = component::ComponentNameFromHash(change.component->hash);
            json_change["attribute"] = mono::EntitySystem::AttributeNameLookup(change.attribute->id);
            json_change["old"] = AttributeValueToJson(*change.attribute);

            change.attribute->value = change.new_value.value;
            json_change["new"] = AttributeValueToJson(*change.attribute);

            // Same path as the property panel, so the entity and any grabbers update straight away.
            proxy->ComponentChanged(*change.component, change.attribute->id);
            editor.RegisterTriggers({ *change.attribute });
            json_changes.push_back(json_change);
        }

        ServerResponse response;
        response.body["id"] = entity_id;
        response.body["changes"] = json_changes;
        return response;
    }

    // Finds the registered component with this name, or nullptr.
    const Component* FindTemplateComponent(const std::string& component_name)
    {
        for(const Component* component : component::GetAllDefaultComponents())
        {
            if(component_name == component::ComponentNameFromHash(component->hash))
                return component;
        }

        return nullptr;
    }

    // Body: { "name": "mission_reward", "properties": { "reward_chips": 50 } }, properties is optional.
    // Components the new one depends on are added too if the entity doesn't have them, with default values.
    ServerResponse HandleAddComponent(editor::Editor& editor, uint32_t entity_id, const std::string& body)
    {
        editor::IObjectProxy* proxy = editor.FindProxyObject(entity_id);
        if(!proxy)
            return MakeError(404, "No entity with id " + std::to_string(entity_id));

        if(proxy->IsLocked())
            return MakeError(409, "Entity " + std::to_string(entity_id) + " is locked in the editor");

        const nlohmann::json json = nlohmann::json::parse(body, nullptr, false);
        if(json.is_discarded() || !json.contains("name"))
            return MakeError(400, "Expected a body like { \"name\": \"mission_reward\", \"properties\": { ... } }");

        const std::string component_name = json["name"].get<std::string>();

        const Component* template_component = FindTemplateComponent(component_name);
        if(!template_component)
            return MakeError(400, "No component named '" + component_name + "', see GET /components");

        const bool already_has_component = component::FindComponentFromHash(template_component->hash, proxy->GetComponents()) != nullptr;
        if(already_has_component && !template_component->allow_multiple)
            return MakeError(409, "Entity already has a '" + component_name + "' and it can only have one");

        std::vector<Attribute> property_overrides;
        if(json.contains("properties"))
        {
            const std::string parse_error = ParseProperties(*template_component, json["properties"], property_overrides);
            if(!parse_error.empty())
                return MakeError(400, parse_error);
        }

        const std::vector<uint32_t> added_hashes =
            editor.AddComponentWithProperties(template_component->hash, property_overrides, proxy);
        editor.RegisterTriggers(property_overrides);

        nlohmann::json json_added = nlohmann::json::array();
        for(uint32_t added_hash : added_hashes)
            json_added.push_back(component::ComponentNameFromHash(added_hash));

        ServerResponse response;
        response.body["added"] = json_added;
        response.body["entity"] = SerializeEntity(proxy);
        return response;
    }

    struct NewComponent
    {
        const Component* template_component;
        std::vector<Attribute> properties;
    };

    // Body: { "name": "reward_spawner", "folder": "ambush_setup", "position": { "x": 1, "y": 2 },
    //         "components": [ { "name": "entity_spawn_point", "properties": { ... } } ] }
    // name_folder and transform are always added, everything is validated before the entity is created.
    ServerResponse HandleCreateEntity(editor::Editor& editor, const std::string& body)
    {
        const nlohmann::json json = nlohmann::json::parse(body, nullptr, false);
        if(json.is_discarded() || !json.is_object())
            return MakeError(400, "Expected a body like { \"name\": ..., \"folder\": ..., \"position\": { \"x\": 0, \"y\": 0 }, \"components\": [ ... ] }");

        const std::string entity_name = json.value("name", std::string("unnamed"));
        const std::string folder = json.value("folder", std::string());

        math::Vector position;
        if(json.contains("position"))
        {
            try
            {
                position = json["position"].get<math::Vector>();
            }
            catch(const std::exception&)
            {
                return MakeError(400, "position must be { \"x\": ..., \"y\": ... }");
            }
        }

        std::vector<NewComponent> new_components;

        if(json.contains("components"))
        {
            if(!json["components"].is_array())
                return MakeError(400, "components must be an array");

            for(const nlohmann::json& json_component : json["components"])
            {
                const std::string component_name = json_component.value("name", std::string());
                if(component_name == "name_folder" || component_name == "transform")
                    return MakeError(400, "name_folder and transform are always added, use name, folder and position instead");

                const Component* template_component = FindTemplateComponent(component_name);
                if(!template_component)
                    return MakeError(400, "No component named '" + component_name + "', see GET /components");

                NewComponent new_component;
                new_component.template_component = template_component;

                if(json_component.contains("properties"))
                {
                    const std::string parse_error = ParseProperties(*template_component, json_component["properties"], new_component.properties);
                    if(!parse_error.empty())
                        return MakeError(400, parse_error);
                }

                new_components.push_back(std::move(new_component));
            }
        }

        editor::IObjectProxy* proxy = editor.CreateEntityProxy(entity_name, folder, position);

        for(const NewComponent& new_component : new_components)
        {
            const bool already_has_component =
                component::FindComponentFromHash(new_component.template_component->hash, proxy->GetComponents()) != nullptr;
            if(already_has_component && !new_component.template_component->allow_multiple)
                continue;

            editor.AddComponentWithProperties(new_component.template_component->hash, new_component.properties, proxy);
            editor.RegisterTriggers(new_component.properties);
        }

        ServerResponse response;
        response.status = 201;
        response.body = SerializeEntity(proxy);
        return response;
    }

    // DELETE /entity/{id}/components/{name} or /entity/{id}/components/{name}/{index}, the index picks
    // between several components with the same name. A component that another one depends on is kept.
    ServerResponse HandleRemoveComponent(editor::Editor& editor, uint32_t entity_id, const std::string& component_path)
    {
        editor::IObjectProxy* proxy = editor.FindProxyObject(entity_id);
        if(!proxy)
            return MakeError(404, "No entity with id " + std::to_string(entity_id));

        if(proxy->IsLocked())
            return MakeError(409, "Entity " + std::to_string(entity_id) + " is locked in the editor");

        const size_t slash_pos = component_path.find('/');
        const std::string component_name = component_path.substr(0, slash_pos);
        int component_index = 0;
        if(slash_pos != std::string::npos)
        {
            const std::string index_string = component_path.substr(slash_pos + 1);
            char* parse_end = nullptr;
            component_index = int(std::strtol(index_string.c_str(), &parse_end, 10));
            if(index_string.empty() || *parse_end != '\0' || component_index < 0)
                return MakeError(400, "Component index must be a number: " + index_string);
        }

        if(component_name == "name_folder" || component_name == "transform")
            return MakeError(409, "name_folder and transform can't be removed");

        std::vector<Component>& components = proxy->GetComponents();

        int found_index = 0;
        int n_with_hash = 0;
        size_t remove_index = components.size();
        uint32_t remove_hash = 0;

        for(size_t index = 0; index < components.size(); ++index)
        {
            if(component_name != component::ComponentNameFromHash(components[index].hash))
                continue;

            if(found_index == component_index)
            {
                remove_index = index;
                remove_hash = components[index].hash;
            }

            ++found_index;
            ++n_with_hash;
        }

        if(remove_index == components.size())
            return MakeError(400, "Entity has no component '" + component_name + "' at index " + std::to_string(component_index));

        // Only the last one of a kind is needed by the components that depend on it.
        if(n_with_hash == 1)
        {
            for(const Component& component : components)
            {
                const Component template_component = component::DefaultComponentFromHash(component.hash);
                if(template_component.depends_on == remove_hash)
                {
                    return MakeError(
                        409, "Can't remove '" + component_name + "', '" + component::ComponentNameFromHash(component.hash) + "' depends on it");
                }
            }
        }

        editor.RemoveComponentFromProxy(proxy, remove_index);

        ServerResponse response;
        response.body["removed"] = component_name;
        response.body["entity"] = SerializeEntity(proxy);
        return response;
    }

    // Parses /entity/{id} and /entity/{id}/{sub_resource}.
    bool ParseEntityPath(const std::string& path, const std::string& prefix, uint32_t& out_entity_id, std::string& out_sub_resource)
    {
        const std::string rest = path.substr(prefix.size());
        const size_t slash_pos = rest.find('/');
        const std::string id_string = rest.substr(0, slash_pos);
        out_sub_resource = (slash_pos == std::string::npos) ? std::string() : rest.substr(slash_pos + 1);

        char* parse_end = nullptr;
        const unsigned long entity_id = std::strtoul(id_string.c_str(), &parse_end, 10);
        if(id_string.empty() || *parse_end != '\0')
            return false;

        out_entity_id = static_cast<uint32_t>(entity_id);
        return true;
    }

    ServerResponse RouteRequest(editor::Editor& editor, const ServerRequest& request)
    {
        const std::string entity_prefix = "/entity/";
        const bool is_entity_path = (request.path.rfind(entity_prefix, 0) == 0);

        uint32_t entity_id = 0;
        std::string sub_resource;
        if(is_entity_path && !ParseEntityPath(request.path, entity_prefix, entity_id, sub_resource))
            return MakeError(400, "Entity id must be a number: " + request.path.substr(entity_prefix.size()));

        if(request.method == "GET")
        {
            if(request.path == "/")
                return HandleStatus(editor);
            if(request.path == "/entities")
                return HandleGetEntities(editor);
            if(request.path == "/selection")
                return HandleGetSelection(editor);
            if(request.path == "/components")
                return HandleGetComponents();
            if(is_entity_path && sub_resource.empty())
                return HandleGetEntity(editor, entity_id);
        }
        else if(request.method == "POST")
        {
            if(request.path == "/selection")
                return HandleSetSelection(editor, request.body);
            if(is_entity_path && sub_resource == "components")
                return HandleAddComponent(editor, entity_id, request.body);
            if(request.path == "/entity")
                return HandleCreateEntity(editor, request.body);
        }
        else if(request.method == "PATCH")
        {
            if(is_entity_path && sub_resource.empty())
                return HandlePatchEntity(editor, entity_id, request.body);
        }
        else if(request.method == "DELETE")
        {
            const std::string components_prefix = "components/";
            if(is_entity_path && sub_resource.rfind(components_prefix, 0) == 0)
                return HandleRemoveComponent(editor, entity_id, sub_resource.substr(components_prefix.size()));
        }

        return MakeError(404, "Unknown endpoint: " + request.method + " " + request.path);
    }
}

struct editor::EditorServer::Impl
{
    std::mutex queue_mutex;
    std::deque<PendingRequest> pending_requests;
    bool shutting_down = false;

    // Last, so it's destroyed (and its thread joined) before the queue it calls into.
    std::unique_ptr<HttpListener> listener;

    ServerResponse Enqueue(const ServerRequest& request)
    {
        std::future<ServerResponse> future;

        {
            std::lock_guard<std::mutex> lock(queue_mutex);
            if(shutting_down)
                return MakeError(503, "Editor is shutting down");

            PendingRequest pending;
            pending.request = request;
            future = pending.promise.get_future();
            pending_requests.push_back(std::move(pending));
        }

        // The editor answers in its next frame, unless it's stuck in a modal dialog or similar.
        if(future.wait_for(request_timeout) != std::future_status::ready)
            return MakeError(503, "Editor didn't answer in time");

        return future.get();
    }
};

editor::EditorServer::EditorServer(uint16_t port)
    : m_impl(std::make_unique<Impl>())
{
    Impl* impl = m_impl.get();

    const HttpHandler handler = [impl](const HttpRequest& request) {
        const ServerResponse response = impl->Enqueue(request);
        return HttpResponse{ response.status, response.body.dump(2) };
    };

    m_impl->listener = std::make_unique<HttpListener>("127.0.0.1", port, handler);

    if(m_impl->listener->IsListening())
        System::Log("EditorServer|Listening on http://127.0.0.1:%u", port);
    else
        System::Log("EditorServer|Unable to listen on 127.0.0.1:%u, the server is off.", port);
}

editor::EditorServer::~EditorServer()
{
    {
        std::lock_guard<std::mutex> lock(m_impl->queue_mutex);
        m_impl->shutting_down = true;

        for(PendingRequest& pending : m_impl->pending_requests)
            pending.promise.set_value(MakeError(503, "Editor is shutting down"));
        m_impl->pending_requests.clear();
    }

    m_impl->listener = nullptr;
}

void editor::EditorServer::ProcessRequests(Editor& editor)
{
    std::deque<PendingRequest> requests;

    {
        std::lock_guard<std::mutex> lock(m_impl->queue_mutex);
        requests.swap(m_impl->pending_requests);
    }

    for(PendingRequest& pending : requests)
    {
        ServerResponse response;
        try
        {
            response = RouteRequest(editor, pending.request);
        }
        catch(const std::exception& exception)
        {
            response = MakeError(500, exception.what());
        }

        pending.promise.set_value(std::move(response));
    }
}
