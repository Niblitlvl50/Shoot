
#include "Animator.h"
#include "SpriteOffsetDrawer.h"

#include "EventHandler/EventHandler.h"
#include "Events/EventFuncFwd.h"
#include "Events/KeyEvent.h"
#include "Events/MouseEvent.h"
#include "Events/MultiGestureEvent.h"
#include "Events/TimeScaleEvent.h"

#include "Camera/ICamera.h"

#include "Math/Vector.h"
#include "Math/Quad.h"
#include "Math/Matrix.h"
#include "Math/MathFunctions.h"
#include "System/Keycodes.h"

#include "InterfaceDrawer.h"

#include "EntitySystem/EntitySystem.h"
#include "Rendering/Sprite/ISprite.h"
#include "Rendering/Sprite/SpriteSystem.h"
#include "Rendering/Sprite/SpriteFactory.h"
#include "Rendering/Sprite/SpriteBatchDrawer.h"
#include "TransformSystem/TransformSystem.h"

#include "Rendering/RenderSystem.h"
#include "Rendering/IRenderer.h"
#include "Rendering/Texture/ITexture.h"
#include "Rendering/Texture/ITextureFactory.h"
#include "WriteSpriteFile.h"

#include "Generated/animator_sprite_atlas.h"
#include "Generated/add.h"
#include "Generated/delete.h"
#include "Generated/plus.h"
#include "Generated/save.h"

#include <cmath>

using namespace animator;

namespace
{
    mono::ITexturePtr SetupIcons(UIContext& context)
    {
        const mono::ITextureFactory* texture_factory = mono::RenderSystem::GetTextureFactory();

        mono::ITexturePtr texture =
            texture_factory->CreateTextureFromData(animator_sprite_atlas_data, animator_sprite_atlas_data_length, "res/animator_sprite_atlas.png");

        const mono::ISpriteFactory* sprite_factory = mono::RenderSystem::GetSpriteFactory();

        const mono::ISpritePtr add = sprite_factory->CreateSpriteFromRaw(add_data);
        const mono::ISpritePtr del = sprite_factory->CreateSpriteFromRaw(delete_data);
        const mono::ISpritePtr plus = sprite_factory->CreateSpriteFromRaw(plus_data);
        const mono::ISpritePtr save = sprite_factory->CreateSpriteFromRaw(save_data);

        context.tools_texture_id = texture->Id();
        context.save_icon = math::Quad(save->GetCurrentFrame().uv_upper_left, save->GetCurrentFrame().uv_lower_right);
        context.add_icon = math::Quad(add->GetCurrentFrame().uv_lower_right, add->GetCurrentFrame().uv_upper_left);
        context.plus_icon = math::Quad(plus->GetCurrentFrame().uv_lower_right, plus->GetCurrentFrame().uv_upper_left);
        context.delete_icon = math::Quad(del->GetCurrentFrame().uv_lower_right, del->GetCurrentFrame().uv_upper_left);

        return texture;
    }

    void SetupSpriteIcons(const std::vector<std::string>& sprite_files, animator::UIContext& context)
    {
        for(const std::string& sprite_file : sprite_files)
        {
            const std::string full_sprite_path = "res/sprites/" + sprite_file;
            const mono::SpriteData* sprite_data =
                mono::RenderSystem::GetSpriteFactory()->GetSpriteDataForFile(full_sprite_path.c_str());

            std::string category = sprite_data->source_folder;

            const size_t pos = category.find("res/images/");
            if(pos != std::string::npos)
            {
                const size_t offset = std::size("res/images/");
                const size_t slash_pos = category.find_first_of('/', offset);
                category = category.substr(offset - 1, slash_pos - offset + 1);
            }

            context.ui_icons[sprite_file] = {
                mono::RenderSystem::GetTextureFactory()->CreateTexture(sprite_data->texture_file.c_str()),
                sprite_data->frames.front().uv_upper_left,
                sprite_data->frames.front().uv_lower_right,
                sprite_data->frames.front().size,
                category
            };
        }
    }

    class ActiveFrameUpdater : public mono::IUpdatable
    {
    public:

        ActiveFrameUpdater(const mono::ISprite* sprite, animator::UIContext& ui_context)
            : m_sprite(sprite)
            , m_ui_context(ui_context)
        { }

        void Update(const mono::UpdateContext& update_context) override
        {
            (void)update_context;
            m_ui_context.selected_frame = m_sprite->GetActiveAnimationFrame();
        }
        
