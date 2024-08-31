#include "doll/renderer.hpp"
#include "doll/context.hpp"
#include "doll/vertex.hpp"
#include "doll/uniform.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include <chrono>


namespace doll {
	const std::array<Vertex, 5> vertices = {
		Vertex{{ -0.5,-0.5,0.0},{1.0f, 0.0f}},
		Vertex{{ 0.5f, -0.5f,0.0},{0.0f, 0.0f}},
		Vertex{{ 0.5,0.5,0.0},{0.0f, 1.0f}},
		Vertex{{ -0.5,0.5,0.0},{1.0f, 1.0f}}
	};
	const std::vector<uint16_t> indices{ 0, 1, 2, 2, 3, 0 };


	Renderer::Renderer(int maxFlightCount):maxFlightCount_(maxFlightCount),curFrame_(0)
	{
		initCmdPool();
		createCmdBuffers();
		createSemaphores();
		createFences();

		createVertexBuffer();
		bufferVertexData();
		createUniformBuffer();
		createDescriptorPool();
		allocateSets();
		//updateSets();
		createIndexBuffer();
		bufferIndexData();
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

		cmdpool_ = Context::Instance().device.createCommandPoolUnique(createInfo);
	}
	void Renderer::createCmdBuffers()
	{
		vk::CommandBufferAllocateInfo allocInfo;

		allocInfo.setCommandPool(cmdpool_.get())
			.setCommandBufferCount(maxFlightCount_)
			.setLevel(vk::CommandBufferLevel::ePrimary)
			;

		cmdBufs_ = Context::Instance().device.allocateCommandBuffersUnique(allocInfo);
	}

	vk::UniqueCommandBuffer Renderer::createOneCommandBufferUnique()
	{
		return std::move(createCommandBuffersUnique(1)[0]);
	}
	std::vector<vk::UniqueCommandBuffer> Renderer::createCommandBuffersUnique(uint32_t count)
	{
		auto device = Context::Instance().device;

		vk::CommandBufferAllocateInfo allocInfo;
		allocInfo.setCommandPool(cmdpool_.get())
			.setCommandBufferCount(count)
			.setLevel(vk::CommandBufferLevel::ePrimary);

		return device.allocateCommandBuffersUnique(allocInfo);
	}

