#include "image.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
#include "context.hpp"
#include "renderer.hpp"
#include <string>
namespace doll
{
	Image::Image(std::string_view src)
	{
		createImage(src);
		createImageview();
		createTextureSampler();
	}
	Image::~Image()
	{
		Context::Instance().device.waitIdle();
		if (stagingBuffer_)
		{
			stagingBuffer_.reset();
		}
		if (stagingBufferMemory_)
		{
			stagingBufferMemory_.reset();
		}
		if (imageview_)
		{
			imageview_.reset();
		}
		if (textureImage_)
		{
			textureImage_.reset();
		}
		if (textureImageMemory_)
		{
			textureImageMemory_.reset();
		}
		if (sampler_)
		{
			sampler_.reset();
		}
	}
	vk::Image Image::getImage()
	{
		return textureImage_.get();
	}
	vk::Buffer Image::getBuffer()
	{
		return stagingBuffer_.get();
	}
	void Image::Destroy()
	{
		if (stagingBuffer_)
		{
			stagingBuffer_.reset();
		}
		if (stagingBufferMemory_)
		{
			stagingBufferMemory_.reset();
		}
		if (imageview_)
		{
			imageview_.reset();
		}
		if (textureImage_)
		{
			textureImage_.reset();
		}
		if (textureImageMemory_)
		{
			textureImageMemory_.reset();
		}
		if (sampler_)
		{
			sampler_.reset();
		}
	}
	void Image::createImage(std::string_view src)
	{
		auto& device = Context::Instance().device;
		int texChannels;

		stbi_uc* pixels = stbi_load(src.data(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

		/*std::cout << "File: " << src.data() << std::endl;
		std::cout << "Width: " << texWidth << std::endl;
		std::cout << "Height: " << texHeight << std::endl;
		std::cout << "Channels: " << texChannels << std::endl;*/

		if (!pixels) {
			std::cerr << "stbi_load error: " << stbi_failure_reason() << std::endl;
			throw std::runtime_error("Failed to load texture image!");
		}

		imageSize = texWidth * texHeight * 4;

		vk::BufferCreateInfo bufferInfo;
		bufferInfo.setSize(imageSize)
			.setUsage(vk::BufferUsageFlagBits::eTransferSrc)
			.setSharingMode(vk::SharingMode::eExclusive);
		stagingBuffer_ = device.createBufferUnique(bufferInfo);

		auto requirements = device.getBufferMemoryRequirements(stagingBuffer_.get());

		auto property = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;

		auto properties = Context::Instance().physicaldevice.getMemoryProperties();
		uint32_t memTypeIndex;
		for (int i = 0; i < properties.memoryTypeCount; ++i)
		{
			if ((1 << i) & requirements.memoryTypeBits &&
				properties.memoryTypes[i].propertyFlags & property)
			{
				memTypeIndex = i;
				break;
			}
		}
		vk::MemoryAllocateInfo allocInfo;
		allocInfo.setAllocationSize(requirements.size)
			.setMemoryTypeIndex(memTypeIndex);
		stagingBufferMemory_ = device.allocateMemoryUnique(allocInfo);

		device.bindBufferMemory(stagingBuffer_.get(), stagingBufferMemory_.get(),0);

		void* ptr = device.mapMemory(stagingBufferMemory_.get(), 0, imageSize);
		memcpy(ptr, pixels, imageSize);
		device.unmapMemory(stagingBufferMemory_.get());

		std::cout << imageSize << requirements.size << std::endl;
		

		//

		vk::ImageCreateInfo createInfo;
		createInfo.setUsage(vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst)
			.setImageType(vk::ImageType::e2D)
			.setExtent(vk::Extent3D(static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight), 1))
			.setMipLevels(1)
			.setArrayLayers(1)
			.setInitialLayout(vk::ImageLayout::eUndefined)
			.setFormat(vk::Format::eR8G8B8A8Srgb)
			.setTiling(vk::ImageTiling::eOptimal)
			.setSharingMode(vk::SharingMode::eExclusive)
			.setSamples(vk::SampleCountFlagBits::e1);
		textureImage_ = device.createImageUnique(createInfo);

		auto imageRequirements = device.getImageMemoryRequirements(textureImage_.get());

		auto imageProperty = vk::MemoryPropertyFlagBits::eDeviceLocal;

		auto imageProperties = Context::Instance().physicaldevice.getMemoryProperties();
		for (int i = 0; i < properties.memoryTypeCount; ++i)
		{
			if ((1 << i) & imageRequirements.memoryTypeBits &&
				imageProperties.memoryTypes[i].propertyFlags & imageProperty)
			{
				memTypeIndex = i;
				break;
			}
		}
		vk::MemoryAllocateInfo imageAllocInfo;
		imageAllocInfo.setAllocationSize(imageRequirements.size)
			.setMemoryTypeIndex(memTypeIndex);
		textureImageMemory_ = device.allocateMemoryUnique(imageAllocInfo);

		device.bindImageMemory(textureImage_.get(), textureImageMemory_.get(), 0);

		ptr = device.mapMemory(stagingBufferMemory_.get(), 0, imageSize);
		memcpy(ptr, pixels, imageSize);
		device.unmapMemory(stagingBufferMemory_.get());

		stbi_image_free(pixels);
	}
	
	void Image::createImageview()
	{
		vk::ImageSubresourceRange range;
		range.setAspectMask(vk::ImageAspectFlagBits::eColor)
			.setBaseArrayLayer(0)
			.setBaseMipLevel(0)
			.setLayerCount(1)
			.setLevelCount(1);

		vk::ImageViewCreateInfo createInfo;
		createInfo.setImage(textureImage_.get())
			.setViewType(vk::ImageViewType::e2D)
			.setFormat(vk::Format::eR8G8B8A8Srgb)
			.setSubresourceRange(range);

		imageview_ = Context::Instance().device.createImageViewUnique(createInfo);
	}
	
	void Image::createTextureSampler()
	{
		auto p = Context::Instance().physicaldevice.getProperties();
		
		vk::SamplerCreateInfo createInfo;
		createInfo.setMagFilter(vk::Filter::eLinear)
			.setMinFilter(vk::Filter::eLinear)
			.setAddressModeU(vk::SamplerAddressMode::eRepeat)
			.setAddressModeV(vk::SamplerAddressMode::eRepeat)
			.setAddressModeW(vk::SamplerAddressMode::eRepeat)
			.setAnisotropyEnable(false)
			.setMaxAnisotropy(p.limits.maxSamplerAnisotropy)
			.setUnnormalizedCoordinates(false)
			.setBorderColor(vk::BorderColor::eFloatOpaqueWhite)
			.setCompareEnable(false)
			.setCompareOp(vk::CompareOp::eAlways)
			.setMipmapMode(vk::SamplerMipmapMode::eLinear)
			.setMipLodBias(0.0f)
			.setMinLod(0.0f)
			.setMaxLod(0.0f);

		sampler_ = Context::Instance().device.createSamplerUnique(createInfo);
	}

	vk::ImageView Image::getImageView()
	{
		return imageview_.get();
	}
	vk::Sampler Image::getSampler()
	{
		return sampler_.get();
	}
}