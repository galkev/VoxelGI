
#include "VoxelGI.h"

#include "RayTracerCpuEnvironment.h"
#include "RayTracerGLEnvironment.h"
#include "VoxelConeDoubleTracingEnvironment.h"
#include "TestEnvironment.h"

#include "imgui\imgui.h"
#include "imgui/imgui_impl_glfw_gl3.h"

namespace VoxelGI
{
	VoxelGI::VoxelGI()
		: width(1175),
		height(512)
	{
		//environment = new VoxelConeDoubleTracingEnvironment(*this);
		//environment = new TestEnvironment(*this);
		environment = new RayTracerCpuEnvironment<double>(*this);
		//environment = new RayTracerGLEnvironment(*this);
	}


	VoxelGI::~VoxelGI()
	{
		delete environment;
		environment = nullptr;
	}

	VoxelGI* VoxelGI::wndThis(GLFWwindow * window)
	{
		return (VoxelGI*)glfwGetWindowUserPointer(window);
	}

	Input & VoxelGI::getInput()
	{
		return input;
	}

	Timer & VoxelGI::getTimer()
	{
		return timer;
	}

	int VoxelGI::screenWidth()
	{
		return width;
	}

	int VoxelGI::screenHeight()
	{
		return height;
	}

	float VoxelGI::getAspect()
	{
		return (float)width / height;
	}

	bool VoxelGI::init()
	{
		initGL();
		environment->init();

		run();

		return true;
	}

	bool VoxelGI::initGL()
	{
		if (!glfwInit())
		{
			std::cout << "Failed to initialize GLFW" << std::endl;
			return false;
		}

		glfwWindowHint(GLFW_SAMPLES, 1);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

		window = glfwCreateWindow(width, height, "VoxelEngine", NULL, NULL);
		if (window == NULL)
		{
			fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
			glfwTerminate();
			return false;
		}
		glfwSetWindowUserPointer(window, this);
		glfwMakeContextCurrent(window);
		glewExperimental = true;
		if (glewInit() != GLEW_OK)
		{
			fprintf(stderr, "Failed to initialize GLEW\n");
			return false;
		}

		glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

		glfwSetFramebufferSizeCallback(window, [](GLFWwindow * window, int width, int height) { wndThis(window)->onResize(width, height); });

		input.setWindow(window);

		ImGui_ImplGlfwGL3_Init(window, true);

		return true;
	}

	void VoxelGI::run()
	{
		while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS && !glfwWindowShouldClose(window))
		{
			glfwPollEvents();

			ImGui_ImplGlfwGL3_NewFrame();
			timer.update();
			input.update();

			guiUpdate();
			update();
			render();

			// Swap buffers
			glfwSwapBuffers(window);
		}
	}

	void VoxelGI::guiUpdate()
	{
		environment->guiUpdate();
	}

	void VoxelGI::update()
	{
		environment->update();
	}

	void VoxelGI::render()
	{
		environment->render();
		ImGui::Render();
	}

	void VoxelGI::onResize(int newWidth, int newHeight)
	{
		width = newWidth;
		height = newHeight;

		glViewport(0, 0, width, height);

		environment->onResize(newWidth, newHeight);
	}



	void VoxelGI::exit()
	{
		ImGui_ImplGlfwGL3_Shutdown();
		glfwTerminate();
	}

}
