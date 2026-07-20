#include <iostream>
#include <glm/glm.hpp>
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

int main() {
    if (!glfwInit()) {
        std::cout << "failed to initialize glfw\n";
    }
    glm::vec2 v = {1,1};
    std::cout << v.x << std::endl;
    std::cout << v.y << std::endl;
    uint32_t version;
    vkEnumerateInstanceVersion(&version);
    std::cout << "vk version: " << version << std::endl;
    return 0;
}
