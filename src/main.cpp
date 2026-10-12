#include "Core/Window.h"
#include "Render/vk_context.h"
#include "Render/Types/Shader.h"
#include "Render/Renderer.h"
#include "Core/Input.h"
#include "Camera/first_person.h"
#include "Application/Application.h"
#include <iostream>

int main() {
    std::cout << "We are all alone on life's journey, held captive by the limitations of human conciousness.\n";
    
    if (!Window::Init(800, 600, "hi")) {
        return -1;
    }

    if (!slang_context::InitSlangSession()) {
        return -1;
    }

    Application::Init();

    if (!vk_context::Init()) {
        return -1;
    }

    Renderer::PrepareSceneResources();

    FpsCamera camera(glm::vec3(0.0f, 0.0f, 30.0f));
    double last_frame = glfwGetTime();

    while (!Window::ShouldClose()) {
        Window::BeginFrame();
        double current_frame = glfwGetTime();
        float delta_time = static_cast<float>(current_frame - last_frame);
        last_frame = current_frame;

        camera.update(delta_time);
        Renderer::Render(camera);
        Input::Update();
    }

    Renderer::Shutdown();
    vk_context::Shutdown();
    Window::Shutdown();

    return 0;
}
