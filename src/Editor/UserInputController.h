
#pragma once

#include "Events/EventFwd.h"
#include "EventHandler/EventToken.h"

#include "UI/MainMenuOptions.h"

#include "Tools/TranslateTool.h"
#include "Tools/RotateTool.h"
#include "Tools/CameraTool.h"
#include "Tools/MeasureTool.h"
#include "Tools/PaletteTool.h"

namespace System
{
    class IWindow;
}

namespace editor
{
    struct UIContext;
    struct Grabber;
    
    class UserInputController
    {
    public:

        UserInputController(
            mono::ICamera* camera,
            System::IWindow* window,
            editor::Editor* editor,
            editor::UIContext* context,
            mono::EventHandler& event_handler);
        ~UserInputController();

        void HandleContextMenu(int item_index);
        void SelectTool(ToolsMenuOptions option);

    private:

        mono::EventResult OnMouseDown(const event::MouseDownEvent& event);
        mono::EventResult OnMouseUp(const event::MouseUpEvent& event);
        mono::EventResult OnMouseMove(const event::MouseMotionEvent& event);
        mono::EventResult OnMouseWheel(const event::MouseWheelEvent& event);
        mono::EventResult OnMultiGesture(const event::MultiGestureEvent& event);
        mono::EventResult OnKeyDown(const event::KeyDownEvent& event);
        mono::EventResult OnKeyUp(const event::KeyUpEvent& event);

        System::IWindow* m_window;
        mono::EventHandler& m_event_handler;
        editor::Editor* m_editor;
        editor::UIContext* m_context;

        mono::EventToken<event::MouseUpEvent> m_mouseUpToken;
        mono::EventToken<event::MouseDownEvent> m_mouseDownToken;
        mono::EventToken<event::MouseMotionEvent> m_mouseMoveToken;
        mono::EventToken<event::MouseWheelEvent> m_mouseWheelToken;
        mono::EventToken<event::MultiGestureEvent> m_multiGestureToken;
        mono::EventToken<event::KeyDownEvent> m_keyDownToken;
        mono::EventToken<event::KeyUpEvent> m_key_up_token;

        editor::CameraTool m_camera_tool;
        editor::TranslateTool m_translate_tool;
        editor::RotateTool m_rotate_tool;
        editor::MeasureTool m_measure_tool;
        editor::PaletteTool m_palette_tool;

        editor::ITool* m_active_tool;

        editor::Grabber* m_grabber;
        bool m_box_selection;
        math::Vector m_click_point;
        bool m_is_maximized;
    };
}

