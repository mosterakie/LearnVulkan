#include"doll/context.hpp"

namespace doll {
	Context* Context::instance_ = nullptr;

	void Context::Init(std::vector<const char*> extensions, CreateSurfaceFunc func) {
		instance_ = new Context(extensions, func);
	}
	void Context::Quit() {
		delete instance_;
	}

	Context& Context::Instance()
	{
		return *instance_;
	}

	Context::Context(std::vector<const char*> extensions,CreateSurfaceFunc func) {
		createInstance(extensions);
		pickupPhysicalDevice();
		surface = func(instance);
		queryQueueFamliyindices();
		createDevice();
		getQueue();
		renderProcess.reset(new RenderProcess());
	}

	Context::~Context()
	{
		
		vkDestroySurfaceKHR(instance, surface, nullptr);//try use UniquesurfaceKHR
		device.destroy();
		instance.destroy();
	}

	


	void Context::createInstance(std::vector<const char*> extensions)
	{
		layers = { "VK_LAYER_KHRONOS_validation" };
		/*auto layers = vk::enumerateInstanceLayerProperties();
		for (auto& layer : layers)
		{
			std::cout << layer.layerName << std::endl;
		}*/
		vk::InstanceCreateInfo createInfo;
		vk::ApplicationInfo appInfo;
		appInfo.setApiVersion(VK_API_VERSION_1_3)
			.setPEngineName("GLFW");

		createInfo.setPApplicationInfo(&appInfo)
			.setPEnabledLayerNames(layers)
			.setPEnabledExtensionNames(extensions)
			;
		instance = vk::createInstance(createInfo);
	}

	void Context::pickupPhysicalDevice()
	{
		auto devices = instance.enumeratePhysicalDevices();
		physicaldevice = devices[0];
		std::cout << physicaldevice.getProperties().deviceName;
	}

	void Context::createDevice()
	{
		std::array extensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
		vk::DeviceCreateInfo createInfo;
		std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
		float priorities = 1.0;
		if (queueFamliyIndices.presentQueue.value() == queueFamliyIndices.graphicsQueue.value())
		{
			vk::DeviceQueueCreateInfo queueCreateInfo;
			queueCreateInfo.setQueuePriorities(priorities)
				.setQueueCount(1)
				.setQueueFamilyIndex(queueFamliyIndices.graphicsQueue.value())
				;
			queueCreateInfos.push_back(std::move(queueCreateInfo));
		}
		else
		{
			vk::DeviceQueueCreateInfo queueCreateInfo;
			queueCreateInfo.setQueuePriorities(priorities)
				.setQueueCount(1)
				.setQueueFamilyIndex(queueFamliyIndices.graphicsQueue.value())
				;
			queueCreateInfos.push_back(std::move(queueCreateInfo));
			queueCreateInfo.setQueuePriorities(priorities)
				.setQueueCount(1)
				.setQueueFamilyIndex(queueFamliyIndices.graphicsQueue.value())
				;
			queueCreateInfos.push_back(std::move(queueCreateInfo));
		}
		createInfo.setQueueCreateInfos(queueCreateInfos)
			.setPEnabledExtensionNames(extensions)
			;

		device = physicaldevice.createDevice(createInfo);
	}

	void Context::queryQueueFamliyindices()
	{
		auto properties = physicaldevice.getQueueFamilyProperties();
		for (int i=0;i<properties.size();++i)
		{
			if (properties[i].queueFlags & vk::QueueFlagBits::eGraphics)
			{
				queueFamliyIndices.graphicsQueue = i;
			}
			if (physicaldevice.getSurfaceSupportKHR(i, surface))
			{
				queueFamliyIndices.presentQueue = i;
			}

			if (queueFamliyIndices)
				break;
		}
	}

	void Context::getQueue()
	{
		graphicsQueue = device.getQueue(queueFamliyIndices.graphicsQueue.value(),0);
		presentQueue = device.getQueue(queueFamliyIndices.presentQueue.value(), 0);
	}

	void Context::initRenderer()
	{
		renderer.reset(new Renderer());
	}


	vk::Format Context::findSupportedFormat(const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features) {

		for (auto format : candidates) {
			auto props = Context::Instance().physicaldevice.getFormatProperties(format);
			if (tiling == vk::ImageTiling::eLinear && (props.optimalTilingFeatures & features) == features)
			{
				return format;
			}
			else if (tiling == vk::ImageTiling::eOptimal && (props.optimalTilingFeatures & features) == features)
			{
				return format;
			}
		}
		throw std::runtime_error("failed to find supported format!");
	}

	vk::Format Context::findDepthFormat()
	{
		return findSupportedFormat(
			{ vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint },
			vk::ImageTiling::eOptimal,
			vk::FormatFeatureFlagBits::eDepthStencilAttachment
		);
	}

	bool Context::hasStencilComponent(vk::Format format) {
		return format == vk::Format::eD32SfloatS8Uint || format == vk::Format::eD24UnormS8Uint;
	}
}


