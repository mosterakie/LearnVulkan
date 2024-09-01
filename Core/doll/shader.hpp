#pragma once
#include "vulkan/vulkan.hpp"

namespace doll {
	class Shader final
	{
	public:
		static void Init(const std::string& vertexSource, const std::string& fragSource);
		static void Quit();

		static Shader& GetInstance();
		std::vector<vk::PipelineShaderStageCreateInfo> GetStage();

		~Shader();
	private:
		Shader(const std::string& vertexSource, const std::string& fragSource);
		void initStage();

	public:
		vk::ShaderModule vertexMoudle;
		vk::ShaderModule fragMoudle;

		
	private:
		static std::unique_ptr<Shader> instance_;
		std::vector<vk::PipelineShaderStageCreateInfo> stage_;
	};
}