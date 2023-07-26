
#include "UserInputController.h"
#include "Editor.h"
#include "Grabber.h"

#include "EventHandler/EventHandler.h"
#include "Events/EventFuncFwd.h"
#include "Events/MouseEvent.h"
#include "Events/MultiGestureEvent.h"
#include "Events/KeyEvent.h"
#include "System/Keycodes.h"

#include "UI/ImGuiInterfaceDrawer.h"

#include <limits>

namespace
{
    struct ToolData
    {
        editor::ITool* tool;
        editor::Notification notification;
        std::vector<std::string> context_menu;
    };

    ToolData g_tools[editor::ToolsMenuOptions::N_TOOLS];
}

using namespace editor;

UserInputController::UserInputController(
    mono::ICamera* camera,
    editor::Editor* editor,
    editor::UIContext* context,
    mono::EventHandler& event_handler)
    : m_event_handler(event_handler)
    , m_editor(editor)
    , m_context(context)
    , m_camera_tool(camera)
    , m_translate_tool(editor)
    , m_rotate_tool(editor)
    , m_measure_tool(editor)
    , m_palette_tool(editor)
    , m_active_tool(nullptr)
    , m_grabber(nullptr)
    , m_box_selection(false)
{
    using namespace std::placeholders;

    const event::MouseDownEventFunc& mouse_down = std::bind(&UserInputController::OnMouseDown, this, _1);
    const event::MouseUpEventFunc& mouse_up = std::bind(&UserInputController::OnMouseUp, this, _1);
    const event::MouseMotionEventFunc& mouse_move = std::bind(&UserInputController::OnMouseMove, this, _1);
    const event::MouseWheelEventFunc& mouse_wheel = std::bind(&UserInputController::OnMouseWheel, this, _1);
    const event::MultiGestureEventFunc& multi_gesture = std::bind(&UserInputController::OnMultiGesture, this, _1);
    const event::KeyDownEventFunc& key_down = std::bind(&UserInputController::OnKeyDown, this, _1);
    const event::KeyUpEventFunc& key_up = std::bind(&UserInputController::OnKeyUp, this, _1);

    m_mouseDownToken = m_event_handler.AddListener(mouse_down);
    m_mouseUpToken = m_event_handler.AddListener(mouse_up);
    m_mouseMoveToken = m_event_handler.AddListener(mouse_move);
    m_mouseWheelToken = m_event_handler.AddListener(mouse_wheel);
    m_multiGestureToken = m_event_handler.AddListener(multi_gesture);
    m_keyDownToken = m_event_handler.AddListener(key_down);
    m_key_up_token = m_event_handler.AddListener(key_up);

    g_tools[ToolsMenuOptions::TRANSLATE_TOOL]   = { &m_translate_tool, Notification(editor::wrench_texture, "Translate tool", 2.0f), { } };
    g_tools[ToolsMenuOptions::ROTATE_TOOL]      = { &m_rotate_tool, Notification(editor::wrench_texture, "Rotate tool", 2.0f), { } };
    g_tools[ToolsMenuOptions::MEASURE_TOOL]     = { &m_measure_tool, Notification(editor::wrench_texture, "Measure tool", 2.0f), { } };
    g_tools[ToolsMenuOptions::PALETTE_TOOL]     = { &m_palette_tool, Notification(editor::wrench_texture, "Palette Tool", 2.0f), { } };

    SelectTool(ToolsMenuOptions::TRANSLATE_TOOL);
}

UserInputController::~UserInputController()
{
    m_event_handler.RemoveListener(m_mouseDownToken);
    m_event_handler.RemoveListener(m_mouseUpToken);
    m_event_handler.RemoveListener(m_mouseMoveToken);
    m_event_handler.RemoveListener(m_mouseWheelToken);
    m_event_handler.RemoveListener(m_multiGestureToken);
    m_event_handler.RemoveListener(m_keyDownToken);
    m_event_handler.RemoveListener(m_key_up_token);
}

void UserInputController::HandleContextMenu(int item_index)
{
    m_active_tool->HandleContextMenu(item_index);
}

void UserInputController::SelectTool(ToolsMenuOptions option)
{
    const ToolData& tool_data = g_tools[option];
    if(m_active_tool)
    {
        if(m_active_tool == tool_data.tool)
            return;

        m_active_tool->End();
    }

    m_active_tool = tool_data.tool;
    m_context->notifications.push_back(tool_data.notification);
    m_context->context_menu_items = tool_data.context_menu;
    m_context->active_tool_index = static_cast<int>(option);

    m_active_tool->Begin();
}

