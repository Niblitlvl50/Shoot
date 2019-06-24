
#include "RemoteZone.h"
#include "ZoneFlow.h"
#include "Factories.h"
#include "SpriteResources.h"
#include "RenderLayers.h"

#include "Hud/ConsoleDrawer.h"
#include "Hud/Overlay.h"
#include "Hud/NetworkStatusDrawer.h"

#include "WorldFile.h"
#include "World/World.h"
#include "Navigation/NavmeshFactory.h"

#include "Network/NetworkMessage.h"
#include "Network/NetworkReplicator.h"
#include "Network/ClientManager.h"

#include "Rendering/Sprite/SpriteBatchDrawer.h"
#include "Rendering/Sprite/SpriteSystem.h"
#include "Rendering/Sprite/Sprite.h"

#include "EventHandler/EventHandler.h"
#include "Entity/IEntity.h"
#include "Math/Vector.h"
#include "Math/Quad.h"

#include "SystemContext.h"
#include "TransformSystem.h"

#include "Player/PlayerDaemon.h"

using namespace game;

RemoteZone::RemoteZone(const ZoneCreationContext& context)
    : PhysicsZone(math::ZeroVec, 0.0f)
    , m_system_context(context.system_context)
    , m_event_handler(*context.event_handler)
    , m_game_config(*context.game_config)
{
    using namespace std::placeholders;

    const std::function<bool (const TextMessage&)> text_func = std::bind(&RemoteZone::HandleText, this, _1);
    const std::function<bool (const TransformMessage&)> transform_func = std::bind(&RemoteZone::HandleTransformMessage, this, _1);
    const std::function<bool (const SpawnMessage&)> spawn_func = std::bind(&RemoteZone::HandleSpawnMessage, this, _1);
    const std::function<bool (const SpriteMessage&)> sprite_func = std::bind(&RemoteZone::HandleSpriteMessage, this, _1);
    const std::function<bool (const PingMessage&)> ping_func = std::bind(&RemoteZone::HandlePingMessage, this, _1);

    m_text_token = m_event_handler.AddListener(text_func);
    m_transform_token = m_event_handler.AddListener(transform_func);
    m_spawn_token = m_event_handler.AddListener(spawn_func);
    m_sprite_token = m_event_handler.AddListener(sprite_func);
    m_ping_token = m_event_handler.AddListener(ping_func);
}

RemoteZone::~RemoteZone()
{
    m_event_handler.RemoveListener(m_text_token);
    m_event_handler.RemoveListener(m_transform_token);
    m_event_handler.RemoveListener(m_spawn_token);
    m_event_handler.RemoveListener(m_sprite_token);
    m_event_handler.RemoveListener(m_ping_token);
}

void RemoteZone::OnLoad(mono::ICameraPtr& camera)
{
    m_client_manager = std::make_shared<ClientManager>(&m_event_handler, &m_game_config);
    AddUpdatable(m_client_manager);
    AddUpdatable(std::make_shared<ClientReplicator>(m_client_manager.get()));

    auto sprite_drawer = std::make_shared<mono::SpriteBatchDrawer>(m_system_context);
    AddDrawable(sprite_drawer, GAMEOBJECTS);

    m_console_drawer = std::make_shared<ConsoleDrawer>();
    AddDrawable(m_console_drawer, LayerId::UI);

    m_player_daemon = std::make_unique<ClientPlayerDaemon>(m_event_handler);

/*
    {
        file::FilePtr world_file = file::OpenBinaryFile("res/world.world");
        world::LevelFileHeader world_header;
        world::ReadWorld(world_file, world_header);

        std::vector<ExcludeZone> exclude_zones;
        game::LoadWorld(this, world_header.polygons, exclude_zones);
    }
    */

    auto hud_overlay = std::make_shared<UIOverlayDrawer>();
    hud_overlay->AddChild(std::make_shared<ClientStatusDrawer>(math::Vector(10.0f, 10.0f), m_client_manager.get()));
    AddEntity(hud_overlay, UI);
}

int RemoteZone::OnUnload()
{
    return 0;
}

bool RemoteZone::HandleText(const TextMessage& text_message)
{
    m_console_drawer->AddText(text_message.text, 1500);
    return true;
}

bool RemoteZone::HandleTransformMessage(const TransformMessage& transform_message)
{
    mono::TransformSystem* transform_system = m_system_context->GetSystem<mono::TransformSystem>();
    math::Matrix& transform = transform_system->GetTransform(transform_message.entity_id);
    transform = transform_message.transform;

    return false;
}

bool RemoteZone::HandleSpawnMessage(const SpawnMessage& spawn_message)
{
    mono::SpriteSystem* sprite_system = m_system_context->GetSystem<mono::SpriteSystem>();

    if(!spawn_message.spawn)
        sprite_system->ReleaseSprite(spawn_message.entity_id);

    return true;
}

bool RemoteZone::HandleSpriteMessage(const SpriteMessage& sprite_message)
{
    mono::SpriteSystem* sprite_system = m_system_context->GetSystem<mono::SpriteSystem>();
    const bool is_allocated = sprite_system->IsAllocated(sprite_message.entity_id);
    if(!is_allocated)
    {
        mono::SpriteComponents sprite_data;
        sprite_data.sprite_file = game::SpriteHashToString(sprite_message.filename_hash);
        sprite_system->AllocateSprite(sprite_message.entity_id, sprite_data);
    }

    mono::Sprite* sprite = sprite_system->GetSprite(sprite_message.entity_id);

    sprite->SetShade(sprite_message.shade);
    sprite->SetAnimation(sprite_message.animation_id);
    sprite->SetHorizontalDirection(mono::HorizontalDirection(sprite_message.horizontal_direction));
    sprite->SetVerticalDirection(mono::VerticalDirection(sprite_message.vertical_direction));

    return true;
}

bool RemoteZone::HandlePingMessage(const PingMessage& ping_message)
{
    const uint32_t ping = System::GetMilliseconds() - ping_message.local_time_stamp;
    const std::string ping_text = "Ping " + std::to_string(ping);
    m_console_drawer->AddText(ping_text, 200);

    return true;
}
