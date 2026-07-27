#include "Core/Window.h"
#include "Renderer/vk_context.h"
#include <iostream>

int main() {
    std::cout << "We are all alone on life's journey, held captive by the limits of human conciousness.\n";
    
    if (!Window::Init(1920, 1080, "hi")) {
        return -1;
    }

    if (!VkContext::Init()) {
        return -1;
    }

    return 0;
}
