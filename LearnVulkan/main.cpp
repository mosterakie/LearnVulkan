// LearnVulkan.cpp: 定义应用程序的入口点。
//

#include "HelloTriangleApplication.h"


int main() {
    HelloTriangleApplication app;

    try {
        app.run();
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}