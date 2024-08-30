#include "buffer.hpp"
#include "context.hpp"

namespace doll {
	Buffer::Buffer(size_t size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags property):size(size)
	{
		createBuffer(size,usage);
		auto info = queryMemoryInfo(property);
		allocateMemory(info);
		bindingMem2Buf();
	}

	Buffer::~Buffer()
	{
	}
	void Buffer::createBuffer(size_t size,vk::BufferUsageFlags usage)
	{
		vk::BufferCreateInfo createInfo;
		createInfo.setSize(size)
			.setUsage(usage)
			.setSharingMode(vk::SharingMode::eExclusive);
		buffer = Context::Instance().device.createBufferUnique(createInfo);
	}
	void Buffer::allocateMemory(MemoryInfo info)
	{
		vk::MemoryAllocateInfo allocInfo;
		allocInfo.setAllocationSize(info.size)
			.setMemoryTypeIndex(info.index);

		memory = Context::Instance().device.allocateMemoryUnique(allocInfo);
	}
	void Buffer::bindingMem2Buf()
	{
		Context::Instance().device.bindBufferMemory(buffer.get(),memory.get(),0);
	}
	Buffer::MemoryInfo Buffer::queryMemoryInfo( vk::MemoryPropertyFlags property)
	{
		MemoryInfo info;
		auto requirements = Context::Instance().device.getBufferMemoryRequirements(buffer.get());
		info.size = requirements.size;

		auto properties = Context::Instance().physicaldevice.getMemoryProperties();
		for (int i = 0; i < properties.memoryTypeCount; ++i)
		{
			if ((1 << i) & requirements.memoryTypeBits &&
				properties.memoryTypes[i].propertyFlags & property)
			{
				info.index = i;
				break;
			}
		}

		return info;
	}
}