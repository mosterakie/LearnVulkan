#pragma once

#include "vulkan/vulkan.hpp"

namespace doll {
	class Buffer
	{
	public:
		vk::UniqueBuffer buffer;
		vk::UniqueDeviceMemory memory;
		size_t size;
	private:


	public:
		Buffer(size_t size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags property);
		~Buffer();

	private:
		struct MemoryInfo final
		{
			size_t size;
			uint32_t index;
		};
		void createBuffer(size_t size, vk::BufferUsageFlags usage);
		void allocateMemory(MemoryInfo info);
		void bindingMem2Buf();
		MemoryInfo queryMemoryInfo(vk::MemoryPropertyFlags property);
	};

}