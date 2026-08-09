#include "Core/Window.h"
#include "Render/vk_context.h"
#include <iostream>

int main() {
    std::cout << "We are all alone on life's journey, held captive by the limits of human conciousness.\n";
    
    if (!Window::Init(800, 600, "hi")) {
        return -1;
    }

    if (!vk_context::Init()) {
        return -1;
    }

    while (!Window::ShouldClose()) {
        Window::BeginFrame();
    }

    return 0;
}
