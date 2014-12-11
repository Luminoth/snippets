#if !defined __INPUT_H__
#define __INPUT_H__

#include "src/core/math/Vector.h"
#include "InputSym.h"

namespace energonsoftware {

class InputState
{
private:
    struct MouseState
    {
        bool buttons[static_cast<unsigned int>(MouseSym::Max)];
        Position position, wheel_position;

        MouseState();
        virtual ~MouseState() noexcept;
    };

    struct GamePadState
    {
        bool buttons[static_cast<unsigned int>(GamePadSym::Max)];
        Position left_stick, right_stick;
        float left_trigger, right_trigger;

        GamePadState();
        virtual ~GamePadState() noexcept;
    };

public:
    virtual ~InputState() noexcept;

public:
    void keyboard_key(InputKeySym key, bool pressed, bool alert=true);
    bool keyboard_key(InputKeySym key) const { return _keyboard_state[static_cast<unsigned int>(key)]; }

    void mouse_move(const Vector2& amount) { _mouse_state.position += amount; }
    void mouse_position(const Position& position) { _mouse_state.position = position; }
    const Position& mouse_position() const { return _mouse_state.position; }

    void mouse_wheel_move(const Vector2& amount) { _mouse_state.wheel_position += amount; }
    void mouse_wheel_position(const Position& position) { _mouse_state.wheel_position = position; }
    const Position& mouse_wheel_position() const { return _mouse_state.wheel_position; }

    void mouse_button(MouseSym button, const Position& position, bool pressed, bool alert=true);
    bool mouse_button(MouseSym button) const { return _mouse_state.buttons[static_cast<unsigned int>(button)]; }

private:
    bool _keyboard_state[static_cast<unsigned int>(InputKeySym::Max)];
    MouseState _mouse_state;
    GamePadState _gamepad_state;

private:
    friend class Engine;

private:
    InputState();
    DISALLOW_COPY_AND_ASSIGN(InputState);
};

}

#endif
