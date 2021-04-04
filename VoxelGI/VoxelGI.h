#pragma once

#include <GL\glew.h>
#include <GLFW\glfw3.h>

#include <iostream>

#include "Input.h"
#include "Timer.h"

#include "Environment.h"

#include "Gui.h"

namespace VoxelGI
{

	class VoxelGI
	{
	private:
		GLFWwindow* window;
		int width;
		int height;

		Timer timer;
		Input input;
		Gui guiControl;

		IEnvironment* environment;

		bool initGL();
		void run();

		void guiUpdate();
		void update();
		void render();

		void onResize(int newWidth, int newHeight);

	public:
		VoxelGI();
		~VoxelGI();

		bool init();
		void exit();

		static VoxelGI* wndThis(GLFWwindow* window);

		Input& getInput();
		Timer& getTimer();

		int screenWidth();
		int screenHeight();

		float getAspect();
	};

}