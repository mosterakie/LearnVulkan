#include "swapchain.hpp"
#include "context.hpp"

namespace doll {
	Swapchain::Swapchain(int w,int h)
	{
		queryinfo(w,h);
		vk::SwapchainCreateInfoKHR createInfo;
		createInfo.setClipped(true)
			.setImageArrayLayers(1)
			.setImageUsage(vk::ImageUsageFlagBits::eColorAttachment)
			.setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
			.setSurface(Context::Instance().surface)
			.setImageColorSpace(info.format.colorSpace)
			.setImageFormat(info.format.format)
			.setImageExtent(info.imageExtent)
			.setMinImageCount(info.imagecount)
			.setPresentMode(info.presentmode)
			;
		auto& queueIndices = Context::Instance().queueFamliyIndices;
		if (queueIndices.graphicsQueue.value() == queueIndices.presentQueue.value())
		{
			createInfo.setQueueFamilyIndices(queueIndices.graphicsQueue.value())
				.setImageSharingMode(vk::SharingMode::eExclusive)
				;

		}
		else {
			std::array indices = {queueIndices.graphicsQueue.value(),queueIndices.presentQueue.value()};
			createInfo.setQueueFamilyIndices(indices)
				.setImageSharingMode(vk::SharingMode::eConcurrent)
				;
		}
		swapchain = Context::Instance().device.createSwapchainKHR(createInfo);

		getImages();
		createImageViews();
	}
	Swapchain::~Swapchain()
	{
		for (auto& imageView : imageViews)
		{
			Context::Instance().device.destroyImageView(imageView);
		}
		Context::Instance().device.destroySwapchainKHR(swapchain);
	}
	void Swapchain::queryinfo(int w,int h)
	{
		auto& physicalDevice = Context::Instance().physicaldevice;
		auto& surface = Context::Instance().surface;
		auto formats = physicalDevice.getSurfaceFormatsKHR(surface);
		info.format = formats[0];
		for (const auto& format : formats)
		{
			if (format == vk::Format::eR8G8B8A8Srgb
				and format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
			{
				info.format = format;
				break;
			}
		}
		auto capabilities = physicalDevice.getSurfaceCapabilitiesKHR(surface);
		info.imagecount = std::clamp<uint32_t>(2, capabilities.minImageCount, capabilities.maxImageCount);
		info.imageExtent.width = std::clamp<uint32_t>(w, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		info.imageExtent.height = std::clamp<uint32_t>(h, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
		info.transform = capabilities.currentTransform;
		auto presents = physicalDevice.getSurfacePresentModesKHR(surface);
		info.presentmode = vk::PresentModeKHR::eFifo;
		for (const auto& present : presents)
		{
			if (present == vk::PresentModeKHR::eMailbox)
			{
				info.presentmode = present;
				break;
			}
		}
	}
	void Swapchain::getImages()
	{
		images = Context::Instance().device.getSwapchainImagesKHR(swapchain);
	}
	void Swapchain::createImageViews()
	{
		imageViews.resize(images.size());
		for (int i = 0; i < images.size(); ++i)
		{
			vk::ImageViewCreateInfo createInfo;
			vk::ComponentMapping mapping;
			vk::ImageSubresourceRange range;
			range.setBaseMipLevel(0)
				.setLevelCount(1)
				.setBaseArrayLayer(0)
				.setLayerCount(1)
				.setAspectMask(vk::ImageAspectFlagBits::eColor)
				;
			createInfo.setImage(images[i])
				.setComponents(mapping)
				.setViewType(vk::ImageViewType::e2D)
				.setFormat(info.format.format)
				.setSubresourceRange(range)
				;
			imageViews[i] = Context::Instance().device.createImageView(createInfo);
		}
	}

	void Swapchain::CreateFramebuffers(int w,int h)
	{
		framebuffers.resize(images.size());
		for (int i=0;i<framebuffers.size();++i)
		{
			vk::FramebufferCreateInfo createInfo;
			createInfo.setAttachments(imageViews[i])
				.setWidth(w)
				.setHeight(h)
				.setRenderPass(Context::Instance().renderProcess->renderpass.get())
				.setLayers(1)
				;
			framebuffers[i] = Context::Instance().device.createFramebufferUnique(createInfo);
		}
	}
}