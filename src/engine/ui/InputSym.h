#if !defined __INPUTSYM_H__
#define __INPUTSYM_H__

namespace energonsoftware {

enum class InputKeySym : unsigned int
{
    null = 0,

    // ASCII mapped keys
    Backspace = '\b',
    Tab = '\t',
    Return = '\r',
    Pause = 19,
    Escape = 27,
    Space = ' ',
    DoubleQuote = '\"',
    Hash = '#',
    Dollar = '$',
    Ampersand = '&',
    Quote = '\'',
    LeftParen = '(',
    RightParen = ')',
    Asterisk = '*',
    Plus = '+',
    Comma = ',',
    Minus = '-',
    Period = '.',
    Slash = '/',

    Num0 = '0',
    Num1,
    Num2,
    Num3,
    Num4,
    Num5,
    Num6,
    Num7,
    Num8,
    Num9,

    Colon = ':',
    Semicolon = ';',
    Less = '<',
    Equals = '=',
    Greater = '>',
    Question = '?',
    At = '@',
    LeftBracket = '[',
    RightBracket = ']',
    Backslash = '\\',
    Caret = '^',
    Underscore = '_',
    Backquote = '`',

    A = 'a',
    B,
    C,
    D,
    E,
    F,
    G,
    H,
    I,
    J,
    K,
    L,
    M,
    N,
    O,
    P,
    Q,
    R,
    S,
    T,
    U,
    V,
    W,
    X,
    Y,
    Z,

    Delete = 127,

    // non-ASCII keys
    Up = 273,
    Down,
    Right,
    Left,
    Insert,
    Home,
    End,
    PageUp,
    PageDown,

    F1 = 282,
    F2,
    F3,
    F4,
    F5,
    F6,
    F7,
    F8,
    F9,
    F10,
    F11,
    F12,
    F13,
    F14,
    F15,
    F16,

    NumLock = 300,
    CapsLock,
    ScrollLock,
    RightShift,
    LeftShift,
    RightCtrl,
    LeftCtrl,
    RightAlt,
    LeftAlt,

    Help = 315,
    Print,
    SysReq,
    Break,

    Max
};

enum class MouseSym : unsigned int
{
    Left,
    Middle,
    Right,

    Max
};

enum class GamePadSym : unsigned int
{
    Home,

    DPadUp,
    DPadDown,
    DPadLeft,
    DPadRight,

    Start,
    Back,

    BumperLeft,
    BumperRight,
    TriggerLeft,
    TriggerRight,

    A,
    B,
    X,
    Y,

    Max
};

}

#endif
