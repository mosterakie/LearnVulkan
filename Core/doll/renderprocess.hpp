#pragma once
#include "vulkan/vulkan.hpp"

namespace doll {
	class  RenderProcess final
	{
	public:
		void InitPipeline(int width,int height);
		void InitLayout();
		void InitRenderPass();
		vk::UniqueDescriptorSetLayout createSetlayout();
		~RenderProcess();
	private:


	public:
		vk::UniquePipeline pipeline;
		vk::UniquePipelineLayout layout;
		vk::UniqueRenderPass renderpass;
		vk::UniqueDescriptorSetLayout setlayout;
	private:
	};

	 
}