        const mono::ISprite* m_sprite;
        animator::UIContext& m_ui_context;
    };
}

Animator::Animator(
    mono::TransformSystem* transform_system,
    mono::SpriteSystem* sprite_system,
    mono::RenderSystem* render_system,
    mono::EntitySystem* entity_system,
    mono::EventHandler* event_handler,
    float pixels_per_meter,
    const char* sprite_file)
    : m_transform_system(transform_system)
    , m_sprite_system(sprite_system)
    , m_render_system(render_system)
    , m_entity_system(entity_system)
    , m_event_handler(event_handler)
    , m_pixels_per_meter(pixels_per_meter)
    , m_sprite_file(sprite_file)
{
    using namespace std::placeholders;

    m_context.update_speed = 1.0f;
    m_context.offset_mode = false;

    animator::LoadAllSprites("res/sprites/all_sprite_files.json");

    // Setup UI callbacks
    m_context.on_save                   = std::bind(&Animator::SaveSprite, this);

    m_context.add_animation             = std::bind(&Animator::OnAddAnimation, this);
    m_context.delete_animation          = std::bind(&Animator::OnDeleteAnimation, this);
    m_context.set_active_animation      = std::bind(&Animator::SetAnimation, this, _1);
    m_context.set_name                  = std::bind(&Animator::OnNameAnimation, this, _1);
    m_context.toggle_loop               = std::bind(&Animator::OnLoopToggle, this, _1);

    m_context.add_frame                 = std::bind(&Animator::OnAddFrame, this);
    m_context.delete_frame              = std::bind(&Animator::OnDeleteFrame, this, _1);
    m_context.animation_frame_updated   = std::bind(&Animator::SetAnimationFrame, this, _1, _2);
    m_context.set_frame_duration        = std::bind(&Animator::SetFrameDuration, this, _1);
    m_context.set_active_frame          = std::bind(&Animator::SetActiveFrame, this, _1);
    m_context.set_frame_offset          = std::bind(&Animator::SetFrameOffset, this, _1);

    m_context.set_speed                 = std::bind(&Animator::SetSpeed, this, _1);
    m_context.toggle_playing            = std::bind(&Animator::TogglePlaying, this);
    m_context.toggle_offset_mode        = std::bind(&Animator::ToggleOffsetMode, this);
}

Animator::~Animator()
{
    m_event_handler->RemoveListener(m_key_down_token);
    m_event_handler->RemoveListener(m_mouse_wheel_token);
    m_event_handler->RemoveListener(m_multi_gesture_token);
}

void Animator::OnLoad(mono::ICamera* camera, mono::IRenderer* renderer)
{
    (void)renderer;

    m_camera = camera;
    m_camera->SetViewportSize(math::Vector(10.0f, 7.0f));

    m_tools_texture = SetupIcons(m_context);
    SetupSpriteIcons(animator::GetAllSprites(), m_context);

    m_sprite_data = const_cast<mono::SpriteData*>(mono::RenderSystem::GetSpriteFactory()->GetSpriteDataForFile(m_sprite_file));
    m_context.sprite_data = m_sprite_data;
    m_context.animation_playing = false;

    mono::Entity* entity = m_entity_system->AllocateEntity("animator");

    mono::SpriteComponents sprite_component;
    sprite_component.sprite_file = m_sprite_file;
    m_sprite = m_sprite_system->AllocateSprite(entity->id, sprite_component);
    m_sprite->SetAnimationPlayback(mono::PlaybackMode::PAUSED);

    m_sprite_batch_drawer = new mono::SpriteBatchDrawer(m_transform_system, m_sprite_system, m_render_system);

    SetAnimation(m_sprite->GetActiveAnimation());

    m_input_handler = std::make_unique<ImGuiInputHandler>(*m_event_handler);

    using namespace std::placeholders;
    const event::KeyDownEventFunc& key_down_func        = std::bind(&Animator::OnKeyDownUp, this, _1);
    const event::MouseWheelEventFunc& mouse_wheel       = std::bind(&Animator::OnMouseWheel, this, _1);
    const event::MultiGestureEventFunc& multi_gesture   = std::bind(&Animator::OnMultiGesture, this, _1);

    m_key_down_token = m_event_handler->AddListener(key_down_func);
    m_mouse_wheel_token = m_event_handler->AddListener(mouse_wheel);
    m_multi_gesture_token = m_event_handler->AddListener(multi_gesture);

    AddUpdatable(new ActiveFrameUpdater(m_sprite, m_context));
    AddDrawable(m_sprite_batch_drawer, 0);
    AddDrawable(new SpriteOffsetDrawer(m_transform_system, m_sprite_data, entity->id, m_context.offset_mode), 0);
    AddDrawable(new InterfaceDrawer(m_context), 1);
}

