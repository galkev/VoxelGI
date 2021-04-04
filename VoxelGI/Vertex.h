#pragma once

#include <glm\glm.hpp>

namespace VoxelGI
{
	template <typename T>
	struct Vertex
	{
		glm::tvec3<T> pos;
		glm::tvec3<T> normal;
		glm::tvec2<T> uv;
	};

}