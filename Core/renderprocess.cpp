#include "renderprocess.hpp"
#include "shader.hpp"
#include "context.hpp"
#include "swapchain.hpp"


namespace doll {
	
	void RenderProcess::InitPipeline(int width,int height)
	{
		vk::GraphicsPipelineCreateInfo createInfo;

		//1.Vertex Input
		vk::PipelineVertexInputStateCreateInfo inputstate;
		createInfo.setPVertexInputState(&inputstate);


		//2.Vertex Assembly
		vk::PipelineInputAssemblyStateCreateInfo inputAsm;
		inputAsm.setPrimitiveRestartEnable(false)
			.setTopology(vk::PrimitiveTopology::eTriangleList);
		createInfo.setPInputAssemblyState(&inputAsm);

		//3.Shader
		auto stages = Shader::GetInstance().GetStage();
		createInfo.setStages(stages);

		//4.viewport
		vk::PipelineViewportStateCreateInfo viewportState;
		vk::Viewport viewport(0,0,width,height,0,1);
		vk::Rect2D rect({ 0,0 }, { static_cast<uint32_t>(width),static_cast<uint32_t>(height) });
		viewportState.setViewports(viewport)
			.setScissors(rect)
			;
		createInfo.setPViewportState(&viewportState);


		//5.Rasterization
		vk::PipelineRasterizationStateCreateInfo rasterization;
		rasterization.setCullMode(vk::CullModeFlagBits::eNone)
			.setRasterizerDiscardEnable(false)
			.setFrontFace(vk::FrontFace::eCounterClockwise)
			.setPolygonMode(vk::PolygonMode::eFill)
			.setLineWidth(1)
			;
		createInfo.setPRasterizationState(&rasterization);

		//6.multisample
		vk::PipelineMultisampleStateCreateInfo multisample;
		multisample.setSampleShadingEnable(false)
			.setRasterizationSamples(vk::SampleCountFlagBits::e1)
			;
		createInfo.setPMultisampleState(&multisample);

		//7.test
		//


		//8.color blending
		vk::PipelineColorBlendStateCreateInfo colorblend;
		vk::PipelineColorBlendAttachmentState attachment;
		attachment.setBlendEnable(false)
			.setColorWriteMask(vk::ColorComponentFlagBits::eA
									| vk::ColorComponentFlagBits::eB
									| vk::ColorComponentFlagBits::eG
									| vk::ColorComponentFlagBits::eR)
			;
		colorblend.setLogicOpEnable(false)
			.setAttachments(attachment)
			;
		createInfo.setPColorBlendState(&colorblend);

		//9.renderpass and layout

		createInfo.setRenderPass(renderpass.get())
			.setLayout(layout.get())
			;


		auto res = Context::Getinstance().device.createGraphicsPipeline(nullptr,createInfo);
		if (res.result != vk::Result::eSuccess)
		{
			throw std::runtime_error("failed to create pipeline!");
		}
		pipeline = res.value;
	}

	void RenderProcess::InitLayout()
	{
		vk::PipelineLayoutCreateInfo createInfo;
		layout = Context::Getinstance().device.createPipelineLayoutUnique(createInfo);
	}

	void RenderProcess::InitRenderPass()
	{
		vk::RenderPassCreateInfo createInfo;
		vk::AttachmentDescription attachDesc;
		attachDesc.setFormat(Context::Getinstance().swapchain->info.format.format)
			.setInitialLayout(vk::ImageLayout::eUndefined)
			.setFinalLayout(vk::ImageLayout::ePresentSrcKHR)
			.setLoadOp(vk::AttachmentLoadOp::eClear)
			.setStoreOp(vk::AttachmentStoreOp::eStore)
			.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
			.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
			.setSamples(vk::SampleCountFlagBits::e1)
			;
		createInfo.setAttachments(attachDesc);

		vk::SubpassDescription subpassDesc;
		vk::AttachmentReference reference;
		reference.setAttachment(0) //attachments的下标
			.setLayout(vk::ImageLayout::eColorAttachmentOptimal)
			;
		subpassDesc.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
			.setColorAttachments(reference)
			; 
		createInfo.setSubpasses(subpassDesc);

		vk::SubpassDependency dependency;
		dependency.setSrcSubpass(VK_SUBPASS_EXTERNAL)
			.setDstSubpass(0) //subpasses的下标
			.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite)
			.setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
			.setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
			;
		createInfo.setDependencies(dependency);

		renderpass = Context::Getinstance().device.createRenderPassUnique(createInfo);
	}
	
	RenderProcess::~RenderProcess()
	{
		Context::Getinstance().device.destroyPipeline(pipeline);
	}
}