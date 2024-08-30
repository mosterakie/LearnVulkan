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
			std::vector<vk::DescriptorSetLayoutBinding> bindings(1);

			bindings[0].setStageFlags(vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment)
				.setBinding(0)
				.setDescriptorType(vk::DescriptorType::eUniformBuffer)
				.setDescriptorCount(1);

			return bindings;
		}
	};
}