#include "shader.hpp"
#include "context.hpp"
#include <fstream>

namespace doll {
	std::unique_ptr<Shader> Shader::instance_ = nullptr;

	void Shader::Init(const std::string& vertexSource, const std::string& fragSource)
	{
		instance_.reset(new Shader(vertexSource, fragSource));
	}
	void Shader::Quit()
	{
		instance_.reset();
	}
	Shader::Shader(const std::string& vertexSource, const std::string& fragSource)
	{
		vk::ShaderModuleCreateInfo createInfo;
		createInfo.setCodeSize(vertexSource.size())
			.setPCode((uint32_t*)(vertexSource.data()))
			;
		vertexMoudle = Context::Instance().device.createShaderModule(createInfo);
		createInfo.setCodeSize(fragSource.size())
			.setPCode((uint32_t*)(fragSource.data()))
			;
		fragMoudle = Context::Instance().device.createShaderModule(createInfo);
		initStage();
	}
	Shader& Shader::GetInstance()
	{
		return *instance_;
	}
	Shader::~Shader()
	{
		auto& device = Context::Instance().device;
		device.destroyShaderModule(vertexMoudle);
		device.destroyShaderModule(fragMoudle);
	}

	std::vector<vk::PipelineShaderStageCreateInfo> Shader::GetStage()
	{
		return stage_;
	}

	void Shader::initStage()
	{
		stage_.resize(2);
		stage_[0].setStage(vk::ShaderStageFlagBits::eVertex)
			.setModule(vertexMoudle)
			.setPName("main");
		stage_[1].setStage(vk::ShaderStageFlagBits::eFragment)
			.setModule(fragMoudle)
			.setPName("main");
	}

}