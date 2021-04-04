#include "Input.h"

#include "VoxelGI.h"

namespace VoxelGI
{

	Input::Input(GLFWwindow * wnd)
		: window(wnd),
		mouseScrollX(0.0f),
		mouseScrollY(0.0f)
	{
		for (int i = 0; i < numKeys; i++)
		{
			keyStates[i] = KS_KeyReleased;
			receivedKeyEvents[i] = -1;
		}
	}

	Input::~Input()
	{

	}

	void Input::scrollCallback(GLFWwindow * wnd, double xOffset, double yOffset)
	{
		scrollXOffsetCallback += (float)xOffset;
		scrollYOffsetCallback += (float)yOffset;
	}

	void Input::onKey(int key, int scancode, int action, int mods)
	{
		if (key != -1)
		{
			switch (action)
			{
			case GLFW_PRESS:
				receivedKeyEvents[key] = GLFW_PRESS;
				break;
			case  GLFW_RELEASE:
				receivedKeyEvents[key] = GLFW_RELEASE;
				break;
			}
		}
	}

	void Input::setWindow(GLFWwindow* wnd)
	{
		window = wnd;

		glfwSetKeyCallback(wnd, [](GLFWwindow* wnd, int key, int scancode, int action, int mods)
		{VoxelGI::wndThis(wnd)->getInput().onKey(key, scancode, action, mods); });

		glfwSetScrollCallback(window, scrollCallback);
	}

	void Input::update()
	{
		mouseScrollX = scrollXOffsetCallback;
		mouseScrollY = scrollYOffsetCallback;

		scrollXOffsetCallback = 0.0f;
		scrollYOffsetCallback = 0.0f;

		for (int i = 0; i < numKeys; i++)
		{
			if (receivedKeyEvents[i] == GLFW_PRESS)
				keyStates[i] = KS_KeyDown;
			else if (receivedKeyEvents[i] == GLFW_RELEASE)
				keyStates[i] = KS_KeyUp;
			else if (keyStates[i] == KS_KeyDown)
				keyStates[i] = KS_KeyPressed;
			else if (keyStates[i] == KS_KeyUp)
				keyStates[i] = KS_KeyReleased;

			receivedKeyEvents[i] = -1;
		}
	}


	bool Input::getMouseButton(int button)
	{
		return glfwGetMouseButton(window, button) == GLFW_PRESS;
	}

	Input::Pos2D Input::getMousePosition()
	{
		double x, y;
		glfwGetCursorPos(window, &x, &y);

		Pos2D mousePos = { std::floor(x), std::floor(y) };

		return mousePos;
	}

	bool Input::getKey(int key)
	{
		return glfwGetKey(window, key) == GLFW_PRESS;
	}

	bool Input::getKeyDown(int key)
	{
		return keyStates[key] == KS_KeyDown;
	}

	bool Input::getKeyUp(int key)
	{
		return keyStates[key] == KS_KeyUp;
	}

	Input::KeyState Input::getKeyState(int key)
	{
		return keyStates[key];
	}

	float Input::scrollX()
	{
		return mouseScrollX;
	}

	float Input::scrollY()
	{
		return mouseScrollY;
	}


	float Input::scrollXOffsetCallback = 0.0f;
	float Input::scrollYOffsetCallback = 0.0f;

}