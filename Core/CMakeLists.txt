# CMakeList.txt: LearnVulkan 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#

project (Core)



#add_library (Core STATIC   "HelloTriangleApplication.h" "HelloTriangleApplication.cpp" "tool.h" "core.cpp" "core.hpp" "core.cpp")

file(GLOB_RECURSE HEADER_FILES "*.hpp")
file(GLOB_RECURSE SOURCE_FILES "*.cpp")

list(REMOVE_ITEM HEADER_FILES "${CMAKE_CURRENT_SOURCE_DIR}/HelloTriangleApplication.hpp")
list(REMOVE_ITEM SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/HelloTriangleApplication.cpp")

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${HEADER_FILES} ${SOURCE_FILES})

add_library(Core STATIC ${HEADER_FILES} ${SOURCE_FILES})

set_property(TARGET Core PROPERTY CXX_STANDARD 17)


# set the director of vulkan
set(VULKAN_DIR "F:/Vulkan")

target_include_directories(Core PUBLIC ${PROJECT_SOURCE_DIR})
target_include_directories(Core PUBLIC ${PROJECT_SOURCE_DIR}/lib/glm)
target_include_directories(Core PUBLIC ${VULKAN_DIR}/Include)
target_include_directories(Core PUBLIC ${PROJECT_SOURCE_DIR}/lib/glfw/include)

target_link_libraries(Core ${PROJECT_SOURCE_DIR}/lib/glfw/lib-vc2022/glfw3.lib)

target_link_libraries(Core ${VULKAN_DIR}/Lib/vulkan-1.lib)

#add_compile_definitions(DEBUG)
target_compile_definitions(Core PRIVATE $<$<CONFIG:Debug>:DEBUG>)

# TODO: 如有需要，请添加测试并安装目标。
