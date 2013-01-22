#include "src/pch.h"
#include "UIController.h"
#include "InputState.h"

namespace energonsoftware {

InputState::MouseState::MouseState()
    : position(), wheel_position()
{
    ZeroMemory(buttons, static_cast<unsigned int>(MouseSym::Max) * sizeof(bool));
}

InputState::MouseState::~MouseState() throw()
{
}

InputState::GamePadState::GamePadState()
    : left_stick(), right_stick(),
        left_trigger(0.0f), right_trigger(0.0f)
{
    ZeroMemory(buttons, static_cast<unsigned int>(GamePadSym::Max) * sizeof(bool));
}

InputState::GamePadState::~GamePadState() throw()
{
}

void InputState::destroy(InputState* const state, MemoryAllocator* const allocator)
{
    state->~InputState();
    operator delete(state, 16, *allocator);
}

InputState::InputState()
    : _mouse_state(), _gamepad_state()
{
    ZeroMemory(_keyboard_state, static_cast<unsigned int>(InputKeySym::Max) * sizeof(bool));
}

InputState::~InputState() throw()
{
}

void InputState::keyboard_key(InputKeySym key, bool pressed, bool alert)
{
    _keyboard_state[static_cast<unsigned int>(key)] = pressed;
    if(alert) {
        if(pressed) {
            UIController::controller()->key_down(key);
        } else {
            UIController::controller()->key_up(key);
        }
    }
}

void InputState::mouse_button(MouseSym button, const Position& position, bool pressed, bool alert)
{
    _mouse_state.position = position;
    _mouse_state.buttons[static_cast<unsigned int>(button)] = pressed;
    if(alert) {
        if(pressed) {
            UIController::controller()->mouse_button_down(button, position);
        } else {
            UIController::controller()->mouse_button_up(button, position);
        }
    }
}

}
