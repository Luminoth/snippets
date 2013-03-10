#include "src/pch.h"
#include "UIController.h"

namespace energonsoftware {

std::shared_ptr<UIController> UIController::current_controller(new UIController());
std::stack<std::shared_ptr<UIController>> UIController::controller_stack;

void UIController::release_controllers()
{
    current_controller.reset(new UIController());
    while(!controller_stack.empty()) {
        controller_stack.pop();
    }
}

UIController::UIController()
{
}

UIController::~UIController() noexcept
{
}

}
