# Vulkan

**Vulkan倾向于通过结构体传递信息**，我们需要填写一个或多个结构体来提供足够的信息创建Vulkan实例。

**Vulkan是平台无关的API**，所以需要一个和窗口系统交互的扩展

#### **VkStructureType：sType**

>VkStructureType - Vulkan structure types (pname:sType)
>
>每个值对应于具有成员的特定结构 具有匹配的名称。去掉vk，在名称前加上VK_STRUCTURE_TYPE_，以__链接每个单词同时保持全大写。

#### **pNext**

>`pNext` is `NULL` or a pointer to a structure extending this structure.
>
>**链式扩展**: Vulkan中的许多结构体，包括`VkApplicationInfo`，都具有`pNext`指针，它可以指向另一个结构体（通常是扩展结构体）。这些扩展结构体通过链式方式连接起来，形成一个扩展链。
>
>**扩展机制**: 如果你想使用一个Vulkan扩展（例如，某个特定平台或供应商的扩展），可以将相应的扩展结构体附加到`pNext`。当Vulkan API调用处理这些结构体时，它们会遍历`pNext`链，识别和处理每个结构体中的信息。
>
>如：我们启用校验层时就将VkInstanceCreateInfo的pNext设置为了(VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo；

#### VulkanInstance

调用**vkCreateInstance**创建、析构

```c++
// Provided by VK_VERSION_1_0
VkResult vkCreateInstance(
    const VkInstanceCreateInfo*                 pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkInstance*                                 pInstance);

// Provided by VK_VERSION_1_0
void vkDestroyInstance(
    VkInstance                                  instance,
    const VkAllocationCallbacks*                pAllocator);
```

**VkInstanceCreateInfo结构体**

```c++
// Provided by VK_VERSION_1_0
typedef struct VkInstanceCreateInfo {
    VkStructureType             sType;
    const void*                 pNext;
    VkInstanceCreateFlags       flags;
    const VkApplicationInfo*    pApplicationInfo;
    uint32_t                    enabledLayerCount;
    const char* const*          ppEnabledLayerNames;
    uint32_t                    enabledExtensionCount;
    const char* const*          ppEnabledExtensionNames;
} VkInstanceCreateInfo;
```

**VkApplicationInfo结构体**

```c++
// Provided by VK_VERSION_1_0
typedef struct VkApplicationInfo {
    VkStructureType    sType;
    const void*        pNext;
    const char*        pApplicationName;
    uint32_t           applicationVersion;
    const char*        pEngineName;
    uint32_t           engineVersion;
    uint32_t           apiVersion;
} VkApplicationInfo;
```



#### **Validation layers**校验层

Vulkan库本身并没有提供任何内建的校验层，使用LunarG的Vulkan SDK提供的校验层实现。

Vulkan可以使用两种不同类型的校验层：实例校验层和设备校验层。

实例校验层只检查和全局Vulkan对象相关的调用，比如Vulkan实例。

设备校验层只检查和特定GPU相关的调用。*设备校验层现在已经不推荐使用，也就是说，应该使用实例校验层来检测所有的Vulkan调用。*

使用：

>首先显式定义可用层字符串数组，其中是需要启用的layer。在创建其它的结构体时(如启用校验层创建Vulkan实例时)会需要传入enabledLayerCount和ppEnabledLayerNames。
>
>这里要检查需要启用的层是否可用，如checkValidationLayerSupport函数，使用双层循环对两个数组进行比较。



##### VK EXT debug utils扩展

仅仅启用校验层并没有任何用处，我们不能得到任何有用的调试信息。我们使用VK EXT debug utils(对应的宏为：VK_EXT_DEBUG_UTILS_EXTENSION_NAME)扩展，设置回调函数来接受调试信息。

*使用macro可以避免在输入字符串时打错字*

使用：

>这里同样需要定义一个字符串数组，不过与validation layers不同，这里的数组只在这里使用，所以不会是全局变量。
>
>首先需要通过glfw得到vulkan在平台需要的扩展，然后在数组的末尾添加VK_EXT_DEBUG_UTILS_EXTENSION_NAME；
>
>*getRequiredExtensions函数在目前在应用中只使用了一次，如果后续没有其它使用可以删了*

接受调试信息的回调函数

```c++
VKAPI_ATTR VkBool32 VKAPI_CALL HelloTriangleApplication::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) 
{
    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

    return VK_FALSE;
}
```

使用VkDebugUtilsMessengerEXT对象来存储回调函数信息（定义在主类

```c++
// Provided by VK_EXT_debug_utils
typedef struct VkDebugUtilsMessengerCreateInfoEXT {
    VkStructureType                         sType;
    const void*                             pNext;
    VkDebugUtilsMessengerCreateFlagsEXT     flags;
    VkDebugUtilsMessageSeverityFlagsEXT     messageSeverity;
    VkDebugUtilsMessageTypeFlagsEXT         messageType;
    PFN_vkDebugUtilsMessengerCallbackEXT    pfnUserCallback;
    void*                                   pUserData;
} VkDebugUtilsMessengerCreateInfoEXT;
```

设置Vulkan使用回调函数

在createInfo的指定pfnUserCallback为debugCallback, 

```c++
void HelloTriangleApplication::setupDebugMessenger() {
    if (!enableValidationLayers) return;
    VkDebugUtilsMessengerCreateInfoEXT createInfo;
    populateDebugMessengerCreateInfo(createInfo);

    if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
        RTE("failed to set up debug messenger!")
    }
}

void HelloTriangleApplication::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
}

// Provided by VK_EXT_debug_utils
VkResult vkCreateDebugUtilsMessengerEXT(
    VkInstance                                  instance,
    const VkDebugUtilsMessengerCreateInfoEXT*   pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDebugUtilsMessengerEXT*                   pMessenger); //即自己定义的回调函数

// Dynamically load the function pointer.
// This is necessary because the function may not be available if the extension is not supported.
// 因为启用校验层后才会被是使用，所以vkCreateDebugUtilsMessengerEXT、vkDestroyDebugUtilsMessengerEXT都不会被自动加载
VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    }
    else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}
```

##### vkGetInstanceProcAddr

```c++
// Provided by VK_VERSION_1_0
PFN_vkVoidFunction vkGetInstanceProcAddr(
    VkInstance                                  instance,
    const char*                                 pName);
```







#### PhysicalDevice

创建VkInstance后，我们需要查询系统中的显卡设备，选择一个支持我们需要的特性的设备使用。

VkPhysicalDevice对象用来存储显卡信息，且在Instance清除时会自动清除自己。

获取方法和之前一样，先获取数量，创建对应数量的数组再获取一遍。

```c++
uint32_t deviceCount = 0;
vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

if (deviceCount == 0)
{
    throw std::runtime_error("Failed to find GPUs with Vulkan support!");
}

std::vector<VkPhysicalDevice> devices(deviceCount);
vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());
```



之后怕判断该物理设备是否满足我们的要求，再赋值给类中的对象。



##### 设备需求检测

```c++
//vkGetPhysicalDeviceProperties获取物理设备的基本信息
// Provided by VK_VERSION_1_0
void vkGetPhysicalDeviceProperties(
    VkPhysicalDevice                            physicalDevice,
    VkPhysicalDeviceProperties*                 pProperties);

// Provided by VK_VERSION_1_0
typedef struct VkPhysicalDeviceProperties {
    uint32_t                            apiVersion;
    uint32_t                            driverVersion;
    uint32_t                            vendorID;
    uint32_t                            deviceID;
    VkPhysicalDeviceType                deviceType;
    char                                deviceName[VK_MAX_PHYSICAL_DEVICE_NAME_SIZE];
    uint8_t                             pipelineCacheUUID[VK_UUID_SIZE];
    VkPhysicalDeviceLimits              limits;
    VkPhysicalDeviceSparseProperties    sparseProperties;
} VkPhysicalDeviceProperties;


//设备特性
// Provided by VK_VERSION_1_0
void vkGetPhysicalDeviceFeatures(
    VkPhysicalDevice                            physicalDevice,
    VkPhysicalDeviceFeatures*                   pFeatures);

// Provided by VK_VERSION_1_0
typedef struct VkPhysicalDeviceFeatures {
    VkBool32    robustBufferAccess;
    VkBool32    fullDrawIndexUint32;
    VkBool32    imageCubeArray;
    VkBool32    independentBlend;
    VkBool32    geometryShader;
    VkBool32    tessellationShader;
    VkBool32    sampleRateShading;
    VkBool32    dualSrcBlend;
    VkBool32    logicOp;
    VkBool32    multiDrawIndirect;
    VkBool32    drawIndirectFirstInstance;
    VkBool32    depthClamp;
    VkBool32    depthBiasClamp;
    VkBool32    fillModeNonSolid;
    VkBool32    depthBounds;
    VkBool32    wideLines;
    VkBool32    largePoints;
    VkBool32    alphaToOne;
    VkBool32    multiViewport;
    VkBool32    samplerAnisotropy;
    VkBool32    textureCompressionETC2;
    VkBool32    textureCompressionASTC_LDR;
    VkBool32    textureCompressionBC;
    VkBool32    occlusionQueryPrecise;
    VkBool32    pipelineStatisticsQuery;
    VkBool32    vertexPipelineStoresAndAtomics;
    VkBool32    fragmentStoresAndAtomics;
    VkBool32    shaderTessellationAndGeometryPointSize;
    VkBool32    shaderImageGatherExtended;
    VkBool32    shaderStorageImageExtendedFormats;
    VkBool32    shaderStorageImageMultisample;
    VkBool32    shaderStorageImageReadWithoutFormat;
    VkBool32    shaderStorageImageWriteWithoutFormat;
    VkBool32    shaderUniformBufferArrayDynamicIndexing;
    VkBool32    shaderSampledImageArrayDynamicIndexing;
    VkBool32    shaderStorageBufferArrayDynamicIndexing;
    VkBool32    shaderStorageImageArrayDynamicIndexing;
    VkBool32    shaderClipDistance;
    VkBool32    shaderCullDistance;
    VkBool32    shaderFloat64;
    VkBool32    shaderInt64;
    VkBool32    shaderInt16;
    VkBool32    shaderResourceResidency;
    VkBool32    shaderResourceMinLod;
    VkBool32    sparseBinding;
    VkBool32    sparseResidencyBuffer;
    VkBool32    sparseResidencyImage2D;
    VkBool32    sparseResidencyImage3D;
    VkBool32    sparseResidency2Samples;
    VkBool32    sparseResidency4Samples;
    VkBool32    sparseResidency8Samples;
    VkBool32    sparseResidency16Samples;
    VkBool32    sparseResidencyAliased;
    VkBool32    variableMultisampleRate;
    VkBool32    inheritedQueries;
} VkPhysicalDeviceFeatures;
```





##### Queuefamily

一个物理设备可能有多个队列族，一个队列**族**有一个及以上个队列，但它们通常会共享相同的能力集。一个队列族通过VkQueueFlags来标识支持的操作类型。如：

>`VK_QUEUE_GRAPHICS_BIT`：队列支持图形操作（如绘制）。
>
>`VK_QUEUE_COMPUTE_BIT`：队列支持计算操作（如着色器计算）。
>
>`VK_QUEUE_TRANSFER_BIT`：队列支持数据传输操作（如内存拷贝）。
>
>`VK_QUEUE_SPARSE_BINDING_BIT`：队列支持稀疏资源操作（如稀疏内存管理）。

```c++
// Provided by VK_VERSION_1_0
void vkGetPhysicalDeviceQueueFamilyProperties(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pQueueFamilyPropertyCount,
    VkQueueFamilyProperties*                    pQueueFamilyProperties);

// Provided by VK_VERSION_1_0
typedef struct VkQueueFamilyProperties {
    VkQueueFlags    queueFlags;
    uint32_t        queueCount; //>=1
    uint32_t        timestampValidBits;
    VkExtent3D      minImageTransferGranularity;
} VkQueueFamilyProperties;

//队列族QueueFlags的类型
// Provided by VK_VERSION_1_0
typedef enum VkQueueFlagBits {
    VK_QUEUE_GRAPHICS_BIT = 0x00000001,
    VK_QUEUE_COMPUTE_BIT = 0x00000002,
    VK_QUEUE_TRANSFER_BIT = 0x00000004,
    VK_QUEUE_SPARSE_BINDING_BIT = 0x00000008,
  // Provided by VK_VERSION_1_1
    VK_QUEUE_PROTECTED_BIT = 0x00000010,
  // Provided by VK_KHR_video_decode_queue
    VK_QUEUE_VIDEO_DECODE_BIT_KHR = 0x00000020,
  // Provided by VK_KHR_video_encode_queue
    VK_QUEUE_VIDEO_ENCODE_BIT_KHR = 0x00000040,
  // Provided by VK_NV_optical_flow
    VK_QUEUE_OPTICAL_FLOW_BIT_NV = 0x00000100,
} VkQueueFlagBits;
```



#### LogicalDeivce

创建逻辑设备需要填写VkDeviceCreateInfo结构体

```C++
// Provided by VK_VERSION_1_0
VkResult vkCreateDevice(
    VkPhysicalDevice                            physicalDevice,
    const VkDeviceCreateInfo*                   pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDevice*                                   pDevice);

// Provided by VK_VERSION_1_0
typedef struct VkDeviceCreateInfo {
    VkStructureType                    sType;
    const void*                        pNext;
    VkDeviceCreateFlags                flags; //预留
    uint32_t                           queueCreateInfoCount;  //unsigned int，pQueueCreateInfos的size
    const VkDeviceQueueCreateInfo*     pQueueCreateInfos;  //指向要创建的队列信息的指针
    // enabledLayerCount is deprecated and should not be used  现在层都由实例统一管理
    uint32_t                           enabledLayerCount;
    // ppEnabledLayerNames is deprecated and should not be used
    const char* const*                 ppEnabledLayerNames; 
    uint32_t                           enabledExtensionCount;
    const char* const*                 ppEnabledExtensionNames;
    const VkPhysicalDeviceFeatures*    pEnabledFeatures;
} VkDeviceCreateInfo;


// Provided by VK_VERSION_1_0
typedef struct VkDeviceQueueCreateInfo {
    VkStructureType             sType;
    const void*                 pNext;
    VkDeviceQueueCreateFlags    flags;
    uint32_t                    queueFamilyIndex;
    uint32_t                    queueCount;
    const float*                pQueuePriorities;
} VkDeviceQueueCreateInfo;
```



##### 获取队列句柄

在创建好逻辑设备后，将队列句柄存储以便之后使用

```c++
// Provided by VK_VERSION_1_0
void vkGetDeviceQueue(
    VkDevice                                    device,
    uint32_t                                    queueFamilyIndex,
    uint32_t                                    queueIndex,
    VkQueue*                                    pQueue);
```





#### Surface

Vulkan是一个平台无关的API，它不能直接和窗口系统交互。为了将Vulkan渲染的图像显示在窗口上，我们需要使用WSI(Window System Integration)扩展。

**VkSurfaceKHR**

> VkSurfaceKHR对象是平台无关的，但它的创建依赖窗口系统。比如，在Windows系统上，它的创建需要HWND和HMODULE。
>
> 可以使用GLFW库的glfwCreateWindowSurface函数来完成表面创建。

##### 使用VK KHR win32 surface创建窗口表面

```C++
//VkWin32SurfaceCreateInfoKHR createInfo = {};
//createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
//createInfo.hwnd = glfwGetWin32Window(window);
//createInfo.hinstance = GetmMoudleHandle(nullptr);
```



##### 使用glfw创建窗口表面

```c++
if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
    RTE("failed to create window surface!")
}
```

清除Surface

```c++
// Provided by VK_KHR_surface
void vkDestroySurfaceKHR(
    VkInstance                                  instance,
    VkSurfaceKHR                                surface,
    const VkAllocationCallbacks*                pAllocator);
```



#### SwapChain

首先要保证**VK_KHR_swapchain**(在Vulkan_core中的宏为**VK_KHR_SWAPCHAIN_EXTENSION_NAME**)被设备支持。

实际上，如果设备支持呈现队列，那么它就一 定支持交换链。但我们最好还是显式地进行交换链扩展的检测，然后显式地启用交换链扩展。

*最好对判断设备是否满足需求的函数进行扩展。*

在创建交换链前，需要从物理设备取得具体的细节。

```c++
//用于存储设备对应的交换链细节
struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

// Provided by VK_KHR_surface
VkResult vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
    VkPhysicalDevice                            physicalDevice,
    VkSurfaceKHR                                surface,
    VkSurfaceCapabilitiesKHR*                   pSurfaceCapabilities);

// Provided by VK_KHR_surface
VkResult vkGetPhysicalDeviceSurfaceFormatsKHR(
    VkPhysicalDevice                            physicalDevice,
    VkSurfaceKHR                                surface,
    uint32_t*                                   pSurfaceFormatCount,
    VkSurfaceFormatKHR*                         pSurfaceFormats);

// Provided by VK_KHR_surface
VkResult vkGetPhysicalDeviceSurfacePresentModesKHR(
    VkPhysicalDevice                            physicalDevice,
    VkSurfaceKHR                                surface,
    uint32_t*                                   pPresentModeCount,
    VkPresentModeKHR*                           pPresentModes);
```

##### VkSurfaceFormatKHR

每一个VkSurfaceFormatKHR条目包含了一个format和colorSpace成员变量。

```c++
// Provided by VK_KHR_surface
typedef struct VkSurfaceFormatKHR {
    VkFormat           format;
    VkColorSpaceKHR    colorSpace;
} VkSurfaceFormatKHR;
```

- format成员变量用于指定颜色通道和存储类型。
- colorSpace成员变量用来表示SRGB颜色空间是否被支持.



##### VkPresentModeKHR

呈现模式可以说是交换链中最重要的设置。它决定了什么条件下图像才会显示到屏幕。

***VK_PRESENT_MODE_FIFO_KHR**保证一定可用*

```c++
// Provided by VK_KHR_surface
typedef enum VkPresentModeKHR {
    VK_PRESENT_MODE_IMMEDIATE_KHR = 0,   //应用程序提交的图像会被立即传输到屏幕上，可能会导致撕裂现象。
    VK_PRESENT_MODE_MAILBOX_KHR = 1, //它不会在交换链的队列满时阻塞应用程序，队列中的图像会被直接替换为应用程序新提交的图像。这一模式可以用来实现三倍缓冲，避免撕裂现象的同时减小了延迟问题。
    VK_PRESENT_MODE_FIFO_KHR = 2, //交换链变成一个先进先出的队列，每次从队列头部取出一张图像进行显示，应用程序渲染的图像提交给交换链后，会被放在队列尾部。
    VK_PRESENT_MODE_FIFO_RELAXED_KHR = 3,  //这一模式和上一模式的唯一区别是，如果应用程序延迟，导致交换链的队列在上一次垂直回扫时为空，那么，如果应用程序在下一次垂直回扫前提交图像，图像会立即被显示。
  // Provided by VK_KHR_shared_presentable_image
    VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR = 1000111000,
  // Provided by VK_KHR_shared_presentable_image
    VK_PRESENT_MODE_SHARED_CONTINUOUS_REFRESH_KHR = 1000111001,
} VkPresentModeKHR;
```

##### VkExtent2D

```c++
// Provided by VK_VERSION_1_0
typedef struct VkExtent2D {
    uint32_t    width;
    uint32_t    height;
} VkExtent2D;
```



```c++
// Provided by VK_KHR_swapchain
typedef struct VkSwapchainCreateInfoKHR {
    VkStructureType                  sType;
    const void*                      pNext;
    VkSwapchainCreateFlagsKHR        flags;
    VkSurfaceKHR                     surface;
    uint32_t                         minImageCount;
    VkFormat                         imageFormat;
    VkColorSpaceKHR                  imageColorSpace;
    VkExtent2D                       imageExtent;
    uint32_t                         imageArrayLayers;
    VkImageUsageFlags                imageUsage;
    //指定在多个队列族使用交换链图像的方式,与图形队列和呈现队列是否是同一队列有关。
    VkSharingMode                    imageSharingMode;
    uint32_t                         queueFamilyIndexCount;
    const uint32_t*                  pQueueFamilyIndices;
    //
    VkSurfaceTransformFlagBitsKHR    preTransform; //为交换链中的图像指定一个固定的变换操作
    VkCompositeAlphaFlagBitsKHR      compositeAlpha;//用于指定alpha通道是否被用来和窗口系统中的其它窗口进行混合操作
    VkPresentModeKHR                 presentMode;
    VkBool32                         clipped;//为VK TRUE表示我们不关心被窗口系统中的其它窗口遮挡的像素的颜色，这允许Vulkan采取一定的优化措施，但如果我们回读窗口的像素值就可能出现问题
    VkSwapchainKHR                   oldSwapchain; //用于重建交换链
} VkSwapchainCreateInfoKHR;

// Provided by VK_KHR_swapchain
VkResult vkCreateSwapchainKHR(
    VkDevice                                    device,
    const VkSwapchainCreateInfoKHR*             pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSwapchainKHR*                             pSwapchain);

// Provided by VK_KHR_swapchain
void vkDestroySwapchainKHR(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    const VkAllocationCallbacks*                pAllocator);
```



##### 获取交换链图像

交换链图像不同，图像视图是由我们自己显式创建的，需要我们自己在cleanup函数中清除它们

https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkImageViewCreateInfo.html

```c++
//交换链图像由交换链自己负责创建，并在交换链清除时自动被清除
// Provided by VK_KHR_swapchain
VkResult vkGetSwapchainImagesKHR(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    uint32_t*                                   pSwapchainImageCount,
    VkImage*                                    pSwapchainImages);


```



#### ImageViews

VkImageView

使用任何VkImage对象，包括处于交换链中的，处于渲染管线中的，都需要我们创建一个VkImageView对象来绑定访问它。图像视图描述了访问图像的方式，以及图像的哪一部分可以被访问。

```c++
// Provided by VK_VERSION_1_0
VkResult vkCreateImageView(
    VkDevice                                    device,
    const VkImageViewCreateInfo*                pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkImageView*                                pView);

// Provided by VK_VERSION_1_0
typedef struct VkImageViewCreateInfo {
    VkStructureType            sType;
    const void*                pNext;
    VkImageViewCreateFlags     flags;
    VkImage                    image;
    VkImageViewType            viewType; //viewType成员变量用于指定图像被看作是一维纹理、二维纹理、三维纹理还是立方体贴图
    VkFormat                   format;
    VkComponentMapping         components;//components成员变量用于进行图像颜色通道的映射。
    VkImageSubresourceRange    subresourceRange;//subresourceRange成员变量用于指定图像的用途和图像的哪一部分可以被访问
} VkImageViewCreateInfo;

// Provided by VK_VERSION_1_0
typedef struct VkComponentMapping {
    VkComponentSwizzle    r;
    VkComponentSwizzle    g;
    VkComponentSwizzle    b;
    VkComponentSwizzle    a;
} VkComponentMapping;

// Provided by VK_VERSION_1_0
typedef struct VkImageSubresourceRange {
    VkImageAspectFlags    aspectMask;
    uint32_t              baseMipLevel;
    uint32_t              levelCount;
    uint32_t              baseArrayLayer;
    uint32_t              layerCount;
} VkImageSubresourceRange;

// Provided by VK_VERSION_1_0
void vkDestroyImageView(
    VkDevice                                    device,
    VkImageView                                 imageView,
    const VkAllocationCallbacks*                pAllocator);
```





#### GraphicPipeline

##### VkShaderModule

使用vulkan提供的编译器将着色器从GLSL编译成SPIR-V字节码格式，读入管线中每个shader都要有一个shadermodule；VkShaderModule对象只是一个对着色器字节码的包装。我们还需要指定它们在管线处理哪一阶段被使用。

VkPipelineShaderStageCreateInfo每个module都需要

```c++
// Provided by VK_VERSION_1_0
VkResult vkCreateShaderModule(
    VkDevice                                    device,
    const VkShaderModuleCreateInfo*             pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkShaderModule*                             pShaderModule);

// Provided by VK_VERSION_1_0
typedef struct VkShaderModuleCreateInfo {
    VkStructureType              sType;
    const void*                  pNext;
    VkShaderModuleCreateFlags    flags;//预留
    size_t                       codeSize;
    const uint32_t*              pCode;
} VkShaderModuleCreateInfo;

// Provided by VK_VERSION_1_0
typedef struct VkPipelineShaderStageCreateInfo {
    VkStructureType                     sType;
    const void*                         pNext;
    VkPipelineShaderStageCreateFlags    flags;
    VkShaderStageFlagBits               stage;  //shader使用的阶段
    VkShaderModule                      module; //对应的shadermodule
    const char*                         pName; //pName成员变量用于指定阶段调用的着色器函数。我们可以通过使用不同pName在同一份着色器代码中实现所有需要的着色器，比如在同一份代码中实现多个片段着色器，然后通过不同的pName调用它们。
    const VkSpecializationInfo*         pSpecializationInfo; //指定着色器用到的常量，我们可以对同一个着色器模块对象指定不同的着色器常量用于管线创建，这使得编译器可以根据指定的着色器常量来消除一些条件分支，这比在渲染时，使用变量配置着色器带来的效率要高得多。
} VkPipelineShaderStageCreateInfo;
```

##### FixedFunction

许多图形API会为管线提供一些默认的状态。在Vulkan不存在默认状态，所有状态必须被显式地设置，无论是视口大小，还是使用的颜色混合函数都需要显式地指定。

VkPipelineVertexInputStateCreateInfo结构体来描述传递给顶点着色器的顶点数据格式。

VkPipelineInputAssemblyStateCreateInfo结构体用于描述两个信息：顶点数据定义了哪种类型的几何图元，以及是否启用几何图元重启。

```c++
typedef struct VkPipelineVertexInputStateCreateInfo {
    VkStructureType                             sType;
    const void*                                 pNext;
    VkPipelineVertexInputStateCreateFlags       flags;
    uint32_t                                    vertexBindingDescriptionCount;
    const VkVertexInputBindingDescription*      pVertexBindingDescriptions;
    uint32_t                                    vertexAttributeDescriptionCount;
    const VkVertexInputAttributeDescription*    pVertexAttributeDescriptions;
} VkPipelineVertexInputStateCreateInfo;
    

typedef struct VkPipelineInputAssemblyStateCreateInfo {
    VkStructureType                            sType;
    const void*                                pNext;
    VkPipelineInputAssemblyStateCreateFlags    flags;
    VkPrimitiveTopology                        topology;
    VkBool32                                   primitiveRestartEnable;
} VkPipelineInputAssemblyStateCreateInfo;
```

###### viewport

视口用于描述被用来输出渲染结果的帧缓冲区域。一般而言，会将它设置为(0，0)到(width，height)。

```
typedef struct VkViewport {
    float    x;
    float    y;
    float    width;
    float    height;
    float    minDepth;
    float    maxDepth;
} VkViewport;
```

###### scissor

裁剪矩形定义了哪一区域的像素实际被存储在帧缓存。

```c++
typedef struct VkRect2D {
    VkOffset2D    offset;
    VkExtent2D    extent;
} VkRect2D;
```

视口和裁剪还需要组合在一起

```c++
typedef struct VkPipelineViewportStateCreateInfo {
    VkStructureType                       sType;
    const void*                           pNext;
    VkPipelineViewportStateCreateFlags    flags;
    uint32_t                              viewportCount;
    const VkViewport*                     pViewports;
    uint32_t                              scissorCount;
    const VkRect2D*                       pScissors;
} VkPipelineViewportStateCreateInfo;
```



###### Rasterization

```c++
typedef struct VkPipelineRasterizationStateCreateInfo {
    VkStructureType                            sType;
    const void*                                pNext;
    VkPipelineRasterizationStateCreateFlags    flags; //预留
    VkBool32                                   depthClampEnable;//depthClampEnable成员变量设置为VK TRUE表示在近平面和远平面 外的片段会被截断为在近平面和远平面上，而不是直接丢弃这些片段。
    VkBool32                                   rasterizerDiscardEnable;//rasterizerDiscardEnable成员变量设置为VK TRUE表示所有几何图元都不能通过光栅化阶段。这一设置会禁止一切片段输出到帧缓冲。
    VkPolygonMode                              polygonMode;//polygonMode成员变量用于指定几何图元生成片段的方式。
    VkCullModeFlags                            cullMode;//cullMode成员变量用于指定使用的表面剔除类型。
    VkFrontFace                                frontFace;//frontFace成员变量用于指定顺时针的顶点序是正面，还是逆时针的顶点序是正面。
    
    //光栅化程序可以添加一个常量值或是一个基于片段所处线段的斜率得到的变量值到深度值上。这对于阴影贴图会很有用
    VkBool32                                   depthBiasEnable;
    float                                      depthBiasConstantFactor;
    float                                      depthBiasClamp;
    float                                      depthBiasSlopeFactor;
    
    float                                      lineWidth;//lineWidth成员变量用于指定光栅化后的线段宽度，它以线宽所占的片段数目为单位。
} VkPipelineRasterizationStateCreateInfo;
```



###### Multisample

```c++
typedef struct VkPipelineMultisampleStateCreateInfo {
    VkStructureType                          sType;
    const void*                              pNext;
    VkPipelineMultisampleStateCreateFlags    flags;
    VkSampleCountFlagBits                    rasterizationSamples;
    VkBool32                                 sampleShadingEnable;
    float                                    minSampleShading;
    const VkSampleMask*                      pSampleMask;
    VkBool32                                 alphaToCoverageEnable;
    VkBool32                                 alphaToOneEnable;
} VkPipelineMultisampleStateCreateInfo;
```



###### 深度测试

###### 模板测试



###### colorblend

```c++
typedef struct VkPipelineColorBlendAttachmentState {
    VkBool32                 blendEnable;
    VkBlendFactor            srcColorBlendFactor;
    VkBlendFactor            dstColorBlendFactor;
    VkBlendOp                colorBlendOp;
    VkBlendFactor            srcAlphaBlendFactor;
    VkBlendFactor            dstAlphaBlendFactor;
    VkBlendOp                alphaBlendOp;
    VkColorComponentFlags    colorWriteMask;
} VkPipelineColorBlendAttachmentState;

typedef struct VkPipelineColorBlendStateCreateInfo {
    VkStructureType                               sType;
    const void*                                   pNext;
    VkPipelineColorBlendStateCreateFlags          flags;
    VkBool32                                      logicOpEnable;
    VkLogicOp                                     logicOp;
    uint32_t                                      attachmentCount;
    const VkPipelineColorBlendAttachmentState*    pAttachments;
    float                                         blendConstants[4];
} VkPipelineColorBlendStateCreateInfo;
```



###### dynamictate

```c++
typedef struct VkPipelineDynamicStateCreateInfo {
    VkStructureType                      sType;
    const void*                          pNext;
    VkPipelineDynamicStateCreateFlags    flags;
    uint32_t                             dynamicStateCount;
    const VkDynamicState*                pDynamicStates;
} VkPipelineDynamicStateCreateInfo;
```



##### pipelinelayout

```c++
VkResult vkCreatePipelineLayout(
    VkDevice                                    device,
    const VkPipelineLayoutCreateInfo*           pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkPipelineLayout*                           pPipelineLayout);

typedef struct VkPipelineLayoutCreateInfo {
    VkStructureType                 sType;
    const void*                     pNext;
    VkPipelineLayoutCreateFlags     flags;
    uint32_t                        setLayoutCount;
    const VkDescriptorSetLayout*    pSetLayouts;
    uint32_t                        pushConstantRangeCount;
    const VkPushConstantRange*      pPushConstantRanges;
} VkPipelineLayoutCreateInfo;
```



##### Pipeline

```c++
typedef struct VkGraphicsPipelineCreateInfo {
    VkStructureType                                  sType;
    const void*                                      pNext;
    VkPipelineCreateFlags                            flags;
    uint32_t                                         stageCount;
    const VkPipelineShaderStageCreateInfo*           pStages;
    const VkPipelineVertexInputStateCreateInfo*      pVertexInputState;
    const VkPipelineInputAssemblyStateCreateInfo*    pInputAssemblyState;
    const VkPipelineTessellationStateCreateInfo*     pTessellationState;
    const VkPipelineViewportStateCreateInfo*         pViewportState;
    const VkPipelineRasterizationStateCreateInfo*    pRasterizationState;
    const VkPipelineMultisampleStateCreateInfo*      pMultisampleState;
    const VkPipelineDepthStencilStateCreateInfo*     pDepthStencilState;
    const VkPipelineColorBlendStateCreateInfo*       pColorBlendState;
    const VkPipelineDynamicStateCreateInfo*          pDynamicState;
    VkPipelineLayout                                 layout;
    VkRenderPass                                     renderPass;
    uint32_t                                         subpass;
    VkPipeline                                       basePipelineHandle;
    int32_t                                          basePipelineIndex;
} VkGraphicsPipelineCreateInfo;
```





#### renderpass

```c++
VkResult vkCreateRenderPass(
    VkDevice                                    device,
    const VkRenderPassCreateInfo*               pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkRenderPass*                               pRenderPass);

typedef struct VkRenderPassCreateInfo {
    VkStructureType                   sType;
    const void*                       pNext;
    VkRenderPassCreateFlags           flags;
    uint32_t                          attachmentCount;
    const VkAttachmentDescription*    pAttachments;
    uint32_t                          subpassCount;
    const VkSubpassDescription*       pSubpasses;
    uint32_t                          dependencyCount;
    const VkSubpassDependency*        pDependencies;
} VkRenderPassCreateInfo;
```

##### subpass

一个渲染流程可以包含多个子流程。子流程依赖于上一流程处理后的帧缓冲内容。比如，许多叠加的后期处理效果就是在上一次的处理结果上 进行的。每个子流程可以引用一个或多个附着，这些引用的附着是通过VkAttachmentReference结构体指定的

##### colorattachment



#### Framebuffer

```c++
typedef struct VkFramebufferCreateInfo {
    VkStructureType             sType;
    const void*                 pNext;
    VkFramebufferCreateFlags    flags;
    VkRenderPass                renderPass;
    uint32_t                    attachmentCount;
    const VkImageView*          pAttachments;
    uint32_t                    width;
    uint32_t                    height;
    uint32_t                    layers;
} VkFramebufferCreateInfo;
```



#### Comandpool

```c++
VkResult vkCreateCommandPool(
    VkDevice                                    device,
    const VkCommandPoolCreateInfo*              pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkCommandPool*                              pCommandPool);

typedef struct VkCommandPoolCreateInfo {
    VkStructureType             sType;
    const void*                 pNext;
    VkCommandPoolCreateFlags    flags;
    uint32_t                    queueFamilyIndex;
} VkCommandPoolCreateInfo;
```



#### CommandBuffer

```c++
VkResult vkAllocateCommandBuffers(
    VkDevice                                    device,
    const VkCommandBufferAllocateInfo*          pAllocateInfo,
    VkCommandBuffer*                            pCommandBuffers);

typedef struct VkCommandBufferAllocateInfo {
    VkStructureType         sType;
    const void*             pNext;
    VkCommandPool           commandPool;
    VkCommandBufferLevel    level;
    uint32_t                commandBufferCount;
} VkCommandBufferAllocateInfo;
```



#### recordCommandBuffer





#### drawFrame

两个信号量：一个信号量发出图像已经被获取，可以开始渲染的信号；一个信号量发出渲染已经结果，可以开始呈现的信号。

##### 从交换链获取一张图像

```
vkAcquireNextImageKHR
```



##### 提交指令缓冲

```
VkSubmitInfo

vkQueueSubmit
```



##### 子流程依赖

在渲染流程开始和结束时会自动进行图像布局变换，但在渲染流程开 始时进行的自动变换的时机和我们的需求不符，变换发生在管线开始时， 但那时我们可能还没有获取到交换链图像。

解决方式

>- 设置imageAvailableSemaphore信号量的waitStages为VK PIPELINE STAGE TOP OF PIPE BIT，确保渲染流程在我们获取交换链图像之前不会开始
>- 设置渲染流程等待VK PIPELINE STAGE COLOR ATTACHMENT OUTPUT BIT管线阶段





### Queue专题

- **Graphics Queue**

>**职责**: 图形队列用于执行图形操作，主要包括执行渲染命令，例如绘制三角形、应用着色器等。这个队列是 GPU 进行图形处理的核心队列。
>
>**支持的操作**:
>
>- **绘图指令**: 提交绘图命令，比如 `vkCmdDraw`。
>- **清除操作**: 清除颜色缓冲、深度缓冲等。
>- **其他图形操作**: 一般情况下，图形队列还支持复制（transfer）和计算（compute）操作，但这取决于具体的硬件和驱动实现。
>
>**特性**: 图形队列是功能最全面的队列类型，但在某些情况下，可能有多种不同的队列用来处理特定的任务（如计算队列、传输队列）。

- **Presentation Queue**

>**职责**: 呈现队列的主要职责是将渲染好的图像呈现到屏幕上。具体来说，它负责与窗口系统集成（如 Windows 下的 Win32、X11、Wayland 等），并将渲染好的图像显示给用户。
>
>**支持的操作**:
>
>- **呈现操作**: 通过调用 `vkQueuePresentKHR`，将图像从交换链（Swapchain）提交到窗口系统进行呈现。
>
>**特性**: 并非所有的图形队列都支持呈现操作（即并非所有支持图形操作的队列都支持 `vkQueuePresentKHR`），因此通常需要找到一个既支持图形操作又支持呈现操作的队列。

- **Transfer Queue**

>**职责**: 专门用于执行内存传输操作，如从主机内存向设备内存拷贝数据，或在不同的 GPU 资源之间进行数据拷贝。
>
>**支持的操作**:
>
>- 数据传输操作（`vkCmdCopyBuffer`, `vkCmdCopyImage`等）。
>
>**特性**: 传输队列通常是轻量级的，不支持复杂的图形或计算操作。这种队列类型通常用于加速数据传输任务，避免占用图形或计算队列的资源。

- **Compute Queue**

>**职责**: 专门用于执行计算着色器（Compute Shader）操作。它适合需要大量并行计算的任务，如物理模拟、图像处理或机器学习推理等。
>
>**支持的操作**:
>
>- 执行计算命令（`vkCmdDispatch`等）。
>
>**特性**: 计算队列通常仅支持计算操作，但在某些硬件实现中，计算队列也可能支持传输操作。
