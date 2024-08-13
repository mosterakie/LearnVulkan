#include <iostream>

#include "tool.hpp"

#include "context.hpp"
#include "windowdoll.hpp"
#include "shader.hpp"
const int width = 800;
const int height = 600;

int main() {
    try
    {
        doll::G_Window w({ width,height,"vulkan" });
        auto extensions = w.getExtensions();

        /*for (auto& extension : extensions)
        {
            std::cout << extension << std::endl;
        }*/


        //INIT
        doll::Context::Init(extensions, [&](vk::Instance instance)->VkSurfaceKHR {
            VkSurfaceKHR surface;
            if (glfwCreateWindowSurface(instance, reinterpret_cast<GLFWwindow*>(w.GetNativeWindow()), nullptr, &surface) != VK_SUCCESS)
                throw std::runtime_error("failed to creaete surface!");
            return surface;
            });
        doll::Context::Getinstance().initSwapchain(width,height);
        doll::Shader::Init(doll::readFileString("F:\\dev\\LearnVulkan\\LearnVulkan\\Core\\shaders\\vert.spv"), 
                            doll::readFileString("F:\\dev\\LearnVulkan\\LearnVulkan\\Core\\shaders\\frag.spv"));
        doll::Context::Getinstance().renderProcess->InitRenderPass();
        doll::Context::Getinstance().swapchain->CreateFramebuffers(width,height);
        doll::Context::Getinstance().renderProcess->InitLayout();
        doll::Context::Getinstance().renderProcess->InitPipeline(width,height);
        doll::Context::Getinstance().initRenderer();

        auto& renderer = *doll::Context::Getinstance().renderer;
        
        while (!w.ShouldClose())
        {
            w.OnUpdate();
            renderer.DrawTriangle();
        }

        //QUIT
        //TODO::make a better solution to destruct 
        doll::Context::Getinstance().device.waitIdle();
        doll::Context::Getinstance().renderer.reset();
        doll::Context::Getinstance().renderProcess.reset();
        doll::Shader::Quit();
        doll::Context::Getinstance().DestroySwapchain();
        doll::Context::Quit();
        return EXIT_SUCCESS;
    }
    catch (const std::exception& e)
    {
        std::cout << e.what() << std::endl;
    }
    

    
}

