#pragma once
#include "vulkan/vulkan.hpp"
#include "glm/glm.hpp"

namespace doll {
	struct Color final
	{
		float r, g, b, a;
	};

	struct mvpMatrix final{
		glm::mat4 model;
		glm::mat4 view;
		glm::mat4 proj;
	};

	struct Uniform final
	{
		Color color;

		mvpMatrix mvp;

		static std::vector<vk::DescriptorSetLayoutBinding> GetBinding()
		{
			std::vector<vk::DescriptorSetLayoutBinding> bindings(3);

			bindings[0].setStageFlags(vk::ShaderStageFlagBits::eFragment | vk::ShaderStageFlagBits::eVertex)
				.setBinding(0)
				.setDescriptorType(vk::DescriptorType::eUniformBuffer)
				.setDescriptorCount(1);

			bindings[1].setStageFlags(vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment)
				.setBinding(1)
				.setDescriptorType(vk::DescriptorType::eUniformBuffer)
				.setDescriptorCount(1);

			bindings[2].setStageFlags(vk::ShaderStageFlagBits::eFragment)
				.setBinding(2)
				.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
				.setDescriptorCount(1);

			return bindings;
		}

		static size_t getSize()
		{
			size_t alignedOffset = (sizeof(Color) + 64 - 1) & ~(64 - 1);
			size_t alignedOffset1 = (sizeof(mvpMatrix) + 64 - 1) & ~(64 - 1);
			return alignedOffset + alignedOffset1;
		}
	};
}