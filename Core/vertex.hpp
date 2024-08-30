#pragma once

#include <vulkan/vulkan.hpp>

namespace doll {
	struct Vertex final
	{
		float x, y, z;

		static vk::VertexInputAttributeDescription GetAttrD()
		{
			vk::VertexInputAttributeDescription attr;
			attr.setBinding(0)
				.setFormat(vk::Format::eR32G32B32Sfloat)
				.setLocation(0)
				.setOffset(0);
			return attr;
		}

		static vk::VertexInputBindingDescription GetBindD()
		{
			vk::VertexInputBindingDescription binding;

			binding.setBinding(0)
				.setInputRate(vk::VertexInputRate::eVertex)
				.setStride(sizeof(Vertex));

			return binding;
		}
	};
}
