#include "Timer.h"

namespace VoxelGI
{

	Timer::Timer()
	{
	}


	Timer::~Timer()
	{
	}

	void Timer::update()
	{
		float lastTime = frameStartTime;
		frameStartTime = (float)glfwGetTime();
		frameDeltaTime = frameStartTime - lastTime;
	}

	float Timer::time()
	{
		return (float)glfwGetTime();
	}

	float Timer::frameTime()
	{
		return frameStartTime;
	}

	float Timer::deltaTime()
	{
		return frameDeltaTime;
	}

}