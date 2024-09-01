#include "renderprocess.hpp"
#include "shader.hpp"
#include "context.hpp"
#include "swapchain.hpp"
#include "uniform.hpp"
#include "vertex.hpp"


namespace doll {
	
	void RenderProcess::InitPipeline(int width,int height)
	{
		vk::GraphicsPipelineCreateInfo createInfo;

		//1.Vertex Input
		vk::PipelineVertexInputStateCreateInfo inputstate;
		auto attr = Vertex::GetAttrD();
		auto binding = Vertex::GetBindD();
		inputstate.setVertexAttributeDescriptions(attr)
			.setVertexBindingDescriptions(binding);
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
		vk::PipelineDepthStencilStateCreateInfo depthStencilState;
		depthStencilState.setDepthWriteEnable(true)
			.setDepthTestEnable(true)
			.setDepthCompareOp(vk::CompareOp::eLess)
			.setDepthBoundsTestEnable(false)
			.setMinDepthBounds(0.0f)
			.setMaxDepthBounds(1.0f)
			.setStencilTestEnable(false);
		createInfo.setPDepthStencilState(&depthStencilState);


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


		auto res = Context::Instance().device.createGraphicsPipelineUnique(nullptr,createInfo);
		if (res.result != vk::Result::eSuccess)
		{
			throw std::runtime_error("failed to create pipeline!");
		}
		pipeline= std::move(res.value); //wtf
	}

	void RenderProcess::InitLayout()
	{
		vk::PipelineLayoutCreateInfo createInfo;
		createInfo.setSetLayouts(setlayout.get());
		layout = Context::Instance().device.createPipelineLayoutUnique(createInfo);
	}

	void RenderProcess::InitRenderPass()
	{
		setlayout = createSetlayout();
		vk::AttachmentDescription attachDesc;
		attachDesc.setFormat(Context::Instance().swapchain->info.format.format)
			.setInitialLayout(vk::ImageLayout::eUndefined)
			.setFinalLayout(vk::ImageLayout::ePresentSrcKHR)
			.setLoadOp(vk::AttachmentLoadOp::eClear)
			.setStoreOp(vk::AttachmentStoreOp::eStore)
			.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
			.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
			.setSamples(vk::SampleCountFlagBits::e1)
			;

		vk::AttachmentReference reference;
		reference.setAttachment(0) //attachments的下标
			.setLayout(vk::ImageLayout::eColorAttachmentOptimal)
			;

		//创建深度附着
		vk::AttachmentDescription depAttachDesc;
		depAttachDesc.setFormat(Context::Instance().findDepthFormat())
			.setInitialLayout(vk::ImageLayout::eUndefined)
			.setFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal)
			.setLoadOp(vk::AttachmentLoadOp::eClear)
			.setStoreOp(vk::AttachmentStoreOp::eDontCare)
			.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
			.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
			.setSamples(vk::SampleCountFlagBits::e1)
			;

		vk::AttachmentReference depReference;
		depReference.setAttachment(1) //attachments的下标
			.setLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal)
			;

		vk::SubpassDescription subpassDesc;
		subpassDesc.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
			.setColorAttachments(reference)
			.setPDepthStencilAttachment(&depReference);

		std::array<vk::AttachmentDescription, 2> attachments = { attachDesc, depAttachDesc };

		vk::SubpassDependency dependency;
		dependency.setSrcSubpass(VK_SUBPASS_EXTERNAL)
			.setDstSubpass(0) //subpasses的下标
			.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentWrite)
			.setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests)
			.setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests)
			;

		vk::RenderPassCreateInfo createInfo;
		createInfo.setAttachmentCount(2);
		createInfo.setAttachments(attachments);
		createInfo.setSubpasses(subpassDesc);
		createInfo.setDependencies(dependency);
		renderpass = Context::Instance().device.createRenderPassUnique(createInfo);
	}
	

	vk::UniqueDescriptorSetLayout RenderProcess::createSetlayout()
	{
		vk::DescriptorSetLayoutCreateInfo createInfo;
		auto binding = Uniform::GetBinding();
		createInfo.setBindings(binding);

		return Context::Instance().device.createDescriptorSetLayoutUnique(createInfo);
	}

	RenderProcess::~RenderProcess()
	{
	}
}