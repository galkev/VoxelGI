#pragma once

#include <GL\glew.h>
#include <GLFW/glfw3.h>

namespace VoxelGI
{

	class Input
	{
	public:
		struct Pos2D
		{
			int x, y;
		};

		enum KeyState
		{
			KS_KeyReleased,
			KS_KeyDown,
			KS_KeyPressed,
			KS_KeyUp
		};

	private:
		GLFWwindow* window;

		static const int numKeys = GLFW_KEY_LAST + 1;
		KeyState keyStates[numKeys];
		int receivedKeyEvents[numKeys];

		float mouseScrollX;
		float mouseScrollY;

		static float scrollXOffsetCallback;
		static float scrollYOffsetCallback;
		static void scrollCallback(GLFWwindow* wnd, double xOffset, double yOffset);

		void onKey(int key, int scancode, int action, int mods);

	public:
		Input(GLFWwindow* wnd = nullptr);
		~Input();

		bool getMouseButton(int button);
		Pos2D getMousePosition();

		bool getKey(int key);
		bool getKeyDown(int key);
		bool getKeyUp(int key);
		KeyState getKeyState(int key);

		float scrollX();
		float scrollY();

		void setWindow(GLFWwindow* wnd);

		void update();
	};

}