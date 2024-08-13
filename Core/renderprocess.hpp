#pragma once
#include "vulkan/vulkan.hpp"

namespace doll {
	class  RenderProcess final
	{
	public:
		void InitPipeline(int width,int height);
		void InitLayout();
		void InitRenderPass();
		~RenderProcess();
	private:


	public:
		vk::Pipeline pipeline;
		vk::UniquePipelineLayout layout;
		vk::UniqueRenderPass renderpass;
	private:

	};

	 
}