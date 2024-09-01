#pragma once
#include "vulkan/vulkan.hpp"


namespace doll
{
	class Image
	{
	public:
		Image(std::string_view src);
		~Image();
		vk::Image getImage();
		vk::Buffer getBuffer();
		void Destroy();
		vk::ImageView getImageView();
		vk::Sampler getSampler();
		static void createImage(int width,int height,vk::Format format,vk::ImageTiling tiling,vk::ImageUsageFlagBits usage,vk::MemoryPropertyFlagBits memPro,vk::UniqueImage& image,vk::UniqueDeviceMemory& memory);
		static void createImageView(vk::Image image, vk::Format format,vk::ImageAspectFlags aspect,vk::UniqueImageView& imageView);
	private:
		void createImage(std::string_view src);
		void createImageview();
		void createTextureSampler();
	public:
		uint32_t imageSize;
		int texWidth;
		int texHeight;
	private:
		vk::UniqueBuffer stagingBuffer_;
		vk::UniqueDeviceMemory stagingBufferMemory_;
		vk::UniqueImage textureImage_;
		vk::UniqueDeviceMemory textureImageMemory_;
		vk::UniqueImageView imageview_;
		vk::UniqueSampler sampler_;
	};

}