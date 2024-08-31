#include"windowdoll.hpp"

namespace doll {
	static bool s_GLFWInitialized = false;

	void G_Window::OnUpdate()
	{
		glfwPollEvents();
	}
	G_Window::G_Window(const WindowProps& props)
	{
		Init(props);
	}
	G_Window::~G_Window()
	{
		Shutdown();
	}
	void G_Window::Init(const WindowProps& props)
	{
		if (!s_GLFWInitialized)
		{
			glfwInit();
			glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
			glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
		}
		

		m_window = glfwCreateWindow(props.Width, props.Height,props.Name.c_str(), nullptr, nullptr);
		/*glfwSetWindowUserPointer(window, this);
		glfwSetWindowSizeCallback(window, framebufferResizeCallback);*/

	}
	void G_Window::Shutdown()
	{
		glfwDestroyWindow(m_window);
	}

	bool G_Window::ShouldClose()
	{
		return glfwWindowShouldClose(m_window);
	}

	Window* Create(const WindowProps& props)
	{
		return new G_Window(props);
	}

	std::vector<const char*> G_Window::getExtensions()
	{
		uint32_t count;
		auto glfwExtensions =  glfwGetRequiredInstanceExtensions(&count);

		return std::vector<const char*>(glfwExtensions, glfwExtensions + count);
	}

	void* G_Window::GetNativeWindow()
	{
		return m_window;
	}
}