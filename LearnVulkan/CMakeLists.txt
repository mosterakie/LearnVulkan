# CMakeList.txt: LearnVulkan 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#

project (LearnVulkan)


# 将源代码添加到此项目的可执行文件。
add_executable (LearnVulkan "main.cpp" )

set_property(TARGET LearnVulkan PROPERTY CXX_STANDARD 17)


include_directories (${PROJECT_SOURCE_DIR}/lib/glm)
include_directories (F:/Vulkan/Include)
include_directories (${PROJECT_SOURCE_DIR}/lib/glfw/include)

target_link_libraries(LearnVulkan F:/Vulkan/Lib/vulkan-1.lib
)

target_link_libraries(LearnVulkan 
    ${PROJECT_SOURCE_DIR}/lib/glfw/lib-vc2022/glfw3.lib
)



# TODO: 如有需要，请添加测试并安装目标。
