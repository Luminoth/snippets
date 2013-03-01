#if !defined __UICONTROLLER_H__
#define __UICONTROLLER_H__

#include "src/core/math/Vector.h"
#include "InputSym.h"

namespace energonsoftware {

class UIController
{
private:
    static std::shared_ptr<UIController> current_controller;
    static std::stack<std::shared_ptr<UIController>> controller_stack;

public:
    static std::shared_ptr<UIController> controller() { return current_controller; }
    static void controller(std::shared_ptr<UIController> controller) { current_controller = controller; }
    static void push_controller() { controller_stack.push(current_controller); }
    static void pop_controller() { current_controller = controller_stack.top(); controller_stack.pop(); }
    static void release_controllers();

public:
    virtual ~UIController() noexcept;

public:
    virtual void key_down(InputKeySym key) {}
    virtual void key_up(InputKeySym key) {}
    virtual void mouse_button_down(MouseSym button, const Position& position) {}
    virtual void mouse_button_up(MouseSym button, const Position& position) {}

    // NOTE: be careful when making use of the elapsed time
    // it can be applied twice to Actors and such in some circumstances
    virtual void update(double elapsed) {}

    // these modify the *OS* input, not the *game* input
    virtual void grab_input(bool grab) {}
    virtual void show_mouse_cursor(bool show) {}
    virtual void warp_mouse(const Position& position) {}

protected:
    UIController();

private:
    DISALLOW_COPY_AND_ASSIGN(UIController);
};

}

#endif