int Animator::OnUnload()
{
    m_camera = nullptr;
    SaveSprite();
    return 0;
}

void Animator::SetAnimation(int animation_id)
{
    const int animations = m_sprite_data->animations.size();
    if(animation_id < animations)
    {
        m_sprite->SetAnimation(animation_id);
        m_context.animation_id = animation_id;
        SetActiveFrame(m_sprite->GetActiveAnimationFrame());
    }
}

mono::EventResult Animator::OnKeyDownUp(const event::KeyDownEvent& event)
{
    int animation = -1;
    
    switch(event.key)
    {
        case Keycode::ENTER:
        case Keycode::SPACE:
        {
            TogglePlaying();
            return mono::EventResult::HANDLED;
        }
        case Keycode::LEFT:
        case Keycode::RIGHT:
        {
            if(event.alt)
            {
                const float add_value = (event.key == Keycode::LEFT) ? -0.5f : +0.5f;
                m_context.frame_offset_pixels.x += add_value;
                SetFrameOffset(m_context.frame_offset_pixels);
            }
            else
            {
                const int add_value = (event.key == Keycode::LEFT) ? -1 : +1;
                const int id = m_sprite->GetActiveAnimation() + add_value;
                animation = std::clamp(id, 0, (int)m_sprite_data->animations.size() -1);
            }

            break;
        }
        case Keycode::UP:
        case Keycode::DOWN:
        {
            if(event.alt)
            {
                const float add_value = (event.key == Keycode::UP) ? +0.5f : -0.5f;
                m_context.frame_offset_pixels.y += add_value;
                SetFrameOffset(m_context.frame_offset_pixels);
            }
            else
            {
                const int add_value = (event.key == Keycode::UP) ? -1 : +1;
                const int new_active_frame = m_sprite->GetActiveAnimationFrame() + add_value;

                const mono::SpriteAnimation& sprite_animation = m_sprite_data->animations[m_sprite->GetActiveAnimation()];
                const int frame = std::clamp(new_active_frame, 0, (int)sprite_animation.frames.size() -1);
                SetActiveFrame(frame);

                m_sprite->SetAnimationPlayback(mono::PlaybackMode::PAUSED);
                m_context.animation_playing = false;
            }

            return mono::EventResult::HANDLED;
        }
        case Keycode::ZERO:
            animation = 0;
            break;
        case Keycode::ONE:
            animation = 1;
            break;
        case Keycode::TWO:
            animation = 2;
            break;
        case Keycode::THREE:
            animation = 3;
            break;
        case Keycode::FOUR:
            animation = 4;
            break;
        case Keycode::FIVE:
            animation = 5;
            break;
        case Keycode::SIX:
            animation = 6;
            break;
        case Keycode::SEVEN:
            animation = 7;
            break;
        case Keycode::EIGHT:
            animation = 8;
            break;
        case Keycode::NINE:
            animation = 9;
            break;
        case Keycode::TAB:
            ToggleOffsetMode();
            break;
        default:
            break;
    }

    if(animation != -1)
    {
        SetAnimation(animation);
        return mono::EventResult::HANDLED;
    }

    return mono::EventResult::PASS_ON;
}

mono::EventResult Animator::OnMouseWheel(const event::MouseWheelEvent& event)
{
    const float multiplier = (event.y < 0.0f) ? 1.0f : -1.0f;
    Zoom(multiplier);
    return mono::EventResult::HANDLED;
}

mono::EventResult Animator::OnMultiGesture(const event::MultiGestureEvent& event)
{
    if(std::fabs(event.distance) < 1e-3)
        return mono::EventResult::PASS_ON;

    const float multiplier = (event.distance < 0.0f) ? 1.0f : -1.0f;
    Zoom(multiplier);
    return mono::EventResult::HANDLED;
}

void Animator::OnAddAnimation()
{
    mono::SpriteAnimation new_animation;
    new_animation.name = "new";
    new_animation.frames.push_back(0);
    new_animation.looping = false;

    const int animation_id = m_sprite_data->animations.size();
    m_sprite_data->animations.push_back(new_animation);

    SetAnimation(animation_id);
    m_sprite->RestartAnimation();

    m_sprite_batch_drawer->ReloadSpriteData(m_sprite_data->hash);
}

