#include "Core/Window.h"
#include "Render/vk_context.h"
#include "Render/Types/Shader.h"
#include "Render/Renderer.h"
#include "Input/Input.h"
#include "Camera/Arcball.h"
#include "Application/Application.h"
#include <iostream>

int main() {
    std::cout << "We are all alone on life's journey, held captive by the limits of human conciousness.\n";
    
    if (!Window::Init(800, 600, "hi")) {
        return -1;
    }

    Arcball::Init();

    if (!slang_context::InitSlangSession()) {
        return -1;
    }

    Application::Init();

    if (!vk_context::Init()) {
        return -1;
    }

    while (!Window::ShouldClose()) {
        Window::BeginFrame();
        Arcball::Update();
        Renderer::Render();
        Input::Update();
    }

    vk_context::Shutdown();

    return 0;
}
