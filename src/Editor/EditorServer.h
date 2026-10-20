
#pragma once

#include <cstdint>
#include <memory>

namespace editor
{
    class Editor;

    // A small http server on localhost so tools can read and change the open world. Requests come in
    // on the server's own thread but are only queued there, ProcessRequests runs them on the editor's
    // thread so they can touch editor state without any locking.
    //
    //  GET  /                  - Editor status, the open world and number of entities.
    //  GET  /entities          - All entities, with name, folder, position and component names.
    //  GET  /entity/{id}       - One entity with all its components and attributes.
    //  GET  /selection         - The selected entities in full, and their combined bounds.
    //  POST /selection         - Set the selection, body is { "ids": [ ... ] }.
    //  GET  /components        - Every registered component with its default attributes.
    //  PATCH /entity/{id}      - Change attribute values, body is
    //                            { "components": [ { "name": "spawn_point", "index": 0, "properties": { "spawn_limit": 5 } } ] }
    //  POST /entity/{id}/components - Add a component, and any it depends on. Body is
    //                            { "name": "mission_reward", "properties": { "reward_chips": 50 } }
    //  DELETE /entity/{id}/components/{name}[/{index}] - Remove a component.
    //  POST /entity            - Create an entity, body is { "name": ..., "folder": ..., "position": { "x", "y" },
    //                            "components": [ { "name": ..., "properties": { ... } } ] }
    class EditorServer
    {
    public:

        EditorServer(uint16_t port);
        ~EditorServer();

        void ProcessRequests(Editor& editor);

    private:

        struct Impl;
        std::unique_ptr<Impl> m_impl;
    };
}