void Animator::OnDeleteAnimation()
{
    const int active_animation = m_sprite->GetActiveAnimation();
    m_sprite_data->animations.erase(m_sprite_data->animations.begin() + active_animation);
    if(active_animation >= static_cast<int>(m_sprite_data->animations.size()))
        SetAnimation(active_animation -1);

    m_sprite_batch_drawer->ReloadSpriteData(m_sprite_data->hash);
}

void Animator::OnAddFrame()
{
    const int current_id = m_sprite->GetActiveAnimation();
    m_sprite_data->animations[current_id].frames.push_back(0);

    m_sprite_batch_drawer->ReloadSpriteData(m_sprite_data->hash);
}

void Animator::OnDeleteFrame(int id)
{
    const int current_id = m_sprite->GetActiveAnimation();
    std::vector<int>& frames = m_sprite_data->animations[current_id].frames;
    frames.erase(frames.begin() + id);

    m_sprite_batch_drawer->ReloadSpriteData(m_sprite_data->hash);
}

void Animator::OnNameAnimation(const char* new_name)
{
    const int current_id = m_sprite->GetActiveAnimation();
    m_sprite_data->animations[current_id].name = new_name;
}

void Animator::OnLoopToggle(bool state)
{
    const int current_id = m_sprite->GetActiveAnimation();
    m_sprite_data->animations[current_id].looping = state;
    m_sprite->RestartAnimation();
}

void Animator::SetFrameDuration(int new_frame_duration)
{
    const int current_id = m_sprite->GetActiveAnimation();
    m_sprite_data->animations[current_id].frame_duration = new_frame_duration;
}

void Animator::Zoom(float multiplier)
{
    math::Vector viewport_size = m_camera->GetViewportSize();

    const float resize_value = viewport_size.x * multiplier * 0.15f;
    const float aspect = viewport_size.x / viewport_size.y;

    viewport_size.x += resize_value * aspect;
    viewport_size.y += resize_value;

    m_camera->SetTargetViewportSize(viewport_size);
}

void Animator::SetActiveFrame(int frame)
{
    m_sprite->SetActiveAnimationFrame(frame);
    m_context.selected_frame = frame;

    const int active_animation = m_sprite->GetActiveAnimation();
    const int frame_index = m_sprite_data->animations[active_animation].frames[frame];

    m_context.frame_offset_pixels = m_sprite_data->frames[frame_index].center_offset * m_pixels_per_meter;
}

void Animator::SetAnimationFrame(int animation_frame_index, int new_frame)
{
    const int active_animation = m_sprite->GetActiveAnimation();
    m_sprite_data->animations[active_animation].frames[animation_frame_index] = new_frame;
}

void Animator::SetFrameOffset(const math::Vector& frame_offset_pixels)
{
    const math::Vector new_frame_offset = frame_offset_pixels / m_pixels_per_meter;

    const int active_animation = m_sprite->GetActiveAnimation();
    const int active_frame = m_sprite->GetActiveAnimationFrame();
    const int frame_index = m_sprite_data->animations[active_animation].frames[active_frame];
    m_sprite_data->frames[frame_index].center_offset = new_frame_offset;

    m_sprite_batch_drawer->ReloadSpriteData(m_sprite_data->hash);
}

void Animator::SaveSprite()
{
    WriteSpriteFile(m_sprite_file, m_sprite_data);
}

void Animator::SetSpeed(float new_speed)
{
    m_event_handler->DispatchEvent(event::TimeScaleEvent(new_speed));
}

void Animator::TogglePlaying()
{
    m_context.animation_playing = !m_context.animation_playing;

    // This is a bunch of bs and can probably be done in a less complex way...
    const bool animation_done = m_sprite->IsActiveAnimationDone();
    if(animation_done && !m_context.animation_playing)
        m_context.animation_playing = true;

    const mono::PlaybackMode new_mode =
        m_context.animation_playing ? mono::PlaybackMode::PLAYING : mono::PlaybackMode::PAUSED;

    m_sprite->SetAnimationPlayback(new_mode);

    if(animation_done && new_mode == mono::PlaybackMode::PLAYING)
        m_sprite->RestartAnimation();
}

void Animator::ToggleOffsetMode()
{
    m_context.offset_mode = !m_context.offset_mode;
}
