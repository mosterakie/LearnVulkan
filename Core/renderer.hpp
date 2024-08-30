#pragma once
#include "vulkan/vulkan.hpp"
#include "buffer.hpp"
#include "uniform.hpp"



namespace doll {
	class Renderer final{
	public:
		Renderer(int maxFightCount = 2);
		~Renderer();
		void DrawTriangle();

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
		void updateSets();
		void updateSingleSet(int index);
		void createIndexBuffer();
		void bufferIndexData();
	public:

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

		vk::UniqueDescriptorPool descriptorPool_;
		std::vector<vk::DescriptorSet> sets_;  //DescriptorSet will destroy when pool destroy

	};
}