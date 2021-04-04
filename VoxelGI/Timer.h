#pragma once

#include <GLFW/glfw3.h>

namespace VoxelGI
{

	class Timer
	{
	private:
		float frameStartTime;
		float frameDeltaTime;

	public:
		Timer();
		~Timer();

		void update();

		float time();
		float frameTime();
		float deltaTime();
	};

}