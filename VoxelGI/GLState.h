#pragma once

namespace VoxelGI
{
	template <typename T>
	class GLPushState
	{
	private:
		T oldState;
		T state;

	public:
		template <typename... Args>
		GLPushState(Args&&... args)
			: oldState(), state(args...)
		{
			state.set();
		}

		~GLPushState()
		{
			oldState.set();
		}
	};

	namespace GLState
	{
		class IGLState
		{
			virtual void set() = 0;
		};

		class Viewport 
		{
		private:
			GLint params[4];

		public:
			Viewport()
			{
				glGetIntegerv(GL_VIEWPORT, params);
			}

			Viewport(int x, int y, int width, int height)
			{
				params[0] = x;
				params[1] = y;
				params[2] = width;
				params[3] = height;
			}

			void set()
			{
				glViewport(params[0], params[1], params[2], params[3]);
			}
		};
	}
}