mono::EventResult UserInputController::OnMouseDown(const event::MouseDownEvent& event)
{
    bool handled = false;

    if(event.key == MouseButton::LEFT)
    {
        const math::Vector world_position = { event.world_x, event.world_y };

        // Check for grabbers first
        m_grabber = m_editor->FindGrabber(world_position);
        if(m_grabber)
        {
            handled = true;
        }
        else
        {
            uint32_t entity_id = std::numeric_limits<uint32_t>::max();

            if(event.shift)
            {
                m_click_point = world_position;
                m_box_selection = true;
            }
            else
            {
                m_editor->SetSelectionPoint(world_position);
                IObjectProxy* proxy = m_editor->FindProxyObject(world_position);
                if(proxy)
                {
                    entity_id = proxy->Id();
                    const Selection new_selection = { entity_id };

                    const std::vector<uint32_t> selection = m_editor->GetSelection();
                    const bool already_selected = (std::find(selection.begin(), selection.end(), entity_id) != selection.end());

                    if(event.ctrl)
                        already_selected ? m_editor->RemoveFromSelection(new_selection) : m_editor->AddToSelection(new_selection);
                    else if(!already_selected)
                        m_editor->SetSelection(new_selection);
                }
                else
                {
                    m_editor->ClearSelection();
                }

                m_active_tool->HandleMouseDown(world_position, entity_id);
                handled = m_active_tool->IsActive();
            }
        }
    }

    if(!handled)
        m_camera_tool.HandleMouseDown(math::Vector(event.screen_x, event.screen_y));

    return mono::EventResult::HANDLED;
}

mono::EventResult UserInputController::OnMouseUp(const event::MouseUpEvent& event)
{
    m_grabber = nullptr;

    if(event.key == MouseButton::LEFT)
    {
        if(m_box_selection)
        {
            m_box_selection = false;

            math::Quad box = { m_click_point, math::Vector(event.world_x, event.world_y) };
            math::NormalizeQuad(box);

            std::vector<IObjectProxy*> found_proxies = m_editor->FindProxiesFromBox(box);

            Selection selection;
            for(IObjectProxy* proxy : found_proxies)
                selection.push_back(proxy->Id());
            m_editor->SetSelection(selection);
        }
        else
        {
            m_active_tool->HandleMouseUp(math::Vector(event.world_x, event.world_y));
        }
    }
    else if(event.key == MouseButton::RIGHT)
    {
        m_context->show_context_menu = !m_context->context_menu_items.empty();
    }

    m_camera_tool.HandleMouseUp(math::Vector(event.screen_x, event.screen_y));

    return mono::EventResult::HANDLED;
}

mono::EventResult UserInputController::OnMouseMove(const event::MouseMotionEvent& event)
{
    const math::Vector world_position = { event.world_x, event.world_y };
    const math::Vector screen_position = { (float)event.screen_x, (float)event.screen_y };

    m_editor->SelectGrabber(world_position);

    // Deal with the grabbers first, move if we have one.
    if(m_grabber)
    {
        m_grabber->position = world_position;
        m_grabber->callback(world_position);
    }
    else if(m_box_selection)
    {
        const math::Quad box = { m_click_point, world_position };
        m_editor->SetSelectionBox(box);
    }
    else
    {
        if(m_active_tool->IsActive())
            m_active_tool->HandleMousePosition(world_position);

        if(m_camera_tool.IsActive())
            m_camera_tool.HandleMousePosition(screen_position);
    }

    IObjectProxy* proxy = m_editor->FindProxyObject(world_position);
    m_editor->PreselectProxyObject(proxy);

    m_context->world_mouse_position = world_position;
    return mono::EventResult::HANDLED;
}

mono::EventResult UserInputController::OnMouseWheel(const event::MouseWheelEvent& event)
{
    m_camera_tool.HandleMouseWheel(event.x, event.y);
    return mono::EventResult::HANDLED;
}

mono::EventResult UserInputController::OnMultiGesture(const event::MultiGestureEvent& event)
{
    m_camera_tool.HandleMultiGesture(math::Vector(event.x, event.y), event.distance);
    return mono::EventResult::HANDLED;
}

