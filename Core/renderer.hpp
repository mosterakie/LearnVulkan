#pragma once
#include "vulkan/vulkan.hpp"


namespace doll {
	class Renderer final {
	public:
		Renderer(int maxFightCount = 2);
		~Renderer();
		void DrawTriangle();

	private:

	public:

	private:
		int maxFlightCount_;
		int curFrame_;
		vk::UniqueCommandPool cmdpool_;
		std::vector<vk::UniqueCommandBuffer> cmdBufs_;
		std::vector<vk::UniqueSemaphore> imageAvailableSems_;
		std::vector<vk::UniqueSemaphore> imageDrawFinishSems_;
		std::vector<vk::UniqueFence> cmdAvailableFences_;
		void initCmdPool();
		void createCmdBuffers();
		void createSemaphores();
		void createFences();
	};
}