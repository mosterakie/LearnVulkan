#pragma once

#include <vulkan/vulkan.hpp>

namespace doll {
	struct Vertex final
	{
		glm::vec3 pos;
		glm::vec2 texCoord;

		static std::array<vk::VertexInputAttributeDescription, 2> GetAttrD()
		{
			std::array<vk::VertexInputAttributeDescription, 2> attributeDescriptions{};
			attributeDescriptions[0].setBinding(0)
				.setFormat(vk::Format::eR32G32B32Sfloat)
				.setLocation(0)
				.setOffset(0);

			attributeDescriptions[1].setBinding(0)
				.setFormat(vk::Format::eR32G32Sfloat)
				.setLocation(1)
				.setOffset(offsetof(Vertex, texCoord));

			return attributeDescriptions;
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
