#include "Core/Window.h"
#include "Render/vk_context.h"
#include "Render/Types/Shader.h"
#include "Render/Renderer.h"
#include "Core/Input.h"
#include "Camera/first_person.h"
#include "Application/Application.h"
#include <iostream>

int main(int argc, char** argv) {
    if (argc > 2) {
        std::cerr << "Usage: vk-renderer [scene.glb]\n";
        return -1;
    }
    std::cout << "We are all alone on life's journey, held captive by the limitations of human consciousness.\n";
    
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

    if (!Renderer::PrepareSceneResources(argc > 1 ? argv[1] : "res/Bistro.glb")) {
        Renderer::Shutdown();
        vk_context::Shutdown();
        Window::Shutdown();
        return -1;
    }

    FpsCamera camera(glm::vec3(0.0f, 0.0f, 30.0f));
    double last_frame = glfwGetTime();

    uint32_t frames = 0;
    double elapsed_time = glfwGetTime();
    while (!Window::ShouldClose()) {
        Window::BeginFrame();
        double current_frame = glfwGetTime();
        float delta_time = static_cast<float>(current_frame - last_frame);
        last_frame = current_frame;

        camera.update(delta_time);
        Renderer::Render(camera);
        Input::Update();
        frames++;
    }
    elapsed_time += glfwGetTime();

    Renderer::Shutdown();
    vk_context::Shutdown();
    Window::Shutdown();
    std::cout << "[INFO::MAIN] completed " << frames << " frames\n";
    std::cout << "[INFO::MAIN] average fps: " << frames / elapsed_time << '\n';

    return 0;
}
