#pragma once
#include <vulkan/vulkan.hpp>
#include <memory>

#include <iostream>
#include <vector>
#include <optional>
#include <functional>

#include "swapchain.hpp"
#include "renderprocess.hpp"
#include "renderer.hpp"

using CreateSurfaceFunc = std::function<vk::SurfaceKHR(vk::Instance)>;

struct QueueFamliyIndices final
{
	std::optional<uint32_t> graphicsQueue;
	std::optional<uint32_t> presentQueue;

	operator bool() {
		return graphicsQueue.has_value() and presentQueue.has_value();
	}
};

namespace doll {
	/*const uint32_t W_WIDTH = 1920;
	const uint32_t W_HEIGHT = 1080;*/

	class Context final
	{
	public:
		static void Init(std::vector<const char*> extensions, CreateSurfaceFunc func);
		static void Quit();
		static Context& Instance();
		void initSwapchain(int w,int h){swapchain.reset(new Swapchain(w, h));}
		void DestroySwapchain() { swapchain.reset(); }
		void initRenderer();

		static vk::Format findDepthFormat();

		~Context();
	private:
		Context(std::vector<const char*> extensions,CreateSurfaceFunc func);
		void createInstance(std::vector<const char*> extensions);
		void pickupPhysicalDevice();
		void createDevice();
		void queryQueueFamliyindices();
		void getQueue();
		static bool hasStencilComponent(vk::Format format);
		static vk::Format findSupportedFormat(const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features);
	public:
		std::vector<const char*> layers;
		vk::Instance instance;
		vk::PhysicalDevice physicaldevice;
		vk::Device device;
		QueueFamliyIndices queueFamliyIndices;
		vk::Queue graphicsQueue;
		vk::Queue presentQueue;
		vk::SurfaceKHR surface;
		std::unique_ptr<Swapchain> swapchain;
		std::unique_ptr<RenderProcess> renderProcess;
		std::unique_ptr<Renderer> renderer;

	private:
		//GLFWwindow* window;
		


		static Context* instance_;
	};

}