	void Renderer::DrawFrame()
	{
		auto& device = Context::Instance().device;
		auto& renderProcess = Context::Instance().renderProcess;
		auto& swapchain = Context::Instance().swapchain;

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


		static auto startTime = std::chrono::high_resolution_clock::now();

		auto currentTime = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

		Uniform uni{
			Color{(int(time)%10)/10.0f,0.0,0.0,1.0},
		mvpMatrix{glm::rotate(glm::mat4(1.0f),
					  0.2f*time * glm::radians(90.0f),
					  glm::vec3(0.0f, 0.0f, 1.0f)),
		  glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f),
					  glm::vec3(0.0f, 0.0f, 0.0f),
					  glm::vec3(0.0f, 0.0f, 1.0f)),
		  glm::perspective(glm::radians(45.0f),
						   Context::Instance().swapchain->info.imageExtent.width / (float)Context::Instance().swapchain->info.imageExtent.height,
						   0.1f, 10.0f)} };
		uni.mvp.proj[1][1] *= -1;

		bufferUniformData(uni, curFrame_);
		//updateSingleSet(curFrame_);

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
				cmdbuf.bindPipeline(vk::PipelineBindPoint::eGraphics, renderProcess->pipeline.get());
				vk::DeviceSize offset = 0;
				cmdbuf.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, Context::Instance().renderProcess->layout.get(), 0, sets_[curFrame_], {});
				cmdbuf.bindVertexBuffers(0, gpuVertexBuffer_->buffer.get(), offset);
				cmdbuf.bindIndexBuffer(gpuIndexBuffer_->buffer.get(), 0, vk::IndexType::eUint16);
				//cmdbuf.draw(vertices.size(), 1, 0, 0);
				cmdbuf.drawIndexed(static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
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
		Context::Instance().graphicsQueue.submit(submit, cmdAvailableFences_[curFrame_].get());


		vk::PresentInfoKHR present;
		present.setImageIndices(imageIndex)
			.setSwapchains(swapchain->swapchain)
			.setWaitSemaphores(imageDrawFinishSems_[curFrame_].get());
		if (Context::Instance().presentQueue.presentKHR(present) != vk::Result::eSuccess)
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
			sem = Context::Instance().device.createSemaphoreUnique(createInfo);

		}
		for (auto& sem : imageDrawFinishSems_)
		{
			sem = Context::Instance().device.createSemaphoreUnique(createInfo);
		}
	}

	void Renderer::createFences()
	{
		cmdAvailableFences_.resize(maxFlightCount_);
		vk::FenceCreateInfo createInfo;
		createInfo.setFlags(vk::FenceCreateFlagBits::eSignaled);

		for (auto& fence : cmdAvailableFences_)
		{
			fence = Context::Instance().device.createFenceUnique(createInfo);
		}
	}

	void Renderer::createVertexBuffer()
	{
		cpuVertexBuffer_.reset(new Buffer(sizeof(vertices),
			vk::BufferUsageFlagBits::eTransferSrc,
			vk::MemoryPropertyFlagBits::eHostVisible|vk::MemoryPropertyFlagBits::eHostCoherent));
		gpuVertexBuffer_.reset(new Buffer(sizeof(vertices),
			vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst,
			vk::MemoryPropertyFlagBits::eDeviceLocal));
	}
	void Renderer::bufferVertexData()
	{
		void* ptr = Context::Instance().device.mapMemory(cpuVertexBuffer_->memory.get(), 0, cpuVertexBuffer_->size);
		memcpy(ptr, vertices.data(), sizeof(vertices));
		Context::Instance().device.unmapMemory(cpuVertexBuffer_->memory.get());
		
		copyBuffer(cpuVertexBuffer_->buffer.get(), gpuVertexBuffer_->buffer.get(), cpuVertexBuffer_.get()->size, 0, 0);
	}

	void Renderer::createUniformBuffer()
	{
		cpuUniformBuffer_.resize(maxFlightCount_);
		gpuUniformBuffer_.resize(maxFlightCount_);

		for (auto& buffer : cpuUniformBuffer_)
		{
			buffer.reset(new Buffer(Uniform::getSize(),
				vk::BufferUsageFlagBits::eTransferSrc,
				vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent));
		}

		for (auto& buffer : gpuUniformBuffer_)
		{
			buffer.reset(new Buffer(Uniform::getSize(),
				vk::BufferUsageFlagBits::eUniformBuffer | vk::BufferUsageFlagBits::eTransferDst,
				vk::MemoryPropertyFlagBits::eDeviceLocal));
		}
	}
	void Renderer::bufferUniformData(Uniform& uniform,uint32_t curr)
	{
		//for (int i = 0; i < cpuUniformBuffer_.size(); i++) {
			auto& buffer = cpuUniformBuffer_[curr];
			void* ptr = Context::Instance().device.mapMemory(buffer.get()->memory.get(), 0, buffer->size);
			//memcpy(ptr, &uniform, sizeof(uniform));
			memcpy(ptr, &uniform.color, sizeof(Color));
			memcpy(static_cast<uint8_t*>(ptr) + ((sizeof(Color) + 64 - 1) & ~(64 - 1)), &uniform.mvp, sizeof(mvpMatrix));
			Context::Instance().device.unmapMemory(buffer.get()->memory.get());

			copyBuffer(buffer.get()->buffer.get(), gpuUniformBuffer_[curr]->buffer.get(), buffer->size, 0, 0);
		//}
	}

	void Renderer::createDescriptorPool()
	{
		vk::DescriptorPoolCreateInfo createInfo;
		vk::DescriptorPoolSize poolSize;
		poolSize.setType(vk::DescriptorType::eUniformBuffer)
			.setDescriptorCount(maxFlightCount_);
		createInfo.setMaxSets(maxFlightCount_)
			.setPoolSizes(poolSize);

		descriptorPool_ = Context::Instance().device.createDescriptorPoolUnique(createInfo);
	}
	void Renderer::allocateSets() {
		std::vector<vk::DescriptorSetLayout> layouts(maxFlightCount_, Context::Instance().renderProcess->setlayout.get());
		vk::DescriptorSetAllocateInfo allocInfo;

		allocInfo.setDescriptorPool(descriptorPool_.get())
			.setDescriptorSetCount(maxFlightCount_)
			.setSetLayouts(layouts);

		sets_ = Context::Instance().device.allocateDescriptorSets(allocInfo);
	}

	void Renderer::updateSets(vk::ImageView imageView,vk::Sampler sampler)
	{
		for (int i=0;i<sets_.size();++i)
		{
			auto& set = sets_[i];
			vk::DescriptorBufferInfo bufferInfo0;
			bufferInfo0.setBuffer(gpuUniformBuffer_[i]->buffer.get())
				.setOffset(0)
				.setRange(sizeof(Color));

			vk::WriteDescriptorSet writer0;
			writer0.setDescriptorType(vk::DescriptorType::eUniformBuffer)
				.setBufferInfo(bufferInfo0)
				.setDstBinding(0)
				.setDstSet(set)
				.setDstArrayElement(0)
				.setDescriptorCount(1);

			vk::DescriptorBufferInfo bufferInfo1;
			bufferInfo1.setBuffer(gpuUniformBuffer_[i]->buffer.get())
				.setOffset((sizeof(Color) + 64 - 1) & ~(64 - 1))
				.setRange(sizeof(mvpMatrix));

			vk::WriteDescriptorSet writer1;
			writer1.setDescriptorType(vk::DescriptorType::eUniformBuffer)
				.setBufferInfo(bufferInfo1)
				.setDstBinding(1)
				.setDstSet(set)
				.setDstArrayElement(0)
				.setDescriptorCount(1);


			vk::DescriptorImageInfo imageInfo;
			imageInfo.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
				.setImageView(imageView)
				.setSampler(sampler);

			vk::WriteDescriptorSet writer3;
			writer3.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
				.setImageInfo(imageInfo)
				.setDstBinding(2)
				.setDstSet(set)
				.setDstArrayElement(0)
				.setDescriptorCount(1);

			std::vector<vk::WriteDescriptorSet> writers = { writer0, writer1 , writer3};
			Context::Instance().device.updateDescriptorSets(writers,{});
		}
	}

	void Renderer::updateSingleSet(int index)
	{
		auto& set = sets_[index];
		vk::DescriptorBufferInfo bufferInfo;
		bufferInfo.setBuffer(gpuUniformBuffer_[index]->buffer.get())
			.setOffset(0)
			.setRange(gpuUniformBuffer_[index]->size);

		vk::WriteDescriptorSet writer;
		writer.setDescriptorType(vk::DescriptorType::eUniformBuffer)
			.setBufferInfo(bufferInfo)
			.setDstBinding(0)
			.setDstSet(set)
			.setDstArrayElement(0)
			.setDescriptorCount(1);

		Context::Instance().device.updateDescriptorSets(writer, {});
	}

	void Renderer::createIndexBuffer()
	{
		cpuIndexBuffer_.reset(new Buffer(sizeof(indices[0]) * indices.size(),
			vk::BufferUsageFlagBits::eTransferSrc,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent));
		gpuIndexBuffer_.reset(new Buffer(sizeof(indices[0]) * indices.size(),
			vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst,
			vk::MemoryPropertyFlagBits::eDeviceLocal));
	}
	void Renderer::bufferIndexData()
	{
		void* ptr = Context::Instance().device.mapMemory(cpuIndexBuffer_->memory.get(), 0, cpuIndexBuffer_->size);
		memcpy(ptr, indices.data(), sizeof(indices));
		Context::Instance().device.unmapMemory(cpuIndexBuffer_->memory.get());

		copyBuffer(cpuIndexBuffer_->buffer.get(), gpuIndexBuffer_->buffer.get(), cpuIndexBuffer_.get()->size, 0, 0);
	}



























	void Renderer::copyBuffer(vk::Buffer& src, vk::Buffer& dst, size_t size, size_t srcOffset, size_t dstOffset) {
		auto& cmdBuf = createOneCommandBufferUnique();
		auto cmdbuf = cmdBuf.get();

		vk::CommandBufferBeginInfo begin;
		begin.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
		cmdbuf.begin(begin);
		{
			vk::BufferCopy region;
			region.setSize(size)
				.setSrcOffset(srcOffset)
				.setDstOffset(dstOffset);
			cmdbuf.copyBuffer(src, dst, region);
		} 
		cmdbuf.end();

		vk::SubmitInfo submit;
		submit.setCommandBuffers(cmdbuf);
		Context::Instance().graphicsQueue.submit(submit);

		Context::Instance().device.waitIdle();
	}

	void Renderer::copyBuf2Image(vk::Buffer& src, vk::Image& dst, uint32_t width, uint32_t height)
	{
		auto& cmdBuf = createOneCommandBufferUnique();
		auto cmdbuf = cmdBuf.get();

		vk::ImageSubresourceLayers subsource;
		subsource.setMipLevel(0)
			.setAspectMask(vk::ImageAspectFlagBits::eColor)
			.setLayerCount(1)
			.setBaseArrayLayer(0);

		vk::BufferImageCopy region;
		region.setImageOffset({ 0,0,0 })
			.setBufferOffset(0)
			.setBufferImageHeight(0)
			.setBufferRowLength(0)
			.setImageExtent({ width,height,1 })
			.setImageSubresource(subsource);

		std::vector<vk::BufferImageCopy> regions{ region };

		vk::CommandBufferBeginInfo begin;
		begin.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
		cmdbuf.begin(begin);
		{
			cmdbuf.copyBufferToImage(src,dst,vk::ImageLayout::eTransferDstOptimal,regions);
		}
		cmdbuf.end();

		vk::SubmitInfo submit;
		submit.setCommandBuffers(cmdbuf);
		Context::Instance().graphicsQueue.submit(submit);

		Context::Instance().device.waitIdle();
	}


	void Renderer::transitionImageLayout(vk::Image image, vk::Format format, vk::ImageLayout oldlayout, vk::ImageLayout newlayout)
	{
		vk::ImageSubresourceRange range;
		range.setAspectMask(vk::ImageAspectFlagBits::eColor)
			.setBaseArrayLayer(0)
			.setBaseMipLevel(0)
			.setLayerCount(1)
			.setLevelCount(1);

		vk::ImageMemoryBarrier barrier;
		barrier.setOldLayout(oldlayout)
			.setNewLayout(newlayout)
			.setImage(image)
			.setSubresourceRange(range);

		vk::PipelineStageFlags sourceStage,destinationStage;
		if (oldlayout == vk::ImageLayout::eUndefined && newlayout == vk::ImageLayout::eTransferDstOptimal)
		{
			barrier.setSrcAccessMask(vk::AccessFlagBits::eNone)
				.setDstAccessMask(vk::AccessFlagBits::eTransferWrite)
				.setDstQueueFamilyIndex(vk::QueueFamilyIgnored)
				.setSrcQueueFamilyIndex(vk::QueueFamilyIgnored);
			sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
			destinationStage = vk::PipelineStageFlagBits::eTransfer;
		}else if (oldlayout == vk::ImageLayout::eTransferDstOptimal && newlayout == vk::ImageLayout::eShaderReadOnlyOptimal)
		{
			barrier.setSrcAccessMask(vk::AccessFlagBits::eTransferWrite)
				.setDstAccessMask(vk::AccessFlagBits::eShaderRead)
				.setDstQueueFamilyIndex(vk::QueueFamilyIgnored)
				.setSrcQueueFamilyIndex(vk::QueueFamilyIgnored);
			sourceStage = vk::PipelineStageFlagBits::eTransfer;
			destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
		}



		auto& cmdBuf = createOneCommandBufferUnique();
		auto cmdbuf = cmdBuf.get();


		vk::CommandBufferBeginInfo begin;
		begin.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
		cmdbuf.begin(begin);
		{
			cmdbuf.pipelineBarrier(sourceStage,
				destinationStage,
				{}, {}, nullptr, barrier);
		}
		cmdbuf.end();


		vk::SubmitInfo submit;
		submit.setCommandBuffers(cmdbuf);
		Context::Instance().graphicsQueue.submit(submit);

		Context::Instance().device.waitIdle();
	}
}

