#pragma once

#include "Camera.h"
#include <glm/glm.hpp>

namespace VoxelGI
{

	class VoxelGI;

	class IEnvironment
	{
	public:
		virtual ~IEnvironment() {}

		virtual void init() = 0;
		virtual void guiUpdate() = 0;
		virtual void update() = 0;
		virtual void render() = 0;

		virtual Camera& getCamera() = 0;

		virtual void onResize(int width, int height) {}

		virtual glm::ivec2 getResolution() const = 0;
	};

}