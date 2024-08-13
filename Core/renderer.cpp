#include "renderer.hpp"
#include "context.hpp"

namespace doll {
	Renderer::Renderer(int maxFlightCount):maxFlightCount_(maxFlightCount),curFrame_(0)
	{
		initCmdPool();
		createCmdBuffers();
		createSemaphores();
		createFences();
	}
	Renderer::~Renderer()
	{
		cmdBufs_.clear();         
		imageAvailableSems_.clear();
		imageDrawFinishSems_.clear();
		cmdAvailableFences_.clear();
		cmdpool_.reset();
	}
	void Renderer::initCmdPool()
	{
		vk::CommandPoolCreateInfo createInfo;
		createInfo.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer);

		cmdpool_ = Context::Getinstance().device.createCommandPoolUnique(createInfo);
	}
	void Renderer::createCmdBuffers()
	{
		vk::CommandBufferAllocateInfo allocInfo;

		allocInfo.setCommandPool(cmdpool_.get())
			.setCommandBufferCount(maxFlightCount_)
			.setLevel(vk::CommandBufferLevel::ePrimary)
			;

		cmdBufs_ = Context::Getinstance().device.allocateCommandBuffersUnique(allocInfo);
	}

	void Renderer::DrawTriangle()
	{
		auto& device = Context::Getinstance().device;
		auto& renderProcess = Context::Getinstance().renderProcess;
		auto& swapchain = Context::Getinstance().swapchain;

		if (device.waitForFences(cmdAvailableFences_[curFrame_].get(), true, std::numeric_limits<uint32_t>::max()) != vk::Result::eSuccess)
		{
			throw std::runtime_error("wait for fence failed");
		}

		device.resetFences(cmdAvailableFences_[curFrame_].get());

		auto res = device.acquireNextImageKHR(swapchain->swapchain,
			std::numeric_limits<uint32_t>::max(), imageAvailableSems_[curFrame_].get());
		if (res.result != vk::Result::eSuccess)
		{
			throw std::runtime_error("accquire image failed!");
		}

		auto imageIndex = res.value;

		auto cmdbuf = cmdBufs_[curFrame_].get();
		cmdbuf.reset();

		vk::CommandBufferBeginInfo begin;
		begin.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
		cmdbuf.begin(begin);
		{
			vk::RenderPassBeginInfo renderPassBegin;
			vk::Rect2D area;
			vk::ClearValue cvalue;
			area.setOffset({ 0,0 })
				.setExtent(swapchain->info.imageExtent);

			cvalue.setColor(std::array<float, 4>{ 0.2, 0.2, 0.4, 1 });
			renderPassBegin.setRenderPass(renderProcess->renderpass.get())
				.setRenderArea(area)
				.setFramebuffer(swapchain->framebuffers[imageIndex].get())
				.setClearValues(cvalue)
				;
			cmdbuf.beginRenderPass(renderPassBegin, {}); {
				cmdbuf.bindPipeline(vk::PipelineBindPoint::eGraphics, renderProcess->pipeline);
				cmdbuf.draw(3, 1, 0, 0);
			}
			cmdbuf.endRenderPass();
		}
		cmdbuf.end();

		vk::SubmitInfo submit;
		vk::PipelineStageFlags waitStages[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
		submit.setCommandBuffers(cmdbuf)
			.setWaitSemaphores(imageAvailableSems_[curFrame_].get())
			.setSignalSemaphores(imageDrawFinishSems_[curFrame_].get())
			.setWaitDstStageMask(waitStages)
			;
		Context::Getinstance().graphicsQueue.submit(submit, cmdAvailableFences_[curFrame_].get());


		vk::PresentInfoKHR present;
		present.setImageIndices(imageIndex)
			.setSwapchains(swapchain->swapchain)
			.setWaitSemaphores(imageDrawFinishSems_[curFrame_].get());
		if (Context::Getinstance().presentQueue.presentKHR(present) != vk::Result::eSuccess)
		{
			throw std::runtime_error("image present failed!");
		}

		curFrame_ = (curFrame_ + 1) % maxFlightCount_;
	}
	void Renderer::createSemaphores()
	{
		vk::SemaphoreCreateInfo createInfo;
		imageAvailableSems_.resize(maxFlightCount_);
		imageDrawFinishSems_.resize(maxFlightCount_);

		for(auto& sem : imageAvailableSems_)
		{
			sem = Context::Getinstance().device.createSemaphoreUnique(createInfo);

		}
		for (auto& sem : imageDrawFinishSems_)
		{
			sem = Context::Getinstance().device.createSemaphoreUnique(createInfo);
		}
	}

	void Renderer::createFences()
	{
		cmdAvailableFences_.resize(maxFlightCount_);
		vk::FenceCreateInfo createInfo;
		createInfo.setFlags(vk::FenceCreateFlagBits::eSignaled);

		for (auto& fence : cmdAvailableFences_)
		{
			fence = Context::Getinstance().device.createFenceUnique(createInfo);
		}
	}
}