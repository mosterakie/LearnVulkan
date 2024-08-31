#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <string>	
#include <vector>


namespace doll {
	struct WindowProps
	{
		uint32_t Width;
		uint32_t Height;
		std::string Name;
		WindowProps(uint32_t width = 1920, uint32_t height = 1080, std::string name = "default window") :Width(width), Height(height), Name(name) {

		}

	};

	class Window
	{
	public:
		virtual ~Window() {};
		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;
		virtual void OnUpdate() = 0;
		virtual void Shutdown() = 0;
		static Window* Create(const WindowProps& props = WindowProps());
		virtual bool ShouldClose() = 0;
		virtual void* GetWindow()const = 0;
		virtual std::vector<const char*> getExtensions() = 0;
		virtual void* GetNativeWindow() = 0;
	};

	class G_Window:public Window {
	public:
		G_Window(const WindowProps& props);
		virtual ~G_Window();
		virtual uint32_t GetWidth() const override { return m_props.Width; }
		virtual uint32_t GetHeight() const override { return m_props.Height; }
		virtual void OnUpdate() override;
		virtual bool ShouldClose() override;
		virtual void* GetWindow()const override { return m_window; }
		virtual std::vector<const char*> getExtensions() override;
		virtual void* GetNativeWindow() override;
	private:
		virtual void Init(const WindowProps& props);
		virtual void Shutdown() override;
	public:

	private:
		GLFWwindow* m_window;
		WindowProps m_props;
	};

}

