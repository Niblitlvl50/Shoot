
#include "RemoteZone.h"
#include "ZoneFlow.h"
#include "Factories.h"
#include "RenderLayers.h"
#include "Hud/ConsoleDrawer.h"

#include "WorldFile.h"
#include "World/World.h"
#include "Navigation/NavmeshFactory.h"

#include "Network/MessageDispatcher.h"
#include "Network/RemoteConnection.h"
#include "Network/NetworkMessage.h"

#include "EventHandler/EventHandler.h"
#include "Entity/IEntity.h"
#include "Math/Vector.h"
#include "Math/Quad.h"

using namespace game;

RemoteZone::RemoteZone(const ZoneCreationContext& context)
    : PhysicsZone(math::ZeroVec, 0.0f)
    , m_event_handler(*context.event_handler)
    , m_game_config(*context.game_config)
    , m_dispatcher(std::make_shared<MessageDispatcher>(*context.event_handler))
{
    using namespace std::placeholders;

    const std::function<bool (const TextMessage&)> text_func = std::bind(&RemoteZone::HandleText, this, _1);
    const std::function<bool (const PositionalMessage&)> positional_func = std::bind(&RemoteZone::HandlePosMessage, this, _1);
    const std::function<bool (const SpawnMessage&)> spawn_func = std::bind(&RemoteZone::HandleSpawnMessage, this, _1);
    const std::function<bool (const DespawnMessage&)> despawn_func = std::bind(&RemoteZone::HandleDespawnMessage, this, _1);
    const std::function<bool (const AnimationMessage&)> anim_func = std::bind(&RemoteZone::HandleAnimMessage, this, _1);

    m_text_token = m_event_handler.AddListener(text_func);
    m_pos_token = m_event_handler.AddListener(positional_func);
    m_spawn_token = m_event_handler.AddListener(spawn_func);
    m_despawn_token = m_event_handler.AddListener(despawn_func);
    m_anim_token = m_event_handler.AddListener(anim_func);
}

RemoteZone::~RemoteZone()
{
    m_event_handler.RemoveListener(m_text_token);
    m_event_handler.RemoveListener(m_pos_token);
    m_event_handler.RemoveListener(m_spawn_token);
    m_event_handler.RemoveListener(m_despawn_token);
    m_event_handler.RemoveListener(m_anim_token);
}

void RemoteZone::OnLoad(mono::ICameraPtr& camera)
{
    network::ISocketPtr in_socket = network::OpenLoopbackSocket(m_game_config.client_port, false);
    network::ISocketPtr out_socket = network::OpenLoopbackSocket(6667, false);

    network::Address address;
    address.host = network::GetLoopbackAddress();
    address.port = m_game_config.server_port;

    m_connection =
        std::make_unique<RemoteConnection>(m_dispatcher.get(), std::move(in_socket), std::move(out_socket), address);

    AddUpdatable(m_dispatcher);

    m_console_drawer = std::make_shared<ConsoleDrawer>();
    AddDrawable(m_console_drawer, LayerId::UI);

    {
        file::FilePtr world_file = file::OpenBinaryFile("res/world.world");
        world::LevelFileHeader world_header;
        world::ReadWorld(world_file, world_header);

        std::vector<ExcludeZone> exclude_zones;
        game::LoadWorld(this, world_header.polygons, exclude_zones);
    }
}

void RemoteZone::Accept(mono::IRenderer& renderer)
{
    using LayerDrawable = std::pair<int, mono::IDrawablePtr>;

    const auto sort_on_y = [](const LayerDrawable& first, const LayerDrawable& second) {
        if(first.first == second.first)
            return first.second->BoundingBox().mA.y > second.second->BoundingBox().mA.y;
        
        return first.first < second.first;
    };

    std::sort(m_drawables.begin(), m_drawables.end(), sort_on_y);
    PhysicsZone::Accept(renderer);
}

int RemoteZone::OnUnload()
{
    return QUIT;
}

bool RemoteZone::HandleText(const TextMessage& text_message)
{
    m_console_drawer->AddText(text_message.text);
    return true;
}

bool RemoteZone::HandlePosMessage(const PositionalMessage& pos_message)
{
    mono::IEntityPtr entity = FindEntityFromId(pos_message.entity_id);
    if(entity)
    {
        entity->SetPosition(math::Vector(pos_message.x, pos_message.y));
        entity->SetRotation(pos_message.rotation);
    }

    return true;
}

bool RemoteZone::HandleSpawnMessage(const SpawnMessage& spawn_message)
{
    //AddEntity(enemy, LayerId::GAMEOBJECTS);
    return true;
}

bool RemoteZone::HandleDespawnMessage(const DespawnMessage& despawn_message)
{
    mono::IEntityPtr entity = FindEntityFromId(despawn_message.entity_id);
    if(entity)
        RemoveEntity(entity);
    return true;
}

bool RemoteZone::HandleAnimMessage(const AnimationMessage& anim_message)
{
    mono::IEntityPtr entity = FindEntityFromId(anim_message.entity_id);
    if(entity)
    {
// natalie är bäst
    }

    return true;
}
