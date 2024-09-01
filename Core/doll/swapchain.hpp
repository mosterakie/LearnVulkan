#pragma once

#include "vulkan/vulkan.hpp"

namespace doll {
	class Swapchain final {
	public:
		Swapchain(int w,int h);
		~Swapchain();
		void queryinfo(int w,int h);
		void getImages();
		void createImageViews();
		void CreateFramebuffers(int w,int h);
		vk::Extent2D getExtent();
	private:
		
	public:
		vk::SwapchainKHR swapchain;
		struct SwapchainInfo {
			vk::Extent2D imageExtent;
			uint32_t imagecount;
			vk::SurfaceFormatKHR format;
			vk::SurfaceTransformFlagsKHR transform;
			vk::PresentModeKHR presentmode;
		};
		SwapchainInfo info;
		std::vector<vk::Image> images;
		std::vector<vk::ImageView> imageViews;
		std::vector<vk::UniqueFramebuffer> framebuffers;
	private:

	};
}