mono::EventResult UserInputController::OnKeyDown(const event::KeyDownEvent& event)
{
    m_active_tool->UpdateModifierState(event.ctrl, event.shift, event.alt);

    if(event.key == Keycode::D && event.ctrl)
        m_editor->EditorMenuCallback(editor::EditorMenuOptions::DUPLICATE);
    else if(event.key == Keycode::N && event.ctrl)
        m_editor->EditorMenuCallback(editor::EditorMenuOptions::NEW);
    else if(event.key == Keycode::S && event.ctrl)
        m_editor->EditorMenuCallback(editor::EditorMenuOptions::SAVE);
    else if(event.key == Keycode::F)
        m_editor->TeleportToSelectedProxyObject();
    else if(event.key == Keycode::A && event.ctrl)
        m_editor->AddComponentUI();
    else if(event.key == Keycode::I && event.ctrl && event.shift)
        m_editor->EditorMenuCallback(editor::EditorMenuOptions::IMPORT_ENTITY);
    else if(event.key == Keycode::E && event.ctrl && event.shift)
        m_editor->EditorMenuCallback(editor::EditorMenuOptions::EXPORT_ENTITY);
    else if(event.key == Keycode::ONE)
        SelectTool(ToolsMenuOptions::TRANSLATE_TOOL);
    else if(event.key == Keycode::TWO)
        SelectTool(ToolsMenuOptions::ROTATE_TOOL);
    else if(event.key == Keycode::THREE)
        SelectTool(ToolsMenuOptions::PALETTE_TOOL);
    else if(event.key == Keycode::M)
        SelectTool(ToolsMenuOptions::MEASURE_TOOL);
    else if(event.key == Keycode::N)
        m_editor->EnableDrawObjectNames(!m_editor->DrawObjectNames());
    else if(event.key == Keycode::S)
        m_editor->EnableDrawSnappers(!m_editor->DrawSnappers());
    else if(event.key == Keycode::O)
        m_editor->EnableDrawOutline(!m_editor->DrawOutline());
    else if(event.key == Keycode::L && event.ctrl)
        m_editor->EnableLights(!m_editor->DrawLights());
    else if(event.key == Keycode::L)
        m_editor->EnableDrawLevelMetadata(!m_editor->DrawLevelMetadata());
    else if(event.key == Keycode::G && event.alt)
        m_editor->EnableDrawGrid(!m_editor->DrawGrid());
    else if(event.key == Keycode::G)
        m_editor->EnableSnapToGrid(!m_editor->SnapToGrid());
    else if(event.key == Keycode::H)
        m_editor->MoveSelectionToHere();
    else if(event.key == Keycode::A)
        m_editor->EnableDrawAllObjects(!m_editor->DrawAllObjects());
    else if(event.key == Keycode::BACKSPACE || event.key == Keycode::DELETE)
        m_editor->OnDeleteObject();
    else if(event.key == Keycode::ESCAPE)
        m_editor->Quit();
    else if(event.key == Keycode::LEFT || event.key == Keycode::RIGHT || event.key == Keycode::UP || event.key == Keycode::DOWN)
    {
        const float offset = event.shift ? 0.5f : 1.0f;
        math::Vector delta;

        if(event.key == Keycode::LEFT)
            delta.x -= offset;
        else if(event.key == Keycode::RIGHT)
            delta.x += offset;
        else if(event.key == Keycode::UP)
            delta.y += offset;
        else if(event.key == Keycode::DOWN)
            delta.y -= offset;

        const std::vector<uint32_t> selection = m_editor->GetSelection();
        if(selection.empty())
        {
            m_camera_tool.MoveCamera(delta);
        }
        else
        {
            for(uint32_t id : m_editor->GetSelection())
            {
                IObjectProxy* proxy = m_editor->FindProxyObject(id);
                if(proxy)
                    proxy->SetPosition(proxy->GetPosition() + delta);
            }
        }
    }

    return mono::EventResult::PASS_ON;
}

mono::EventResult UserInputController::OnKeyUp(const event::KeyUpEvent& event)
{
    m_active_tool->UpdateModifierState(event.ctrl, event.shift, event.alt);

    switch(event.key)
    {
        case Keycode::M:
            SelectTool(ToolsMenuOptions::TRANSLATE_TOOL);
            break;
        default:
            break;
    }

    return mono::EventResult::PASS_ON;
}
