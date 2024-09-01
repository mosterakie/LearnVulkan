#pragma once
#include "vulkan/vulkan.hpp"
#include "buffer.hpp"
#include "uniform.hpp"
#include "image.hpp"



namespace doll {
	class Renderer final{
	public:
		Renderer(int maxFightCount = 2);
		~Renderer();
		void DrawFrame();
		void copyBuf2Image(vk::Buffer& src, vk::Image& dst, uint32_t width, uint32_t height);
		void transitionImageLayout(vk::Image image, vk::Format format, vk::ImageLayout oldlayout, vk::ImageLayout newlayout);
		void updateSets(vk::ImageView imageView, vk::Sampler sampler);
		void createDepthresources();
		
	private:
		void initCmdPool();
		void createCmdBuffers();
		void createSemaphores();
		void createFences();
		void createVertexBuffer();
		void bufferVertexData();
		void createUniformBuffer();
		void bufferUniformData(Uniform& uniform,uint32_t curr);
		vk::UniqueCommandBuffer createOneCommandBufferUnique();
		std::vector<vk::UniqueCommandBuffer> createCommandBuffersUnique(uint32_t count);
		void copyBuffer(vk::Buffer& src, vk::Buffer& dst, size_t size, size_t srcOffset, size_t dstOffset);
		void createDescriptorPool();
		void allocateSets();
		void updateSingleSet(int index);
		void createIndexBuffer();
		void bufferIndexData();

	public:
		vk::UniqueImage depthImage_;
		vk::UniqueDeviceMemory depthImageMemory_;
		vk::UniqueImageView depthImageView_;
		
	private:
		int maxFlightCount_;
		int curFrame_;
		vk::UniqueCommandPool cmdpool_;
		std::vector<vk::UniqueCommandBuffer> cmdBufs_;
		std::vector<vk::UniqueSemaphore> imageAvailableSems_;
		std::vector<vk::UniqueSemaphore> imageDrawFinishSems_;
		std::vector<vk::UniqueFence> cmdAvailableFences_;

		std::unique_ptr<Buffer> cpuVertexBuffer_;
		std::unique_ptr<Buffer> gpuVertexBuffer_;

		std::vector<std::unique_ptr<Buffer>> cpuUniformBuffer_;
		std::vector<std::unique_ptr<Buffer>> gpuUniformBuffer_;

		std::unique_ptr<Buffer> cpuIndexBuffer_;
		std::unique_ptr<Buffer> gpuIndexBuffer_;

		std::unique_ptr<Buffer> cpuImageBuffer_;
		vk::UniqueImage textureImage_;

		vk::UniqueDescriptorPool descriptorPool_;
		std::vector<vk::DescriptorSet> sets_;  //DescriptorSet will destroy when pool destroy

	};
}